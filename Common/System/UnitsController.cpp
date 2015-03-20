/********************************************************************************
	UnitsController.cpp
	NairnFEA/NairnMPM

	Created by John Nairn on 3/15/15.
	Copyright (c) 2015 John A. Nairn, All rights reserved.
********************************************************************************/

#include "System/UnitsController.hpp"

// class variables
int UnitsController::unitsType = LEGACY_UNITS;

// Constructor
UnitsController::UnitsController(void)
{
	
}

// When reading a variable, set scale factor (if in legacy units)
// and return the pointer
char *UnitsController::ScaledPtr(char *ptr,double &ptrScaling,double legacyScale)
{
	if(unitsType==LEGACY_UNITS) ptrScaling = legacyScale;
	return ptr;
}

// Return provided scalling factor or 1 if using consistent units
double UnitsController::Scaling(double legacyScale)
{	return unitsType==LEGACY_UNITS ? legacyScale : 1. ;
}

// Return label for a physical quantity
const char *UnitsController::Label(int type)
{
	switch(unitsType)
	{	case LEGACY_UNITS:
			switch(type)
			{	case ERR_UNITS:
					return "J/m^2";
				case OUTLENGTH_UNITS:
					return "mm";
				case TRACTIONSLOPE_UNITS:
					return "MPa/mm";
				case PRESSURE_UNITS:
					return "MPa";
				case HEATCAPACITY_UNITS:
					return "J/(kg-K)";
				case C0_UNITS:
					return "m/sec";
				case VISCOSITY_UNITS:
					return "cP";
				case CONDUCTIVITY_UNITS:
					return "W/(m-K)";
				case DENSITY_UNITS:
					return "g/cm^3";
				case STRESSINTENSITY_UNITS:
					return "MPa-sqrt(m)";
				case FORCE_UNITS:
					// for FEA output
					return "N";
				case WORK_UNITS:
					// for FEA output
					return "J";
				case INTERFACEPARAM_UNITS:
					return "MPa/mm";
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	// no label gound
	return "????";
			
}

