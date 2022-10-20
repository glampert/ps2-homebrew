
// ================================================================================================
// -*- C++ -*-
// File: third_person_camera.cpp
// Author: Guilherme R. Lampert
// Created on: 19/03/15
// Brief: Third person view camera.
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

#include "third_person_camera.hpp"
#include "game_time.hpp"

// ========================================================
// ThirdPersonCamera::ThirdPersonCamera():
// ========================================================

ThirdPersonCamera::ThirdPersonCamera()
{
	reset();
}

// ========================================================
// ThirdPersonCamera::reset():
// ========================================================

void ThirdPersonCamera::reset()
{
	totalYaw        = 0.0f;
	yawRate         = 0.05f;
	totalPitch      = 0.0f;
	pitchRate       = 0.02f;
	cameraDistScale = 1.0f;
	zoomRate        = 0.001f;
	right           = Vector(1.0f, 0.0f, 0.0f, 1.0f);
	up              = Vector(0.0f, 1.0f, 0.0f, 1.0f);
	forward         = Vector(0.0f, 0.0f, 1.0f, 1.0f);
	eye             = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	lastFocusPoint  = Vector(0.0f, 0.0f, 0.0f, 1.0f);

	// Manual adjustment. Make it face north on initialization.
	yawDegrees   = 250.0f;
	pitchDegrees = 50.0f;
}

// ========================================================
// ThirdPersonCamera::update():
// ========================================================

void ThirdPersonCamera::update(GamePad & gamePad, const Vector & focusPoint)
{
	padInput(gamePad);

	const float yawRadians   = degToRad(yawDegrees);
	const float pitchRadians = degToRad(pitchDegrees);

	Vector focusVector = eye - focusPoint;
	rotateAroundAxis(focusVector, focusVector, up,    yawRadians);
	rotateAroundAxis(focusVector, focusVector, right, pitchRadians);

	const float MAX_CAMERA_DIST = 8.0f;
	const float MIN_CAMERA_DIST = 5.0f;
	focusVector *= cameraDistScale;
	focusVector.clampLength(MIN_CAMERA_DIST, MAX_CAMERA_DIST);

	eye = focusPoint + focusVector;
	lastFocusPoint = focusPoint;

	rotateAroundAxis(forward, forward, right, pitchRadians);
	rotateAroundCameraY(yawRadians);

	// Apply damping to angles:
	const float PITCH_YAW_FRAME_DAMPING = 0.01f; // Tweakable
	pitchDegrees *= (PITCH_YAW_FRAME_DAMPING * gTime.deltaTimeMillis);
	yawDegrees   *= (PITCH_YAW_FRAME_DAMPING * gTime.deltaTimeMillis);
}

// ========================================================
// ThirdPersonCamera::padInput():
// ========================================================

void ThirdPersonCamera::padInput(GamePad & gamePad)
{
	using namespace padlib;
	const PadButtonStates & buttons = gamePad.getButtonStates();

	// Alternate camera left/right rotation:
	// - L1 and R1
	//
	if (gamePad.isDown(PAD_L1))
	{
		rotateLeft();
	}
	else if (gamePad.isDown(PAD_R1))
	{
		rotateRight();
	}
	else // Main rotation with thumbstick:
	{
		if (buttons.rjoy_h >= 200) // Right
		{
			rotateRight();
		}
		else if (buttons.rjoy_h <= 50) // Left
		{
			rotateLeft();
		}
	}

	// Alternate camera pitch (up/down rotation):
	// - L2 and R2
	//
	if (gamePad.isDown(PAD_L2))
	{
		pitchUp();
	}
	else if (gamePad.isDown(PAD_R2))
	{
		pitchDown();
	}
	else // Main rotation with thumbstick:
	{
		if (buttons.rjoy_v <= 50) // Up
		{
			pitchDown();
		}
		else if (buttons.rjoy_v >= 200) // Down
		{
			pitchUp();
		}
	}

	// Zoom camera in/out:
	// - L3 and R3
	//
	if (gamePad.isDown(PAD_L3))
	{
		zoomIn();
	}
	else if (gamePad.isDown(PAD_R3))
	{
		zoomOut();
	}
}

// ========================================================
// ThirdPersonCamera::zoomIn():
// ========================================================

void ThirdPersonCamera::zoomIn()
{
	zoom(zoomRate * gTime.deltaTimeMillis);
}

// ========================================================
// ThirdPersonCamera::zoomOut():
// ========================================================

void ThirdPersonCamera::zoomOut()
{
	zoom(-(zoomRate * gTime.deltaTimeMillis));
}

// ========================================================
// ThirdPersonCamera::pitchUp():
// ========================================================

void ThirdPersonCamera::pitchUp()
{
	pitchDegrees += (pitchRate * gTime.deltaTimeMillis);
	totalPitch   += (pitchRate * gTime.deltaTimeMillis);
	if (totalPitch > 360.0f)
	{
		totalPitch = 360.0f - totalPitch;
	}
}

// ========================================================
// ThirdPersonCamera::pitchDown():
// ========================================================

void ThirdPersonCamera::pitchDown()
{
	pitchDegrees -= (pitchRate * gTime.deltaTimeMillis);
	totalPitch   -= (pitchRate * gTime.deltaTimeMillis);
	if (totalPitch < 0.0f)
	{
		totalPitch = 360.0f - totalPitch;
	}
}

// ========================================================
// ThirdPersonCamera::rotateRight():
// ========================================================

void ThirdPersonCamera::rotateRight()
{
	yawDegrees -= (yawRate * gTime.deltaTimeMillis);
	totalYaw   -= (yawRate * gTime.deltaTimeMillis);
	if (totalYaw < 0.0f)
	{
		totalYaw = 360.0f - totalYaw;
	}
}

// ========================================================
// ThirdPersonCamera::rotateLeft():
// ========================================================

void ThirdPersonCamera::rotateLeft()
{
	yawDegrees += (yawRate * gTime.deltaTimeMillis);
	totalYaw   += (yawRate * gTime.deltaTimeMillis);
	if (totalYaw > 360.0f)
	{
		totalYaw = 360.0f - totalYaw;
	}
}

// ========================================================
// ThirdPersonCamera::getViewMatrix():
// ========================================================

Matrix ThirdPersonCamera::getViewMatrix() const
{
	Matrix cameraMatrix;
	cameraMatrix.makeLookAt(eye, lastFocusPoint, up);
	return cameraMatrix;
}

// ========================================================
// ThirdPersonCamera::getCameraTarget():
// ========================================================

Vector ThirdPersonCamera::getCameraTarget() const
{
	return Vector(eye.x + forward.x, eye.y + forward.y, eye.z + forward.z, 1.0f);
}

// ========================================================
// ThirdPersonCamera::zoom():
// ========================================================

void ThirdPersonCamera::zoom(const float amount)
{
	cameraDistScale = clamp(cameraDistScale + amount, 0.99f, 1.01f);
}

// ========================================================
// ThirdPersonCamera::rotateAroundCameraY():
// ========================================================

void ThirdPersonCamera::rotateAroundCameraY(const float angle)
{
	const float sinAng = ps2math::sin(angle);
	const float cosAng = ps2math::cos(angle);

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
