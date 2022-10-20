
// ================================================================================================
// -*- C++ -*-
// File: game_pad.cpp
// Author: Guilherme R. Lampert
// Created on: 03/02/15
// Brief: Game Pad library for the Playstation Dualshock controller.
//
// License:
//  This source code is released under the MIT License.
//  Copyright (c) 2015 Guilherme R. Lampert.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
// ================================================================================================

#include "game_pad.hpp"

// PS2DEV SDK:
#include <string.h>
#include <sifrpc.h>
#include <sifcmd.h>

// ================================================================================================
// PadLib implementation:
// ================================================================================================

/* NOTES:
 *
 * Following code inside the `padlib` namespace was mostly adapted
 * from the `libpadx.c` source file from the PS2DEV SDK. I have
 * created this alternate version of the library because I had
 * to make a few changes to the source code to adequate it to my
 * needs, so ended up just moving the whole thing to this file
 * and creating my own replacement based on the original.
 */
namespace padlib
{

namespace
{

// ========================================================
// SIF RPC constants:
// ========================================================

enum
{
	PAD_BIND_RPC_ID1        = 0x8000010F,
	PAD_BIND_RPC_ID2        = 0x8000011F,
	PAD_RPCCMD_OPEN         = 0x80000100,
	PAD_RPCCMD_INFO_ACT     = 0x80000102,
	PAD_RPCCMD_INFO_MODE    = 0x80000104,
	PAD_RPCCMD_SET_MMODE    = 0x80000105,
	PAD_RPCCMD_SET_ACTDIR   = 0x80000106,
	PAD_RPCCMD_SET_ACTALIGN = 0x80000107,
	PAD_RPCCMD_GET_BTNMASK  = 0x80000108,
	PAD_RPCCMD_SET_BTNINFO  = 0x80000109
};

// ========================================================
// Local helper types:
// ========================================================

struct PadData
{
	uint  frame;
	ubyte state;
	ubyte reqState;
	ubyte ok;
	ubyte unkn7;
	ubyte data[32];
	uint  length;
	uint  unkn44;
	uint  unkn48;
	uint  unkn52;
	uint  unkn54;
	uint  unkn60;
};

struct PadState
{
	int       open;
	uint      port;
	uint      slot;
	PadData * padData;
	ubyte   * padBuf;
};

// ========================================================
// Local data:
// ========================================================

static ubyte              padBuffer[128] ATTRIBUTE_ALIGNED(128);
static SifRpcClientData_t padRpcData[2]  ATTRIBUTE_ALIGNED(64);
static PadState           padState[2][8] ATTRIBUTE_ALIGNED(64);
static bool               padlibInitialised = false;

// ========================================================
// getDmaStr():
// ========================================================

PadData * getDmaStr(int port, int slot)
{
	PadData * pdata = padState[port][slot].padData;
	SyncDCache(pdata, (ubyte *)pdata + 256);

	if (pdata[0].frame < pdata[1].frame)
	{
		return &pdata[1];
	}
	return &pdata[0];
}

} // namespace {}

// ========================================================
// init():
// ========================================================

bool init()
{
	if (padlibInitialised)
	{
		return true;
	}

	//
	// NOTE: We are using the stock 'padman' and 'sio2man'
	// modules, loaded from system ROM. If by whatever reasons
	// these modules are not available, input will not work!
	//

	if (!isIopModuleLoaded("sio2man"))
	{
		if (!loadExecIopModule("rom0:SIO2MAN"))
		{
			return false;
		}
	}
	else
	{
		logComment("sio2man IOP module already loaded!");
	}

	if (!isIopModuleLoaded("padman"))
	{
		if (!loadExecIopModule("rom0:PADMAN"))
		{
			return false;
		}
	}
	else
	{
		logComment("padman IOP module already loaded!");
	}

	padRpcData[0].server = nullptr;
	padRpcData[1].server = nullptr;

	do
	{
		if (SifBindRpc(&padRpcData[0], PAD_BIND_RPC_ID1, 0) < 0)
		{
			return false;
		}
		nopdelay();
	}
	while (!padRpcData[0].server);

	do
	{
		if (SifBindRpc(&padRpcData[1], PAD_BIND_RPC_ID2, 0) < 0)
		{
			return false;
		}
		nopdelay();
	}
	while (!padRpcData[1].server);

	for (int i = 0; i < 8; ++i)
	{
		padState[0][i].open = 0;
		padState[0][i].port = 0;
		padState[0][i].slot = 0;
		padState[1][i].open = 0;
		padState[1][i].port = 0;
		padState[1][i].slot = 0;
	}

	logComment("PadLib initialized!");
	return (padlibInitialised = true);
}

// ========================================================
// openPort():
// ========================================================

int openPort(int port, int slot, void * padData)
{
	/*
	 * The user should provide a pointer to a 256 byte (2 * sizeof(struct PadData))
	 * 64 byte aligned pad data area for each pad port opened
	 * Return: != 0 => OK
	 */

	PadData * dmaBuf = (PadData *)padData;

	// Check 64 byte alignment:
	if ((u32)padData & 0x3F)
	{
		logError("GamePad buffer is misaligned!");
		return 0;
	}

	// Pad data is double buffered.
	for (int i = 0; i < 2; ++i)
	{
		memset(dmaBuf[i].data, 0xFF, 32); // 'Clear' data area
		dmaBuf[i].frame       = 0;
		dmaBuf[i].length      = 0;
		dmaBuf[i].state       = PAD_STATE_EXECCMD;
		dmaBuf[i].reqState    = PAD_RSTAT_BUSY;
		dmaBuf[i].length      = 0;
	}

	*(u32 *)(&padBuffer[0])  = PAD_RPCCMD_OPEN;
	*(u32 *)(&padBuffer[4])  = port;
	*(u32 *)(&padBuffer[8])  = slot;
	*(u32 *)(&padBuffer[16]) = (u32)padData;

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	padState[port][slot].open    = 1;
	padState[port][slot].padData = (PadData *)padData;
	padState[port][slot].padBuf  = *(ubyte **)(&padBuffer[20]);

	return *(u32 *)(&padBuffer[12]);
}

// ========================================================
// readPadButtonStates():
// ========================================================

int readPadButtonStates(int port, int slot, PadButtonStates * data)
{
	/*
	 * Read pad data.
	 * Result is stored in 'data' which should point to a 32 byte array
	 */

	PadData * pdata = getDmaStr(port, slot);
	memcpy(data, pdata->data, pdata->length);
	return pdata->length;
}

// ========================================================
// readPadState():
// ========================================================

int readPadState(int port, int slot)
{
	/*
	 * Get current pad state
	 * Wait until state == 6 (Ready) before trying to access the pad
	 */

	PadData * pdata = getDmaStr(port, slot);
	ubyte state = pdata->state;

	if (state == PAD_STATE_STABLE)
	{
		if (readPadRequestState(port, slot) == PAD_RSTAT_BUSY)
		{
			return PAD_STATE_EXECCMD;
		}
	}
	return state;
}

// ========================================================
// readPadRequestState():
// ========================================================

ubyte readPadRequestState(int port, int slot)
{
	/*
	 * Get pad request state.
	 */

	PadData * pdata = getDmaStr(port, slot);
	return pdata->reqState;
}

// ========================================================
// setPadRequestState():
// ========================================================

int setPadRequestState(int port, int slot, int state)
{
	/*
	 * Set pad request state (after a param setting)
	 */

	PadData * pdata = getDmaStr(port, slot);
	pdata->reqState = state;
	return 1;
}

// ========================================================
// readPadInfoMode():
// ========================================================

int readPadInfoMode(int port, int slot, int infoMode, int index)
{
	/*
	 * Get pad info (digital (4), dualshock (7), etc..)
	 * ID: 3 - KONAMI GUN
	 *     4 - DIGITAL PAD
	 *     5 - JOYSTICK
	 *     6 - NAMCO GUN
	 *     7 - DUAL SHOCK
	 */

	*(u32 *)(&padBuffer[0])  = PAD_RPCCMD_INFO_MODE;
	*(u32 *)(&padBuffer[4])  = port;
	*(u32 *)(&padBuffer[8])  = slot;
	*(u32 *)(&padBuffer[12]) = infoMode;
	*(u32 *)(&padBuffer[16]) = index;

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	if (*(int *)(&padBuffer[20]) == 1)
	{
		setPadRequestState(port, slot, PAD_RSTAT_BUSY);
	}

	return *(int *)(&padBuffer[20]);
}

// ========================================================
// setPadMainMode():
// ========================================================

int setPadMainMode(int port, int slot, int mode, int lock)
{
	/*
	 * mode = 0 -> Digital
	 * mode = 1 -> Analog/dual shock enabled;
	 * lock = 3 -> Mode not changeable by user
	 */

	*(u32 *)(&padBuffer[0])  = PAD_RPCCMD_SET_MMODE;
	*(u32 *)(&padBuffer[4])  = port;
	*(u32 *)(&padBuffer[8])  = slot;
	*(u32 *)(&padBuffer[12]) = mode;
	*(u32 *)(&padBuffer[16]) = lock;

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	if (*(int *)(&padBuffer[20]) == 1)
	{
		setPadRequestState(port, slot, PAD_RSTAT_BUSY);
	}

	return *(int *)(&padBuffer[20]);
}

// ========================================================
// getPadButtonMask():
// ========================================================

int getPadButtonMask(int port, int slot)
{
	*(u32 *)(&padBuffer[0]) = PAD_RPCCMD_GET_BTNMASK;
	*(u32 *)(&padBuffer[4]) = port;
	*(u32 *)(&padBuffer[8]) = slot;

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	return *(int *)(&padBuffer[12]);
}

// ========================================================
// setPadButtonInfo():
// ========================================================

int setPadButtonInfo(int port, int slot, int buttonInfo)
{
	int val;

	*(u32 *)(&padBuffer[0])  = PAD_RPCCMD_SET_BTNINFO;
	*(u32 *)(&padBuffer[4])  = port;
	*(u32 *)(&padBuffer[8])  = slot;
	*(u32 *)(&padBuffer[12]) = buttonInfo;

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	val = *(int *)(&padBuffer[16]);
	if (val == 1)
	{
		setPadRequestState(port, slot, PAD_RSTAT_BUSY);
	}

	return *(int *)(&padBuffer[16]);
}

// ========================================================
// padHasPressureButtons():
// ========================================================

bool padHasPressureButtons(int port, int slot)
{
	/*
	 * Check if the pad has pressure sensitive buttons
	 */

	const int mask = getPadButtonMask(port, slot);
	return (mask ^ 0x3FFFF) ? false : true;
}

// ========================================================
// padEnterPressMode():
// ========================================================

int padEnterPressMode(int port, int slot)
{
	/*
	 * Pressure sensitive mode ON
	 */

	return setPadButtonInfo(port, slot, 0xFFF);
}

// ========================================================
// padExitPressMode():
// ========================================================

int padExitPressMode(int port, int slot)
{
	/*
	 * Pressure sensitive mode OFF
	 */

	return setPadButtonInfo(port, slot, 0);
}

// ========================================================
// readPadActuatorState():
// ========================================================

ubyte readPadActuatorState(int port, int slot, int actuator, int cmd)
{
	/*
	 * Get actuator status for this controller
	 * If `readPadActuatorState(port, slot, -1, 0) != 0`, the controller has actuators.
	 */

	*(u32 *)(&padBuffer[0])  = PAD_RPCCMD_INFO_ACT;
	*(u32 *)(&padBuffer[4])  = port;
	*(u32 *)(&padBuffer[8])  = slot;
	*(u32 *)(&padBuffer[12]) = actuator;
	*(u32 *)(&padBuffer[16]) = cmd;

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	if (*(int *)(&padBuffer[20]) == 1)
	{
		setPadRequestState(port, slot, PAD_RSTAT_BUSY);
	}

	return *(int *)(&padBuffer[20]);
}

// ========================================================
// setPadActuator():
// ========================================================

int setPadActuator(int port, int slot, ubyte actAlign[6])
{
	/*
	 * Initialize actuators on dual shock controller:
	 * actAlign[0] = 0 enables 'small' engine
	 * actAlign[1] = 1 enables 'big' engine
	 * Set actAlign[2-5] to 0xFF to disable
	 */

	*(u32 *)(&padBuffer[0]) = PAD_RPCCMD_SET_ACTALIGN;
	*(u32 *)(&padBuffer[4]) = port;
	*(u32 *)(&padBuffer[8]) = slot;

	ubyte * ptr = (ubyte *)(&padBuffer[12]);
	for (int i = 0; i < 6; ++i)
	{
		ptr[i] = actAlign[i];
	}

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	if (*(int *)(&padBuffer[20]) == 1)
	{
		setPadRequestState(port, slot, PAD_RSTAT_BUSY);
	}

	return *(int *)(&padBuffer[20]);
}

// ========================================================
// setPadActuatorDirect():
// ========================================================

int setPadActuatorDirect(int port, int slot, ubyte actAlign[6])
{
	/*
	 * Set actuator status
	 * on dual shock controller,
	 * actAlign[0] = 0/1 turns off/on 'small' engine
	 * actAlign[1] = 0-255 sets 'big' engine speed
	 */

	*(u32 *)(&padBuffer[0]) = PAD_RPCCMD_SET_ACTDIR;
	*(u32 *)(&padBuffer[4]) = port;
	*(u32 *)(&padBuffer[8]) = slot;

	ubyte * ptr = (ubyte *)(&padBuffer[12]);
	for (int i = 0; i < 6; ++i)
	{
		ptr[i] = actAlign[i];
	}

	if (SifCallRpc(&padRpcData[0], 1, 0, padBuffer, 128, padBuffer, 128, 0, 0) < 0)
	{
		return 0;
	}

	return *(int *)(&padBuffer[20]);
}

} // namespace padlib {}

