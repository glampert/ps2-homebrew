
// ================================================================================================
// -*- C++ -*-
// File: game_pad.hpp
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

#ifndef GAME_PAD_HPP
#define GAME_PAD_HPP

#include "common.hpp"

// ========================================================
// PadLib public interface:
// ========================================================

namespace padlib
{

/*
 * Button bits:
 */
enum PadButtonIds
{
	PAD_LEFT     = 0x0080,
	PAD_DOWN     = 0x0040,
	PAD_RIGHT    = 0x0020,
	PAD_UP       = 0x0010,
	PAD_START    = 0x0008,
	PAD_R3       = 0x0004,
	PAD_L3       = 0x0002,
	PAD_SELECT   = 0x0001,
	PAD_SQUARE   = 0x8000,
	PAD_CROSS    = 0x4000,
	PAD_CIRCLE   = 0x2000,
	PAD_TRIANGLE = 0x1000,
	PAD_R1       = 0x0800,
	PAD_L1       = 0x0400,
	PAD_R2       = 0x0200,
	PAD_L2       = 0x0100
};

/*
 * Actuator numbers:
 */
enum PadActuatorIds
{
	PAD_ACT_SMALL_MOTOR = 0,
	PAD_ACT_BIG_MOTOR   = 1
};

/*
 * Pad states:
 */
enum PadStates
{
	PAD_STATE_DISCONN  = 0x00,
	PAD_STATE_FINDPAD  = 0x01,
	PAD_STATE_FINDCTP1 = 0x02,
	PAD_STATE_EXECCMD  = 0x05,
	PAD_STATE_STABLE   = 0x06,
	PAD_STATE_ERROR    = 0x07
};

/*
 * Pad request states:
 */
enum PadRequestStates
{
	PAD_RSTAT_COMPLETE = 0x00,
	PAD_RSTAT_FAILED   = 0x01,
	PAD_RSTAT_BUSY     = 0x02
};

/*
 * Connected pad type:
 */
enum PadTypes
{
	PAD_TYPE_NEJICON   = 0x02,
	PAD_TYPE_KONAMIGUN = 0x03,
	PAD_TYPE_DIGITAL   = 0x04,
	PAD_TYPE_ANALOG	   = 0x05,
	PAD_TYPE_NAMCOGUN  = 0x06,
	PAD_TYPE_DUALSHOCK = 0x07
};

/*
 * Pad Info Mode values:
 */
enum PadInfoModes
{
	PAD_MODECURID   = 1,
	PAD_MODECUREXID = 2,
	PAD_MODECUROFFS = 3,
	PAD_MODETABLE   = 4
};

/*
 * Pad Main Mode values:
 */
enum PadMainModes
{
	PAD_MMODE_DIGITAL   = 0,
	PAD_MMODE_DUALSHOCK = 1,
	PAD_MMODE_UNLOCK    = 0,
	PAD_MMODE_LOCK      = 3
};

/*
 * Pad Info Actuator commands:
 */
enum PadActuatorCmds
{
	PAD_ACTFUNC  = 1,
	PAD_ACTSUB   = 2,
	PAD_ACTSIZE  = 3,
	PAD_ACTCURR  = 4
};

/*
 * Game Pad button states:
 */
struct ATTRIBUTE_PACKED PadButtonStates
{
	// Misc:
	ubyte ok;
	ubyte mode;

	// Bit vector with the up/down state of each button:
	uint16 buttonBits;

	// Joysticks:
	ubyte rjoy_h;
	ubyte rjoy_v;
	ubyte ljoy_h;
	ubyte ljoy_v;

	// Pressure-sensitive button:
	ubyte right_p;
	ubyte left_p;
	ubyte up_p;
	ubyte down_p;
	ubyte triangle_p;
	ubyte circle_p;
	ubyte cross_p;
	ubyte square_p;
	ubyte l1_p;
	ubyte r1_p;
	ubyte l2_p;
	ubyte r2_p;

