
// ================================================================================================
// -*- C++ -*-
// File: game_time.cpp
// Author: Guilherme R. Lampert
// Created on: 19/03/15
// Brief: A simple helper for frame time and delta times calculation.
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

#include "game_time.hpp"

// ========================================================
// GameTime::GameTime():
// ========================================================

GameTime::GameTime()
{
	reset();
}

// ========================================================
// GameTime::beginFrame():
// ========================================================

void GameTime::beginFrame()
{
	t0ms = clockMilliseconds();
	deltaTimeSeconds   = msecToSec(deltaTimeMillis);
	currentTimeSeconds = msecToSec(t0ms);
	currentTimeMillis  = t0ms;
}

// ========================================================
// GameTime::endFrame():
// ========================================================

void GameTime::endFrame()
{
	// Update the time delta:
	t1ms = clockMilliseconds();
	deltaTimeMillis = scast<float>(t1ms - t0ms);
}

// ========================================================
// GameTime::reset():
// ========================================================

void GameTime::reset()
{
	deltaTimeSeconds   = 0.0f;
	deltaTimeMillis    = (1.0f / 60.0f); // Assume initial frame-rate of 60fps
	currentTimeSeconds = 0.0f;
	currentTimeMillis  = 0;
	t0ms               = 0;
	t1ms               = 0;
}

// ========================================================
// GameTime global:
// ========================================================

GameTime gTime;
