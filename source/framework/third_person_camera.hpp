
// ================================================================================================
// -*- C++ -*-
// File: third_person_camera.hpp
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

#ifndef THIRD_PERSON_CAMERA_HPP
#define THIRD_PERSON_CAMERA_HPP

#include "camera_base.hpp"

// ========================================================
// class ThirdPersonCamera:
// ========================================================

class ThirdPersonCamera
	: public CameraBase
{
public:

	// Construct with defaults.
	ThirdPersonCamera();

	// Resets to its starting position/orientation.
	void reset();

	// Per-frame camera update.
	void update(GamePad & gamePad, const Vector & focusPoint);

	// Zoom in and out towards the focus point (the game character):
	void zoomIn();
	void zoomOut();

	// Rotate up/down (camera pitch). Returns false if one of the pitch constraints was reached.
	void pitchUp();
	void pitchDown();

	// Rotate right/left (camera yaw):
	void rotateRight();
	void rotateLeft();

	// Generate a view matrix from the camera space.
	Matrix getViewMatrix() const;

	// This method returns what the camera is looking at, the camera target or "look-at" vector.
	Vector getCameraTarget() const;

	// Setters:
	void setRightVec(const Vector & rightVec)     { right   = rightVec;   }
	void setUpVec(const Vector & upVec)           { up      = upVec;      }
	void setForwardVec(const Vector & forwardVec) { forward = forwardVec; }
	void setEyePosition(const Vector & eyePos)    { eye     = eyePos;     }

	// Getters:
	const Vector & getRightVec()    const { return right;   }
	const Vector & getUpVec()       const { return up;      }
	const Vector & getForwardVec()  const { return forward; }
	const Vector & getEyePosition() const { return eye;     }

	float getTotalYaw()   const { return totalYaw;   }
	float getTotalPitch() const { return totalPitch; }

	bool isThirdPerson() const { return true;  }
	bool isFirstPerson() const { return false; }

private:

	// Internal helpers:
	void zoom(float amount);
	void rotateAroundCameraY(float angle);
	void padInput(GamePad & gamePad);

	// Angles are all in degrees:
	float yawDegrees;
	float totalYaw;
	float yawRate;
	float pitchDegrees;
	float totalPitch;
	float pitchRate;

	// Camera vectors and origin:
	Vector right;
	Vector up;
	Vector forward;
	Vector eye;
	Vector lastFocusPoint;

	// Misc:
	float cameraDistScale;
	float zoomRate;
};

#endif // THIRD_PERSON_CAMERA_HPP
