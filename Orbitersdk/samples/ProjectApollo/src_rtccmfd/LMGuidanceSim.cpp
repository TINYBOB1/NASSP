/****************************************************************************
This file is part of Project Apollo - NASSP
Copyright 2018

LM Guidance Simulation for the RTCC

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

#include "Orbitersdk.h"
#include "OrbMech.h"
#include "LMGuidanceSim.h"

const double AscentGuidance::F = 15297.43;
const double AscentGuidance::Isp = (308.8 * G);
const double AscentGuidance::mu_M = GGRAV * 7.34763862e+22;
const double AscentGuidance::t_2 = 2.0;
const double AscentGuidance::t_3 = 10.0;
const double AscentGuidance::PRLIMIT = -0.1*0.3048*0.3048*0.3048;

AscentGuidance::AscentGuidance()
{
	Q = _V(0, 0, 0);
	t_go = 450.0;
	R_D = 0.0;
	Y_D = 0.0;
	R_D_dot = 0.0;
	Y_D_dot = 0.0;
	Z_D_dot = 0.0;
	FLVP = true;
}

void AscentGuidance::Init(VECTOR3 R_C, VECTOR3 V_C, double m0, double rls, double v_hor, double v_rad)
{
	Q = unit(crossp(R_C, V_C));
	m_dot = F / Isp;
	a_T = F / m0;
	v_e = F / m_dot;
	tau = m0 / m_dot;
	r_LS = rls;
	R_D = r_LS + 60000.0*0.3048;
	Z_D_dot = v_hor;
	R_D_dot = v_rad;
}

void AscentGuidance::Guidance(VECTOR3 R, VECTOR3 V, double M, VECTOR3 &U_FDP, double &ttgo, double &Thrust)
{
	tau = M / m_dot;
	a_T = v_e / tau;

	r = length(R);
	h = r - r_LS;
	U_R = unit(R);
	U_Z = crossp(U_R, Q);
	U_Y = crossp(U_Z, U_R);
	R_dot = dotp(V, U_R);
	Y_dot = dotp(V, U_Y);
	Z_dot = dotp(V, U_Z);
	Y = r * asin(dotp(U_R, Q));
	V_G = U_R * (R_D_dot - R_dot) + U_Y * (Y_D_dot - Y_dot) + U_Z * (Z_D_dot - Z_dot);

	g_eff = pow(length(crossp(R, V)), 2) / pow(r, 3) - mu_M / r / r;
	t_go -= 2.0;
	V_G = V_G - U_R * 0.5*t_go*g_eff;
	t_go = tau * length(V_G) / v_e * (1.0 - 0.5*length(V_G) / v_e);

	if (t_go >= t_2)
	{
		L = log(1.0 - t_go / tau);
		D_12 = tau + t_go / L;

		if (t_go < t_3)
		{
			B = D = 0.0;
		}
		else
		{

			D_21 = t_go - D_12;
			E = t_go / 2.0 - D_21;
			B = (D_21*(R_D_dot - R_dot) - (R_D - r - R_dot * t_go)) / (t_go * E);
			D = (D_21*(Y_D_dot - Y_dot) - (Y_D - Y - Y_dot * t_go)) / (t_go * E);
		}

		if (B > 0)
		{
			B = 0.0;
		}
		else
		{
			if (B < PRLIMIT*tau)
			{
				B = PRLIMIT * tau;
			}
		}
		A = -D_12 * B - (R_D_dot - R_dot) / L;
		C = -D_12 * D - (Y_D_dot - Y_dot) / L;
	}


	a_TR = (A + B) / tau - g_eff;
	a_TY = (C + D) / tau;
	A_H = U_Y * a_TY + U_R * a_TR;
	a_H = length(A_H);
	if (a_H < a_T)
	{
		a_TP = sqrt(a_T*a_T - a_H * a_H)*OrbMech::sign(Z_D_dot - Z_dot);
	}
	else
	{
		a_TP = 0.0;
	}
	A_T = A_H + U_Z * a_TP;
	U_FDP = unit(A_T);
	ttgo = t_go;

	if (FLVP)
	{
		if (h > 25000.0 || R_dot > 40.0*0.3048)
		{
			FLVP = false;
		}
		else
		{
			U_FDP = unit(R);
		}
	}
	Thrust = F;
}

const double AscDescIntegrator::mu_M = GGRAV * 7.34763862e+22;
const double AscDescIntegrator::Isp = (308.8 * G);

AscDescIntegrator::AscDescIntegrator()
{
	dt = 0.0;
	dt_max = 2.0;
}

bool AscDescIntegrator::Integration(VECTOR3 &R, VECTOR3 &V, double &mnow, double &t_total, VECTOR3 U_TD, double t_remain, double Thrust)
{
	dt = min(dt_max, t_remain);
	DVDT = U_TD * Thrust / mnow * dt;
	G_P = gravity(R);
	R = R + (V + G_P * dt / 2.0 + DVDT / 2.0)*dt;
	G_PDT = gravity(R);
	V = V + (G_PDT + G_P)*dt / 2.0 + DVDT;
	mnow -= Thrust / Isp * dt;
	t_total += dt;

	if (t_remain <= dt_max)
	{
		return true;
	}

	return false;
}

VECTOR3 AscDescIntegrator::gravity(VECTOR3 R)
{
	VECTOR3 U_R, g;
	double rr;

	U_R = unit(R);
	rr = dotp(R, R);
	g = -U_R * mu_M / rr;

	return g;
}