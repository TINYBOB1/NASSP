/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005 Mark Grant

  ORBITER vessel module: CSM caution and warning system code.

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

  **************************** Revision History ****************************
  *	$Log$
  *	Revision 1.5  2005/10/11 16:50:01  tschachim
  *	Added more alarms.
  *	
  *	Revision 1.4  2005/08/21 16:23:32  movieman523
  *	Added more alarms.
  *	
  *	Revision 1.3  2005/08/21 13:13:43  movieman523
  *	Wired in a few caution and warning lights.
  *	
  *	Revision 1.2  2005/08/21 11:51:59  movieman523
  *	Initial version of CSM caution and warning lights: light test switch now works.
  *	
  *	Revision 1.1  2005/08/13 14:59:24  movieman523
  *	Added initial null implementation of CSM caution and warning system, and removed 'master alarm' flag from Saturn class.
  *	
  **************************************************************************/


#if !defined(_PA_CSMCAUTIONWARNING_H)
#define _PA_CSMCAUTIONWARNING_H

// moved from Saturn.h as "foreward reference" because of FuelCellBad
typedef struct {
	double H2FlowLBH;
	double O2FlowLBH;
	double TempF;
	double CondenserTempF;
	double CoolingTempF;
} FuelCellStatus;

class CSMCautionWarningSystem : public CautionWarningSystem {

public:
	CSMCautionWarningSystem(Sound &mastersound, Sound &buttonsound);
	void TimeStep(double simt);
	void RenderLights(SURFHANDLE surf, SURFHANDLE lightsurf, bool leftpanel);

protected:

	//
	// Don't need to be saved.
	//

	double NextUpdateTime;
	int TimeStepCount;

	double NextO2FlowCheckTime;
	bool LastO2FlowCheckHigh;
	int O2FlowCheckCount;
	int FuelCellCheckCount[4];

	//
	// Helper functions.
	//

	void RenderLightPanel(SURFHANDLE surf, SURFHANDLE lightsurf, bool *LightState, bool LightTest, int sdx, int sdy);
	bool FuelCellBad(FuelCellStatus fc, int index);
};

//
// Caution and warning light numbers.
//

#define CSM_CWS_CO2_LIGHT			3
#define CSM_CWS_CRYO_PRESS_LIGHT	10
#define CSM_CWS_GLYCOL_TEMP_LOW		11
#define CSM_CWS_FC1_LIGHT			31
#define CSM_CWS_FC2_LIGHT			32
#define CSM_CWS_FC3_LIGHT			33
#define CSM_CWS_CMC_LIGHT			46
#define CSM_CWS_BUS_B_UNDERVOLT		48
#define CSM_CWS_BUS_A_UNDERVOLT		49
#define CSM_CWS_CWS_POWER			51
#define CSM_CWS_O2_FLOW_HIGH_LIGHT	52
#define CSM_CWS_SUIT_COMPRESSOR		53

#endif
