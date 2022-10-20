
// ================================================================================================
// -*- C++ -*-
// File: first_person_camera.hpp
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

#ifndef FIRST_PERSON_CAMERA_HPP
#define FIRST_PERSON_CAMERA_HPP

#include "camera_base.hpp"

// ========================================================
// class FirstPersonCamera:
// ========================================================

class FirstPersonCamera
	: public CameraBase
{
public:

	// Valid directions to move the camera in:
	enum MoveDir
	{
		FORWARD, BACK,
		RIGHT,   LEFT,
		UP,      DOWN
	};

	// Construction:
	FirstPersonCamera();

	// Resets to its starting position/orientation.
	void reset();

	// Per-frame camera update. GamePad should have already been itself updated.
	void update(GamePad & gamePad, const Vector & focusPoint);

	// Pitches around its X-axis by an angle in radians.
	void pitch(float radians);

	// Rotates around its Y-axis by an angle in radians.
	void rotate(float radians);

	// Moves the camera by the given direction, using the current movement speed.
	// The last three parameters indicate in which axis to move. If it is equal to 1,
	// move in that axis, if it is zero don't move.
	void move(MoveDir dir);

	// Generate a view matrix from the camera space.
	Matrix getViewMatrix() const;

	// This method returns what the camera is looking at,
	// the camera target or "look-at" vector.
	Vector getCameraTarget() const;

	// Setters:
	void setRightVec(const Vector & rightVec)     { right   = rightVec;    }
	void setUpVec(const Vector & upVec)           { up      = upVec;       }
	void setForwardVec(const Vector & forwardVec) { forward = forwardVec;  }
	void setEyePosition(const Vector & eyePos)    { eye     = eyePos;      }
	void setMovementSpeed(const float speed)      { movementSpeed = speed; }
	void setTurnSpeed(const float speed)          { turnSpeed     = speed; }

	// Getters:
	const Vector & getRightVec()    const { return right;         }
	const Vector & getUpVec()       const { return up;            }
	const Vector & getForwardVec()  const { return forward;       }
	const Vector & getEyePosition() const { return eye;           }
	float getMovementSpeed()        const { return movementSpeed; }
	float getTurnSpeed()            const { return turnSpeed;     }

	bool isThirdPerson() const { return false; }
	bool isFirstPerson() const { return true;  }

private:

	// Look around using the analog joysticks of the GamePad/Controller.
	void joystickLook(const padlib::PadButtonStates & buttons);

	// Camera states:
	float  movementSpeed;    // Movement amount to displace by in `camera.move()`.
	float  turnSpeed;        // Speed with which to turn the camera around (look).
	float  pitchDegrees;     // Current up/down pitch.
	float  yawDegrees;       // Current let/right rotation.
	float  lastPitchDegrees; // Up/down pitch history (in degrees).
	Vector right;            // The normalized axis that points to the "right".
	Vector up;               // The normalized axis that points "up".
	Vector forward;          // The normalized axis that points "forward".
	Vector eye;              // The position of the camera (camera's eye and the origin of the camera's coordinate system).
};

#endif // FIRST_PERSON_CAMERA_HPP
