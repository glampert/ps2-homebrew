
// ================================================================================================
// -*- C++ -*-
// File: ingame_console.cpp
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

#include "ingame_console.hpp"
#include "renderer.hpp"

//
// Default InGameConsole implementation prints to
// the game screen. The "simple" version just forwards
// print() calls to STDOUT. All other functions are no-ops
// for the simple console.
//
#ifndef INGAME_CONSOLE_STDOUT_SIMPLE

// ========================================================
// InGameConsole local constants:
// ========================================================

namespace
{

const BuiltInFontId TEXT_CHAR_FONT     = FONT_CONSOLAS_24;
const uint          TEXT_CHAR_HEIGHT   = 24;
const float         TEXT_START_OFFSET  = 10.0f;
const Color4b       DEFAULT_TEXT_COLOR = { 255, 255, 255, 255 }; // White (fully opaque)
const Color4b       BACKGROUND_COLOR   = { 10,  70,  40,  75  }; // Olive green (semi-transparent)

} // namespace {}

// ========================================================
// InGameConsole::InGameConsole():
// ========================================================

InGameConsole::InGameConsole()
	: currentTextColor(DEFAULT_TEXT_COLOR)
	, currentLine(&lines[0])
	, linesUsed(1) // lines[0] is taken.
{
	memset(lines, 0, sizeof(lines));
}

// ========================================================
// InGameConsole::print():
// ========================================================

void InGameConsole::print(const char * str)
{
	ps2assert(currentLine != nullptr);

	if (str == nullptr || *str == '\0')
	{
		return;
	}

	for (Line * line = currentLine; *str != '\0'; ++str)
	{
		if (*str == '\n' || line->charsUsed == MAX_CHARS_PER_LINE)
		{
			line = allocNewLine();
			// Newline chars are not added to the line buffer.
			if (*str == '\n')
			{
				continue;
			}
		}

		ps2assert(line->charsUsed < MAX_CHARS_PER_LINE);
		Char & lineChar = line->chars[line->charsUsed++];
		lineChar.chr = *str;
		lineChar.r = currentTextColor.r;
		lineChar.g = currentTextColor.g;
		lineChar.b = currentTextColor.b;
		// Alpha channel is ignored to save space.
	}
}

// ========================================================
// InGameConsole::allocNewLine():
// ========================================================

InGameConsole::Line * InGameConsole::allocNewLine()
{
	if (linesUsed == MAX_TEXT_LINES)
	{
		// Remove line 0 (the oldest) and shift the array:
		linesUsed--;
		memmove(&lines[0], &lines[1], linesUsed * sizeof(Line));
	}

	currentLine = &lines[linesUsed++];
	currentLine->charsUsed = 0;
	return currentLine;
}

// ========================================================
// InGameConsole::draw():
// ========================================================

void InGameConsole::draw() const
{
	if (linesUsed <= 1 && lines[0].charsUsed == 0)
	{
		return; // Just one empty line. Draws nothing.
	}

	// Console window background:
	const Rect4i rect = {
		0, 0,
		gRenderer.getScreenWidth(),
		scast<int>(scast<float>(getHeight()) * gRenderer.getGlobalTextScale())
	};
	gRenderer.drawRectFilled(rect, BACKGROUND_COLOR);

	// Draw text lines on top of the window background:
	Vec2f pos = { TEXT_START_OFFSET, TEXT_START_OFFSET };
	for (uint l = 0; l < linesUsed; ++l)
	{
		for (uint c = 0; c < lines[l].charsUsed; ++c)
		{
			Color4b color;
			color.r = lines[l].chars[c].r;
			color.g = lines[l].chars[c].g;
			color.b = lines[l].chars[c].b;
			color.a = 255;
			gRenderer.drawChar(pos, color, TEXT_CHAR_FONT, lines[l].chars[c].chr);
		}

		// Manually break the line, since the '\n' was not added to the Line chars.
		pos.x = TEXT_START_OFFSET;
		gRenderer.drawText(pos, DEFAULT_TEXT_COLOR, TEXT_CHAR_FONT, "\n");
	}
}

// ========================================================
// InGameConsole::clear():
// ========================================================

void InGameConsole::clear()
{
	currentTextColor = DEFAULT_TEXT_COLOR;
	currentLine      = &lines[0];
	linesUsed        = 1; // lines[0] is taken.

	memset(lines, 0, sizeof(lines));
}

// ========================================================
// InGameConsole::getHeight():
// ========================================================

int InGameConsole::getHeight() const
{
	if (linesUsed <= 1 && lines[0].charsUsed == 0)
	{
		return 0; // Just one empty line. Draws nothing.
	}
	return (linesUsed + 1) * TEXT_CHAR_HEIGHT;
}

// ========================================================
// InGameConsole::setTextColor():
// ========================================================

void InGameConsole::setTextColor(const Color4b color)
{
	currentTextColor = color;
}

// ========================================================
// InGameConsole::setTextColor():
// ========================================================

void InGameConsole::setTextColor(const uint32 color)
{
	currentTextColor.r = scast<ubyte>((color & 0xFF000000) >> 24);
	currentTextColor.g = scast<ubyte>((color & 0x00FF0000) >> 16);
	currentTextColor.b = scast<ubyte>((color & 0x0000FF00) >>  8);
	currentTextColor.a = scast<ubyte>( color & 0x000000FF);
}

// ========================================================
// InGameConsole::restoreTextColor():
// ========================================================

void InGameConsole::restoreTextColor()
{
	currentTextColor = DEFAULT_TEXT_COLOR;
}

// ========================================================
// InGameConsole::preloadFonts():
// ========================================================

void InGameConsole::preloadFonts()
{
	if (!gRenderer.isBuiltInFontLoaded(TEXT_CHAR_FONT))
	{
		gRenderer.loadBuiltInFont(TEXT_CHAR_FONT);
	}
}

#else // !INGAME_CONSOLE_STDOUT_SIMPLE

//
// Simple console redirects all print calls to STDOUT.
// Other methods are no-ops.
//
void InGameConsole::print(const char * str)
{
	if (str != nullptr && *str != '\0')
	{
		printf("%s", str);
	}
}

// No-op stubs:
InGameConsole::InGameConsole()            { }
int  InGameConsole::getHeight() const     { return 0; }
void InGameConsole::draw()      const     { }
void InGameConsole::clear()               { }
void InGameConsole::setTextColor(Color4b) { }
void InGameConsole::setTextColor(uint32)  { }
void InGameConsole::restoreTextColor()    { }
void InGameConsole::preloadFonts()        { }

#endif // INGAME_CONSOLE_STDOUT_SIMPLE

// ========================================================
// InGameConsole global:
// ========================================================

InGameConsole gConsole;
