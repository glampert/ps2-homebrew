
// ================================================================================================
// -*- C++ -*-
// File: camera_base.hpp
// Author: Guilherme R. Lampert
// Created on: 03/04/15
// Brief: Minimal interface for a Virtual Camera class.
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

#ifndef CAMERA_BASE_HPP
#define CAMERA_BASE_HPP

#include "common.hpp"
#include "game_pad.hpp"
#include "ps2_math/vector.hpp"
#include "ps2_math/matrix.hpp"

// ========================================================
// class CameraBase:
// ========================================================

class CameraBase
{
public:

	virtual void update(GamePad & gamePad, const Vector & focusPoint) = 0;

	virtual Matrix getViewMatrix()   const = 0;
	virtual Vector getCameraTarget() const = 0;

	virtual const Vector & getUpVec()      const = 0;
	virtual const Vector & getRightVec()   const = 0;
	virtual const Vector & getForwardVec() const = 0;

	virtual const Vector & getEyePosition() const = 0;
	virtual void setEyePosition(const Vector & eyePos) = 0;

	virtual bool isThirdPerson() const = 0;
	virtual bool isFirstPerson() const = 0;

	virtual ~CameraBase() { }
};

#endif // CAMERA_BASE_HPP
