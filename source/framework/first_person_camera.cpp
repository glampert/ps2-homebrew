
// ================================================================================================
// -*- C++ -*-
// File: first_person_camera.cpp
// Author: Guilherme R. Lampert
// Created on: 11/03/15
// Brief: Basic First Person 3D camera.
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

#include "first_person_camera.hpp"
#include "game_time.hpp"

// ========================================================

static inline float fixAngle(const float degrees)
{
	float radians = degToRad(degrees);
	if (radians > PS2MATH_TWOPI)
	{
		radians -= PS2MATH_TWOPI;
	}
	else if (radians < -PS2MATH_TWOPI)
	{
		radians += PS2MATH_TWOPI;
	}
	return radians;
}

// ========================================================
// FirstPersonCamera::FirstPersonCamera():
// ========================================================

FirstPersonCamera::FirstPersonCamera()
{
	reset();
}

// ========================================================
// FirstPersonCamera::reset():
// ========================================================

void FirstPersonCamera::reset()
{
	movementSpeed    = 0.005f;
	turnSpeed        = 0.04f;
	pitchDegrees     = 0.0f;
	yawDegrees       = 0.0f;
	lastPitchDegrees = 0.0f;
	right            = Vector(1.0f, 0.0f, 0.0f, 1.0f);
	up               = Vector(0.0f, 1.0f, 0.0f, 1.0f);
	forward          = Vector(0.0f, 0.0f, 1.0f, 1.0f);
	eye              = Vector(0.0f, 0.0f, 0.0f, 1.0f);
}

// ========================================================
// FirstPersonCamera::update():
// ========================================================

void FirstPersonCamera::update(GamePad & gamePad, const Vector & /* focusPoint */)
{
	using namespace padlib;
	const PadButtonStates & buttons = gamePad.getButtonStates();

	// Button movement:
	// - Pad arrows move forward/back | left/right
	// - L3 and R3 move up/down
	//
	if (gamePad.isDown(PAD_UP))
	{
		move(FORWARD);
	}
	else if (gamePad.isDown(PAD_DOWN))
	{
		move(BACK);
	}

	if (gamePad.isDown(PAD_RIGHT))
	{
		move(RIGHT);
	}
	else if (gamePad.isDown(PAD_LEFT))
	{
		move(LEFT);
	}

	if (gamePad.isDown(PAD_L3))
	{
		move(UP);
	}
	else if (gamePad.isDown(PAD_R3))
	{
		move(DOWN);
	}

	// Analog stick movement:
	// - Left  stick: Move forward/back | left/right
	// - Right stick: Look around/rotate the camera
	// NOTE: Controller input range is 0 to 255 for X(h) and Y(v)
	//
	if (buttons.ljoy_v <= 50)
	{
		move(FORWARD);
	}
	else if (buttons.ljoy_v >= 200)
	{
		move(BACK);
	}

	if (buttons.ljoy_h >= 200)
	{
		move(RIGHT);
	}
	else if (buttons.ljoy_h <= 50)
	{
		move(LEFT);
	}

	// Alternate camera rotation with L/R:
	//
	if (gamePad.isDown(PAD_L1))
	{
		rotate(fixAngle(turnSpeed * gTime.deltaTimeMillis));
	}
	else if (gamePad.isDown(PAD_R1))
	{
		rotate(fixAngle(-(turnSpeed * gTime.deltaTimeMillis)));
	}
	else if (gamePad.isDown(PAD_L2))
	{
		pitch(fixAngle(-(turnSpeed * gTime.deltaTimeMillis)));
	}
	else if (gamePad.isDown(PAD_R2))
	{
		pitch(fixAngle(turnSpeed * gTime.deltaTimeMillis));
	}
	else
	{
		// Main thumb-stick rotation:
		joystickLook(buttons);
	}

	// Apply damping to angles:
	const float PITCH_YAW_FRAME_DAMPING = 0.01f; // Tweakable
	pitchDegrees *= (PITCH_YAW_FRAME_DAMPING * gTime.deltaTimeMillis);
	yawDegrees   *= (PITCH_YAW_FRAME_DAMPING * gTime.deltaTimeMillis);
}

// ========================================================
// FirstPersonCamera::joystickLook():
// ========================================================

