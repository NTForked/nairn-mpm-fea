# Introduction #

This page explains how to create a new types of elements for NairnFEA and NairnMPM.

# Add an Element for NairnFEA and/or NairnMPM #

The following sections described the various tasks needed to support a new element type in NairnFEA and NairnMPM. Since NairnMPM is typcially based only on 4-node 2D and 8-node 3D elements, these tasks are mostly about adding them for NairnFEA, but a research task on new MPM elements could be a potential reason to add MPM elements.

## Create New Element Class Source Files ##

The first step is to create a new class that is a subclass of an existing element or a subclass of `ElementBase` if it is  new class of element. The steps for this task are:

  1. Define new source for the element and create `.cpp` and `.hpp` files
  1. Define a constant (numeric ID) for the new element. To avoid potential future conflicts, all these constants are defines in `ElementBase.hpp`.
  1. Write two constructors (as needed) for the element class:
    * `NewElement(int eNum, int *eNode)` - for MPM, the constructor arguments are the element number and an integer array of node numbers.
    * `NewElement(int eNum, int *eNode, int eMat,double eAng,double eThick)`- for FEA, the additional arguments are the material number, the angle (for anisotropic materials), and the thickness (for plane strain or plane stress calculations).
  1. All elements must override the following methods (unless handled by a superclass) and return new element values:
    * `ElementName(void)` - return numeric code defined in `ElementBase.hpp`
    * `NumberNodes(void)` - return number of nodes
    * `NumberSides(void)` - return number of sides (default is 4 if not overridden)
    * `GetArea(void)` - element area (only used in MPM to decide on wave speed)
    * `FaceNodes(void)` - return number of nodes on each face
    * `PtInElement()` - if point is in the element return 1 otherwise 0
    * `ShapeFunction()` - load shape functions, their derivatives (if requested), and information for axisymmetric analyses. Both MPM and FEA may use this function (the one with B matrix calculations), but MPM will require more shape function methods described below.
  1. Elements that will be used in FEA calculations have other methods related to stiffness matrix and stress calculations. These functions can be ignored if the new element will not be used for FEA calculations.
    * `CalcEdgeLoads()` - this method is called by `GetConsistentLoads(re,np)` that is called by `ForcesOnEdges()` to support stress based boundary conditions on an element edge.
    * `Stiffness()` - called in `BuildStiffnessMatrix()` to calculate element stiffness matrix. If element uses `IsoparametricStiffness()` then may need to set `numGauss` and `gaussSet`.
    * `ForceStress()` - called in `ForceStressEnergyResults(void)` to calculate forces, stresses, and energy. If use `IsoparametricForceStress()` need to fill in option for extrapolation to nodes for new element type. Also may need to set `numGauss` and `gaussSet`.
  1. Elements that will be used in MPM calculations must implement an additional shape function method (for speed) and a few other methods. These first functions activate the element for non-GIMP calculations only (see next item for GIMP as well). These functions can be ignored if the new element will not be used for MPM calculations:
    * `ShapeFunction()` - load shape functions and their derivatives (if requested). Note that MPM uses the FEA shape function (from above) and has an additional call for speed. The second one is used when running in a regular grid and therefore gradients can be found with scaling factors (2/dx and 2/dy) rather than needing B matrix coordinate transformation.
    * `FindExtent()` - find range of nodal coordinates. The basic task is handled in `ElementBase` but elements may want some extra calculations here for speed in later calculations. This method is called only once.
    * `GetXiPos()` - find dimensionless position. The base method in `ElementBase` finds the position numerically. Ideally all MPM elements will find it by an analytical expression, which may depend on shape of element and can use parameters calculated in `FindExtent()`.
    * `Orthogonal()` - is the element a rectangle aligned with x and z axes? Called only once and only for non-regular grids (uncommon).
  1. Elements that will do MPM-GIMP calculations need above MPM methods and the following extra methods for GIMP shape functions and gradients.  These function can be ignored if the new element will not be used for MPM-GIMP calculations:
    * `GetGimpNodes()` - get nodes for GIMP shape functions
    * `GimpShapeFunction()` get GIMP shape functions
  1. All other element needs should be handled by `ElementBase.cpp` (which is partly in `MoreElementBase.cpp` for FEA and in `MoreMPMElementCase.cpp` for MPM). These methods can be checked for compatibility if problems arise.

## Create Instances of Elements ##

During initialization of a calculation, the mesh is created and divided into elements. The tasks in this section are needed to be able to add the new element to a mesh.

  1. Include the new element's header file in `ElementsController.cpp`.
  1. The `ElementsController.cpp` code handles all tasks when creating elements for a mesh. New elements will need some element-specific modifications to this source file withinthe following subroutines:
    * `ElementsCompatible()` - document elements that are compatible in same mesh with new element
    * `HasMidsideNodes()` - return true if new element has mid side nodes
    * `ElementSides()` - return number of sides for the new element
    * `ElementBase *MeshElement()` - creates elements for MPM grid. The input parameters are the element ID, the element number, and an integer array of nodes. To use the element in MPM grids, add a `case` to the `switch` block for the new element and return the element instance. Note: NairnMPM currently assumes 4 node quadrilateral elements in 2D and 8 node brick elements is 3D. Before new elements can be used, the MPM input commands will need a new attribute or command related to the grid to specify the new element type. The required changes are very minor.
    * `int MeshElement()` - creates elements for FEA mesh. The input parameters are 4 or 8 nodes counter clockwise around a quadrilateral area (for interface elements it is 4 or 6 nodes counter clockwise back and forth along the interface). To use the element in FEA meshes, add a `case` to the `switch` block for the new element. See existing example for needs. It is OK for the new code to add internal nodes (if needed) or even to create more than one element within the quadrilateral area. See triangular elements and 9-node Lagrange 2D elements for examples of such additions.
    * `CreateElement()` - Elements can be created from XML `elem` command, which has the format
```
<elem type='2' matl='1' angle='0' thick='10'>1,5,69,68</elem>
```
> > where type is element code (numeric ID defined above), `matl` is numerical material ID, `angle` is material angle for anisotropic materials, and `thick` is thickness in mm. The data is list of node numbers (delimited by white space, commas, colons, or semicolons). This creating method is only used when explicitly generating meshes, which is very uncommon in FEA and not recommended for MPM (because it cannot use GIMP). Support for the new element in this subroutine is therefore optional. To provide support, add a new `case` to the `switch` block to verify the right number of nodes were received and then to create an instance of the new element. If omitted, this uncommon mesh method will not be able to use the new element type.

## Possible Issues ##

Both NairnFEA and NairnMPM have some legacy code that might be hard coded to maximum number of nodes in an element of assume certain element shape (_i.e._, assume nodes correspond to edges and mid side nodes in a specific order). If you create FEA element with more than 9 nodes or MPM element with more than 8 nodes, any such legacy issues might cause problems. Elements with fewer nodes should be OK. To check for other issues, look at methods in super classes to new elements to verify none of them have specific assumptions that do not apply to the current element.