/* Copyright (c) 2016 Benjamin Gwin
*
* This file is licensed under the terms of the GPLv3.
* See the included license file for details.
*/

#pragma once

#include <Windows.h>
#include <dinput.h>

struct BongoControllerState
{
	bool b1, b2, b3, b4, b5;
	WORD clap;

	bool any()
	{
		return b1 || b2 || b3 || b4 || b5;
	}

	bool operator==(const BongoControllerState& other)
	{
		return b1 == other.b1 && b2 == other.b2 && b3 == other.b3 &&
			b4 == other.b4 && b5 == other.b5 && clap == other.clap;
	}
};

class BongoController
{
	LPDIRECTINPUT8 dinputInst;
	IDirectInputDevice8* dinputDev;
public:
	BongoController(HWND wnd);
	~BongoController();
	void getState(BongoControllerState& state);
};