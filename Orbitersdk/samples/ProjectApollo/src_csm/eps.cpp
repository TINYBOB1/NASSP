/***************************************************************************
This file is part of Project Apollo - NASSP
Copyright 2024

Command Module Electrical Power Subsystem

Project Apollo is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Project Apollo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Project Apollo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

See http://nassp.sourceforge.net/license/ for more details.

**************************************************************************/

#pragma include_alias( <fstream.h>, <fstream> )
#include "Orbitersdk.h"
#include <stdio.h>

#include "PanelSDK/PanelSDK.h"
#include "PanelSDK/Internals/Hsystems.h"

#include "soundlib.h"
#include "nasspdefs.h"
#include "toggleswitch.h"

#include "saturn.h"

CryoPressureSwitch::CryoPressureSwitch()
{
	saturn = NULL;
	lowpress = 0;
	highpress = 0;
	PressureSwitch1 = false;
	PressureSwitch2 = false;

}

CryoPressureSwitch::~CryoPressureSwitch() 
{

}

void CryoPressureSwitch::Init(Saturn *s, h_Tank *tnk1, h_Tank *tnk2, Boiler *htr1, Boiler *htr2, Boiler *fn1, Boiler *fn2, 
	ThreePosSwitch *htrsw1, ThreePosSwitch *htrsw2, ThreePosSwitch *fnsw1, ThreePosSwitch *fnsw2,
	CircuitBrakerSwitch *dc1, CircuitBrakerSwitch *dc2, 
	CircuitBrakerSwitch *ac1a, CircuitBrakerSwitch *ac1b, CircuitBrakerSwitch *ac1c, CircuitBrakerSwitch *ac2a, CircuitBrakerSwitch *ac2b, CircuitBrakerSwitch *ac2c,
	double lp, double hp)
{
	saturn = s;
	tank1 = tnk1;
	tank2 = tnk2;
	heater1 = htr1;
	heater2 = htr2;
	fan1 = fn1;
	fan2 = fn2;

	htrswitch1 = htrsw1;
	htrswitch2 = htrsw2;
	fanswitch1 = fnsw1;
	fanswitch2 = fnsw2;

	dcbreaker1 = dc1;
	dcbreaker2 = dc2;
	ac1abreaker = ac1a;
	ac1bbreaker = ac1b;
	ac1cbreaker = ac1c;
	ac2abreaker = ac2a;
	ac2bbreaker = ac2b;
	ac2cbreaker = ac2c;

	lowpress = lp;
	highpress = hp;
}

bool CryoPressureSwitch::IsACBus1Powered()
{
	if (ac1abreaker->Voltage() > SP_MIN_ACVOLTAGE && ac1bbreaker->Voltage() > SP_MIN_ACVOLTAGE && ac1cbreaker->Voltage() > SP_MIN_ACVOLTAGE)
	{
		return true;
	}

	else
		return false;
}

bool CryoPressureSwitch::IsACBus2Powered()
{
	if (ac2abreaker->Voltage() > SP_MIN_ACVOLTAGE && ac2bbreaker->Voltage() > SP_MIN_ACVOLTAGE && ac2cbreaker->Voltage() > SP_MIN_ACVOLTAGE)
	{
		return true;
	}

	else
		return false;
}

void CryoPressureSwitch::SystemTimestep(double simdt)
{
	if (!tank1 || !tank2) return;

	//Tank 1 Pressure Switch
	if (tank1->space.Press < (lowpress / PSI))
	{
		PressureSwitch1 = true;
	}

	else if (PressureSwitch1 == true && tank1->space.Press >(highpress / PSI))
	{
		PressureSwitch1 = false;
	}

	else
	{
		PressureSwitch1 = false;
	}

	//Tank 2 Pressure Switch
	if (tank2->space.Press < (lowpress / PSI))
	{
		PressureSwitch2 = true;
	}

	else if (PressureSwitch2 == true && tank2->space.Press >(highpress / PSI))
	{
		PressureSwitch2 = false;
	}

	else
	{
		PressureSwitch2 = false;
	}

	//Tank 1 Heater Control
	//AUTO
	if (PressureSwitch1 == true && PressureSwitch2 == true && htrswitch1->GetState() == THREEPOSSWITCH_UP)
	{
		heater1->SetPumpOn();
	}
	//ON
	else if (htrswitch1->GetState() == THREEPOSSWITCH_DOWN)
	{
		heater1->SetPumpOn();
	}
	//OFF
	else
	{
		heater1->SetPumpOff();
	}

	//Tank 2 Heater Control
	//AUTO
	if (PressureSwitch1 == true && PressureSwitch2 == true && htrswitch2->GetState() == THREEPOSSWITCH_UP)
	{
		heater2->SetPumpOn();
	}
	//ON
	else if (htrswitch1->GetState() == THREEPOSSWITCH_DOWN)
	{
		heater2->SetPumpOn();
	}
	//OFF
	else
	{
		heater2->SetPumpOff();
	}

	//Tank 1 Fan Control
	//AUTO
	if (IsACBus1Powered() && PressureSwitch1 == true && PressureSwitch2 == true && fanswitch1->GetState() == THREEPOSSWITCH_UP)
	{
		fan1->SetPumpOn();
	}
	//ON
	else if (fanswitch1->GetState() == THREEPOSSWITCH_DOWN)
	{
		fan1->SetPumpOn();
	}
	//OFF
	else
	{
		fan1->SetPumpOff();
	}

	//Tank 2 Fan Control
	//AUTO
	if (IsACBus2Powered() && PressureSwitch1 == true && PressureSwitch2 == true && fanswitch2->GetState() == THREEPOSSWITCH_UP)
	{
		fan2->SetPumpOn();
	}
	//ON
	else if (fanswitch1->GetState() == THREEPOSSWITCH_DOWN)
	{
		fan2->SetPumpOn();
	}
	//OFF
	else
	{
		fan2->SetPumpOff();
	}

}

void CryoPressureSwitch::LoadState(char *line)
{
	int i, j;

	sscanf(line + 15, "%i %i", &i, &j);

	PressureSwitch1 = (i != 0);
	PressureSwitch2 = (j != 0);
}

void CryoPressureSwitch::SaveState(FILEHANDLE scn)
{
	char buffer[100];

	sprintf(buffer, "%d %d", PressureSwitch1, PressureSwitch2);
	oapiWriteScenario_string(scn, "CRYOPRESSSWITCH", buffer);
}