// ================================================================================================
// GamePad implementation:
// ================================================================================================

static const char * padTypeStr[] =
{
	"Generic Controller",
	"Mouse",
	"Nejicon",
	"Konami Gun",
	"Digital",
	"Analog",
	"Namco Gun",
	"DualShock"
};

// ========================================================
// GamePad::GamePad():
// ========================================================

GamePad::GamePad(int port)
	: portNum(port)
{
	memset(&buttonStates, 0, sizeof(buttonStates));
	memset(actuatorSpeed, 0, sizeof(actuatorSpeed));
	memset(padData,       0, sizeof(padData));

	padlib::init();
	padlib::openPort(portNum, 0, padData);
}

// ========================================================
// GamePad::defaultSetup():
// ========================================================

void GamePad::defaultSetup()
{
	// Check if DualShock and if so, activate analog mode:
	uint mTable[8] = {0};
	bool dualshock = false;
	const int modeTableNum = padlib::readPadInfoMode(portNum, 0, padlib::PAD_MODETABLE, -1);
	for (int i = 0; i < modeTableNum; ++i)
	{
		mTable[i] = padlib::readPadInfoMode(portNum, 0, padlib::PAD_MODETABLE, i);
	}

	// Works for DualShock2 too:
	if ((mTable[0] == 4) && (mTable[1] == 7) && (modeTableNum == 2))
	{
		dualshock = true;
	}

	// Activate and lock analog mode:
	if (dualshock)
	{
		padlib::setPadMainMode(portNum, 0, padlib::PAD_MMODE_DUALSHOCK, padlib::PAD_MMODE_LOCK);
		synchronize();
	}

	// Set offsets for motor parameters for `setPadActuatorDirect()`:
	const int actuatorCount = padlib::readPadActuatorState(portNum, 0, -1, 0);
	if (actuatorCount > 0)
	{
		ubyte actAlign[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		for (int i = 0; i < actuatorCount; ++i)
		{
			actAlign[i] = i;
		}

		padlib::setPadActuator(portNum, 0, actAlign);
		synchronize();
	}

	// Enable pressure sensitive buttons, if we have any:
	const bool pressureMode = padlib::padHasPressureButtons(portNum, 0);
	if (pressureMode)
	{
		padlib::padEnterPressMode(portNum, 0);
		synchronize();
	}

	const int modeId = padlib::readPadInfoMode(portNum, 0, padlib::PAD_MODECURID, 0);
	logComment("Pad OK: port=%d, dualshock=%d, pressMode=%d, \"%s\".",
			portNum, dualshock, pressureMode, padTypeStr[modeId]);
}

// ========================================================
// GamePad::isBusy():
// ========================================================

bool GamePad::isBusy() const
{
	return padlib::readPadRequestState(portNum, 0) != padlib::PAD_RSTAT_COMPLETE;
}

// ========================================================
// GamePad::isStable():
// ========================================================

bool GamePad::isStable() const
{
	return padlib::readPadState(portNum, 0) == padlib::PAD_STATE_STABLE;
}

// ========================================================
// GamePad::isDisconnected():
// ========================================================

bool GamePad::isDisconnected() const
{
	return padlib::readPadState(portNum, 0) == padlib::PAD_STATE_DISCONN;
}

// ========================================================
// GamePad::isInErrorState():
// ========================================================

bool GamePad::isInErrorState() const
{
	return padlib::readPadState(portNum, 0) == padlib::PAD_STATE_ERROR;
}

// ========================================================
// GamePad::synchronize():
// ========================================================

void GamePad::synchronize() const
{
	while (isBusy())
	{
		nopdelay();
	}

	while (!isStable())
	{
		nopdelay();
	}
}

// ========================================================
// GamePad::update():
// ========================================================

bool GamePad::update()
{
	return padlib::readPadButtonStates(portNum, 0, &buttonStates) != 0;
}

// ========================================================
// GamePad::startActuator():
// ========================================================

void GamePad::startActuator(const uint actuatorNum, const ubyte speed)
{
	ps2assert(actuatorNum < 6);
	if (actuatorSpeed[actuatorNum] != speed)
	{
		actuatorSpeed[actuatorNum] = speed;
		padlib::setPadActuatorDirect(portNum, 0, actuatorSpeed);
		synchronize();
	}
}

// ========================================================
// GamePad::stopActuator():
// ========================================================

void GamePad::stopActuator(const uint actuatorNum)
{
	startActuator(actuatorNum, 0);
}

// ========================================================
// GamePad::isActuatorOn():
// ========================================================

bool GamePad::isActuatorOn(const uint actuatorNum) const
{
	ps2assert(actuatorNum < 6);
	return actuatorSpeed[actuatorNum] != 0;
}

// ========================================================
// initGamePad():
// ========================================================

GamePad * initGamePad(const int port, const bool waitReady, const uint timeoutMsec)
{
	ps2assert(port >= 0 && port < 2);

	static GamePad pad_0(0);
	static GamePad pad_1(1);
	static GamePad * pads[] = { &pad_0, &pad_1 };

	if (!pads[port]->isDisconnected() && !pads[port]->isInErrorState())
	{
		if (waitReady)
		{
			const uint initialTime = clockMilliseconds();
			do
			{
				if (!pads[port]->isBusy() && pads[port]->isStable())
				{
					break;
				}
				nopdelay();
			}
			while ((clockMilliseconds() - initialTime) < timeoutMsec);

			pads[port]->defaultSetup();
		}

		return pads[port];
	}

	logComment("No GamePad found at port #%d!", port);
	return nullptr;
}
