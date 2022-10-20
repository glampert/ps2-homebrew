
// ================================================================================================
// -*- C++ -*-
// File: ingame_console.hpp
// Author: Guilherme R. Lampert
// Created on: 13/01/15
// Brief: Simple in-game developer console to display our log messages in real-time.
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

#ifndef INGAME_CONSOLE_HPP
#define INGAME_CONSOLE_HPP

#include "common.hpp"

// ========================================================
// class InGameConsole:
// ========================================================

class InGameConsole
{
public:

	InGameConsole();

	// Print a null terminated string to the in-game console.
	void print(const char * str);

	// Renders the console chars to screen.
	void draw() const;

	// Clears all text lines. Also resets color.
	void clear();

	// Get the approximate height (in pixels) of the current block of text lines.
	int getHeight() const;

	// Set text color for all subsequent print calls.
	void setTextColor(Color4b color);
	void setTextColor(uint32  color);

	// Restore default text color for all subsequent print calls.
	void restoreTextColor();

	// Ensures console text fonts are already loaded.
	// If this is not explicitly called, fonts are loaded on-demand.
	void preloadFonts();

private:

	// Copy/assign disallowed.
	InGameConsole(const InGameConsole &);
	InGameConsole & operator = (const InGameConsole &);

#ifndef INGAME_CONSOLE_STDOUT_SIMPLE

	// Max sizes of statically allocated arrays.
	// Max chars per line should be adjusted to roughly match
	// the number of chars we can fit in the screen with the
	// font selected for use with the console.
	static const uint MAX_TEXT_LINES     = 20;
	static const uint MAX_CHARS_PER_LINE = 80;

	// One byte for the character itself plus its RGB color.
	struct ATTRIBUTE_PACKED Char
	{
		ubyte chr;
		ubyte r, g, b;
	};

	// A line is a fixed-length array of characters.
	struct ATTRIBUTE_ALIGNED(16) Line
	{
		uint charsUsed;
		Char chars[MAX_CHARS_PER_LINE];
	};

	// Makes room for another line of text,
	// possibly throwing away the oldest line.
	Line * allocNewLine();

	// Text color state:
	Color4b currentTextColor;

	// Pointer to current line in the `lines[]` array. Never null.
	Line * currentLine;

	// Pool of text lines and total used:
	uint linesUsed;
	Line lines[MAX_TEXT_LINES];

#endif // INGAME_CONSOLE_STDOUT_SIMPLE
};

// ========================================================
// InGameConsole global instance:
// ========================================================

extern InGameConsole gConsole;

#endif // INGAME_CONSOLE_HPP