	// Unused.
	ubyte padding[12];
};

/*
 * Initialize the GamePad library and open a given port.
 * Currently, only ports 0 and 1 are supported.
 */
bool init();
int  openPort(int port, int slot, void * padData);

/*
 * Poll pad connection state and button states.
 */
int   readPadState(int port, int slot);
ubyte readPadRequestState(int port, int slot);
int   readPadButtonStates(int port, int slot, PadButtonStates * data);
int   readPadInfoMode(int port, int slot, int infoMode, int index);

/*
 * Miscellaneous pad state queries and setters.
 */
int setPadRequestState(int port, int slot, int state);
int setPadMainMode(int port, int slot, int mode, int lock);
int getPadButtonMask(int port, int slot);
int setPadButtonInfo(int port, int slot, int buttonInfo);

/*
 * Test if button hardware is pressure sensitive
 * and enable/disable pressure sensors.
 */
bool padHasPressureButtons(int port, int slot);
int  padEnterPressMode(int port, int slot);
int  padExitPressMode(int port, int slot);

/*
 * Manage GamePad actuators, AKA "Force-Feedback" motors.
 */
ubyte readPadActuatorState(int port, int slot, int actuator, int cmd);
int   setPadActuator(int port, int slot, ubyte actAlign[6]);
int   setPadActuatorDirect(int port, int slot, ubyte actAlign[6]);

} // namespace padlib {}

// ========================================================
// class GamePad:
// ========================================================

class ATTRIBUTE_ALIGNED(64) GamePad
{
public:

	// Initializes a GamePad/Controller at a given port. Only ports 0 and 1 are currently supported.
	// The returned pointer belongs to the library, so don't even think about freeing it!
	// This will also ensure PadLib is properly initialized. Returns null if no connection
	// with port can established, which probably means there is no GamePad attached to it.
	friend GamePad * initGamePad(int port, bool waitReady, uint timeoutMsec);

	// Test if a data transfer with the device is still pending.
	bool isBusy() const;

	// Test if connection is stable and working.
	bool isStable() const;

	// Test if connection with the device was lost.
	bool isDisconnected() const;

	// Test if the PAD_STATE_ERROR flag is set.
	bool isInErrorState() const;

	// Blocks until all pending communication with the device is finished.
	void synchronize() const;

	// Refresh pad button/joysticks states.
	// Normally called once per update step of the game.
	// Might fail if pad was disconnected.
	bool update();

	// Default pad initialization. If a pad is connected to
	// a port, `initGamePad()` will call this automatically.
	void defaultSetup();

	// Start/stop actuator motors.
	// 0 = PAD_ACT_SMALL_MOTOR
	// 1 = PAD_ACT_BIG_MOTOR
	// Speed <= 255; 0 is the same as stopping the actuator.
	void startActuator(uint actuatorNum, ubyte speed);
	void stopActuator(uint actuatorNum);
	bool isActuatorOn(uint actuatorNum) const;

	// Get current buttons & joysticks states:
	const padlib::PadButtonStates & getButtonStates() const { return buttonStates; }

	// Test current button press states:
	bool isUp(uint16 btnId)   const { return ((buttonStates.buttonBits ^ 0xFFFF) & btnId) ? false : true;  }
	bool isDown(uint16 btnId) const { return ((buttonStates.buttonBits ^ 0xFFFF) & btnId) ? true  : false; }

private:

	explicit GamePad(int port);

	// Copy/assign disallowed.
	GamePad(const GamePad &);
	GamePad & operator = (const GamePad &);

	// Pad 0 or Pad 1.
	const int portNum;

	// Set of buttons and joysticks for this pad.
	padlib::PadButtonStates buttonStates;

	// Actuator motor speeds.
	ubyte actuatorSpeed[6];

	// Buffer passed to `padlib::openPort()`. Aligned for DMA transfers.
	ubyte padData[256] ATTRIBUTE_ALIGNED(64);
};

#endif // GAME_PAD_HPP
