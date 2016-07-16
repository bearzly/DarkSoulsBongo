/* Copyright (c) 2016 Benjamin Gwin
*
* This file is licensed under the terms of the GPLv3.
* See the included license file for details.
*/

#include "BongoController.h"

#include <dinput.h>
#include <iostream>
#include <vector>
#include <cstdio>

using namespace std;

// You will need to replace this with the actual GUID of your own controller
static GUID BongoGUID = {0x9FFDACE0, 0x1CC7, 0x11E5, {0x80,0x01,0x44,0x45,0x53,0x54,0x00,0x00}};

bool guidEqual(GUID* g1, GUID* g2)
{
	return memcmp(g1, g2, sizeof(GUID)) == 0;
}

BOOL CALLBACK enumCb(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	vector<GUID>* vect = (vector<GUID>*)pvRef;
	vect->push_back(lpddi->guidInstance);
	return DIENUM_CONTINUE;
}

BongoController::BongoController(HWND wnd):
	dinputInst(NULL),
	dinputDev(NULL)
{
	HRESULT res = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&dinputInst, NULL);
	if (res != DI_OK)
		throw std::exception("DirectInput8Create failed", res);

	vector<GUID> devices;
	res = dinputInst->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)enumCb, (void*)&devices, DIEDFL_ATTACHEDONLY);
	if (res != DI_OK)
		throw std::exception("EnumDevices failed", res);

	int idx = -1;
	for (size_t i = 0; i < devices.size(); ++i)
	{
		if (guidEqual(&devices[i], &BongoGUID))
		{
			idx = i;
			break;
		}
	}
	if (idx == -1)
		return;

	res = dinputInst->CreateDevice(devices[idx], &dinputDev, NULL);
	if (res != DI_OK)
		throw std::exception("CreateDevice failed", res);

	dinputDev->SetCooperativeLevel(wnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	dinputDev->SetDataFormat(&c_dfDIJoystick);
	dinputDev->Acquire();
}

BongoController::~BongoController()
{
	if (dinputDev)
	{
		dinputDev->Unacquire();
		dinputDev->Release();
	}
	dinputInst->Release();
}

void
BongoController::getState(BongoControllerState& state)
{
	if (!dinputDev)
	{
		memset(&state, 0, sizeof(state));
		return;
	}
	DIJOYSTATE data;
	dinputDev->Poll();
	dinputDev->GetDeviceState(sizeof(data), &data);
	state.b1 = data.rgbButtons[0] != 0;
	state.b2 = data.rgbButtons[1] != 0;
	state.b3 = data.rgbButtons[2] != 0;
	state.b4 = data.rgbButtons[3] != 0;
	state.b5 = data.rgbButtons[9] != 0;
	state.clap = (WORD)data.lRy;
}