void FirstPersonCamera::joystickLook(const padlib::PadButtonStates & buttons)
{
	// Map controller input range to -1,+1:
	float deltaX = mapValueRange(scast<float>(buttons.rjoy_h), 0.0f, 255.0f, -1.0f, 1.0f);
	float deltaY = mapValueRange(scast<float>(buttons.rjoy_v), 0.0f, 255.0f, -1.0f, 1.0f);

	// Very rudimentary clamping but avoids the camera
	// from keeping moving due to noise in the system.
	if (ps2math::abs(deltaX) < 0.5f) { deltaX = 0.0f; }
	if (ps2math::abs(deltaY) < 0.5f) { deltaY = 0.0f; }

	// Max degrees of rotation to avoid a lock:
	const float MAX_PITCH_ANGLE = 89.5f;

	// Rotate left/right:
	yawDegrees += (deltaX * turnSpeed * gTime.deltaTimeMillis);
	rotate(fixAngle(-yawDegrees));

	// Calculate amount to rotate (pitch) up/down and clamp it:
	pitchDegrees += (deltaY * turnSpeed * gTime.deltaTimeMillis);
	if ((lastPitchDegrees + pitchDegrees) <= -MAX_PITCH_ANGLE)
	{
		pitchDegrees = -MAX_PITCH_ANGLE - lastPitchDegrees;
		lastPitchDegrees = -MAX_PITCH_ANGLE;
	}
	else if ((lastPitchDegrees + pitchDegrees) >= MAX_PITCH_ANGLE)
	{
		pitchDegrees = MAX_PITCH_ANGLE - lastPitchDegrees;
		lastPitchDegrees = MAX_PITCH_ANGLE;
	}
	else
	{
		lastPitchDegrees += pitchDegrees;
	}

	pitch(fixAngle(pitchDegrees));
}

// ========================================================
// FirstPersonCamera::pitch():
// ========================================================

void FirstPersonCamera::pitch(const float radians)
{
	// Calculate new forward:
	rotateAroundAxis(forward, forward, right, radians);

	// Calculate new camera up vector:
	up = forward.cross(right);
}

// ========================================================
// FirstPersonCamera::rotate():
// ========================================================

void FirstPersonCamera::rotate(const float radians)
{
	const float sinAng = ps2math::sin(radians);
	const float cosAng = ps2math::cos(radians);

	// Save off forward components for computation:
	float xxx = forward.x;
	float zzz = forward.z;

	// Rotate forward vector:
	forward.x = xxx *  cosAng + zzz * sinAng;
	forward.z = xxx * -sinAng + zzz * cosAng;

	// Save off up components for computation:
	xxx = up.x;
	zzz = up.z;

	// Rotate up vector:
	up.x = xxx *  cosAng + zzz * sinAng;
	up.z = xxx * -sinAng + zzz * cosAng;

	// Save off right components for computation:
	xxx = right.x;
	zzz = right.z;

	// Rotate right vector:
	right.x = xxx *  cosAng + zzz * sinAng;
	right.z = xxx * -sinAng + zzz * cosAng;
}

// ========================================================
// FirstPersonCamera::move():
// ========================================================

void FirstPersonCamera::move(const MoveDir dir)
{
	const float amount = movementSpeed * gTime.deltaTimeMillis;
	switch (dir)
	{
	case FORWARD :
		eye.x += forward.x * amount;
		eye.y += forward.y * amount;
		eye.z += forward.z * amount;
		break;

	case BACK :
		eye.x -= forward.x * amount;
		eye.y -= forward.y * amount;
		eye.z -= forward.z * amount;
		break;

	case LEFT :
		eye.x += right.x * amount;
		eye.y += right.y * amount;
		eye.z += right.z * amount;
		break;

	case RIGHT :
		eye.x -= right.x * amount;
		eye.y -= right.y * amount;
		eye.z -= right.z * amount;
		break;

	case UP :
		eye.x += up.x * amount;
		eye.y += up.y * amount;
		eye.z += up.z * amount;
		break;

	case DOWN :
		eye.x -= up.x * amount;
		eye.y -= up.y * amount;
		eye.z -= up.z * amount;
		break;

	default :
		fatalError("Invalid camera MoveDir!");
	}
}

// ========================================================
// FirstPersonCamera::getCameraTarget():
// ========================================================

Vector FirstPersonCamera::getCameraTarget() const
{
	return Vector(eye.x + forward.x, eye.y + forward.y, eye.z + forward.z, 1.0f);
}

// ========================================================
// FirstPersonCamera::getViewMatrix():
// ========================================================

Matrix FirstPersonCamera::getViewMatrix() const
{
	Matrix cameraMatrix;
	cameraMatrix.makeLookAt(eye, getCameraTarget(), up);
	return cameraMatrix;
}
