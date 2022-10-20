
// ================================================================================================
// -*- C++ -*-
// File: game_world.cpp
// Author: Guilherme R. Lampert
// Created on: 29/03/15
// Brief: Game world and simulation data.
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

#include "game_world.hpp"
#include "framework/sound.hpp"
#include "framework/game_time.hpp"
#include "framework/ingame_console.hpp"

// ========================================================
// Local data and constants:
// ========================================================

namespace
{

//
// Common textures:
//
#include "misc/shadow_texture.h"
Texture shadowTexture;

#include "misc/lightmap_texture.h"
Texture lightmapTexture;

#include "misc/power_circle_texture.h"
Texture powerCircleTexture;

#include "misc/particle_fire_texture.h"
Texture particleFireTexture;

#include "misc/particle_star_texture.h"
Texture particleStarTexture;

#include "misc/health_bar_texture.h"
Texture healthBarTexture;

#include "misc/ingame_menu_texture.h"
Texture inGameMenuTexture;

#include "misc/menu_bg_texture.h"
Texture menuBgTexture;

#include "misc/end_game_overlay_texture.h"
Texture endGameOverlayTexture;

//
// Game level/map data:
//
#include "maps/the_dungeons.h"
#include "maps/the_graveyard.h"
#include "maps/test_map.h"

//
// The background soundtrack: A WAV converted to ADPCM with
// `adpenc` (tool provided by the PS2DEV SDK) and then
// dumped as a C-style array of bytes using `bin2c`.
//
// Check the play_adpcm_demo for details on this.
//
#include "misc/chiller_16bit_8k_mono.h"

// ========================================================

// REGISTER ALL LEVELS HERE!
const struct
{
	const char   * mapName;
	const TileId * tileMap;
	PropDesc    ** propMap;
	const uint     mapWidth;
	const uint     mapHeight;
}
allLevels[LEVEL_COUNT] ATTRIBUTE_ALIGNED(16) =
{
	{ dungeonMapName,   dungeonTileMap,   dungeonPropMap,   dungeonMapWidth,   dungeonMapHeight   }, // The Dungeons map
	{ graveyardMapName, graveyardTileMap, graveyardPropMap, graveyardMapWidth, graveyardMapHeight }, // The Graveyard map
	{ testMapName,      testTileMap,      testPropMap,      testMapWidth,      testMapHeight      }, // A small test map with no props
};

// ========================================================

#define PROFILE_ENABLED 1
#include "framework/profile.hpp"

PROFILE_DECL(prof_update,          "Update    ");
PROFILE_DECL(prof_render3d,        "Render 3D ");
PROFILE_DECL(prof_mapDraw,         " Map draw ");
PROFILE_DECL(prof_entDraw,         " Ent draw ");
PROFILE_DECL(prof_lightShadowDraw, " L.S. draw");
PROFILE_DECL(prof_prtDraw,         " PRTs draw");

} // namespace {}

// ========================================================
// GameWorld::GameWorld():
// ========================================================

GameWorld::GameWorld()
	: gamePad(nullptr)
	, currCamera(nullptr)
	, entitiesDrawn(0)
	, shadowsDrawn(0)
	, lightmapsDrawn(0)
	, prtsDrawn(0)
	, drawModelBounds(false)
	, showDevConsole(false)
	, fadeAlpha(0)
	, drawFadeScreen(false)
	, fadeOut(false)
	, fadeIn(false)
	, inMainMenu(true)
	, currLevel(NO_LEVEL)
	, currMainMenuSel(0)
	, enemyEntitiesInUse(0)
	, renderEntitiesInUse(0)
	, shadowBlobsInUse(0)
	, lightmapsInUse(0)
	, prtEmittersInUse(0)
{
	// Init game controller 0 (first slot in the Console). Wait up to
	// 200ms for it to be ready. If we can't connect, terminate immediately.
	gamePad = initGamePad(0, /* waitReady = */ true, /* timeoutMsec = */ 200);
	if (gamePad == nullptr)
	{
		fatalError("Failed to initialize Game Controller 0 (slot 1)! Make sure it is connected.");
	}

	// Start the game with the default third person camera view:
	currCamera = &thirdPersonCamera;

	// Set projection and view matrices:
	//
	viewMatrix.makeIdentity();
	projectionMatrix.makePerspectiveProjection(degToRad(60.0f), gRenderer.getAspectRatio(),
			gRenderer.getScreenWidth(), gRenderer.getScreenHeight(), 2.0f, 2000.0f);

	// Frustum needs a different projection matrix because the one used
	// to render is scaled with the GS virtual screen size.
	//
	frustum.setProjection(degToRad(60.0f), gRenderer.getScreenWidth(),
			gRenderer.getScreenHeight(), 2.0f, 2000.0f);

	// Set correct types for the LightShadowBlobs:
	//
	for (uint s = 0; s < MAX_SHADOW_BLOBS; ++s)
	{
		shadowBlobs[s].type = LightShadowBlob::SHADOW_BLOB;
	}
	for (uint l = 0; l < MAX_LIGHTMAPS; ++l)
	{
		lightmaps[l].type = LightShadowBlob::LIGHTMAP;
	}

	// Load other permanent textures:
	//
	loadPermanentTexture(menuBgTexture,         menu_bg_texture,          size_menu_bg_texture,          TEXTURE_FUNCTION_DECAL);
	loadPermanentTexture(shadowTexture,         shadow_texture,           size_shadow_texture,           TEXTURE_FUNCTION_DECAL);
	loadPermanentTexture(lightmapTexture,       lightmap_texture,         size_lightmap_texture,         TEXTURE_FUNCTION_HIGHLIGHT);
	loadPermanentTexture(powerCircleTexture,    power_circle_texture,     size_power_circle_texture,     TEXTURE_FUNCTION_HIGHLIGHT);
	loadPermanentTexture(particleFireTexture,   particle_fire_texture,    size_particle_fire_texture,    TEXTURE_FUNCTION_MODULATE);
	loadPermanentTexture(particleStarTexture,   particle_star_texture,    size_particle_star_texture,    TEXTURE_FUNCTION_MODULATE);
	loadPermanentTexture(healthBarTexture,      health_bar_texture,       size_health_bar_texture,       TEXTURE_FUNCTION_DECAL);
	loadPermanentTexture(inGameMenuTexture,     ingame_menu_texture,      size_ingame_menu_texture,      TEXTURE_FUNCTION_DECAL);
	loadPermanentTexture(endGameOverlayTexture, end_game_overlay_texture, size_end_game_overlay_texture, TEXTURE_FUNCTION_DECAL);

	// Play the sound loop forever.
	//
	soundSetVolume(SOUND_MAX_VOLUME);
	if (soundPlay(chiller_16bit_8k_mono, size_chiller_16bit_8k_mono))
	{
		logComment("Playing the background sound loop at full volume ...");
	}
	else
	{
		logError("soundPlay() failed: %s", soundGetLastErrorStr());
	}
}

// ========================================================
// GameWorld::updateGameLogic():
// ========================================================

void GameWorld::updateGameLogic()
{
	PROFILE_BEGIN(prof_update);

	// Get user input:
	gamePad->update();

	// Allow opening/closing the developer console with [SELECT]:
	//
	static bool selectBtnDown = false;
	if (gamePad->isDown(padlib::PAD_SELECT))
	{
		selectBtnDown = true;
	}
	else if (gamePad->isUp(padlib::PAD_SELECT) && selectBtnDown)
	{
		selectBtnDown  = false;
		showDevConsole = !showDevConsole;
	}

	// Toggle debug drawing with [START]:
	//
	static bool startBtnDown = false;
	if (gamePad->isDown(padlib::PAD_START))
	{
		startBtnDown = true;
	}
	else if (gamePad->isUp(padlib::PAD_START) && startBtnDown)
	{
		startBtnDown    = false;
		drawModelBounds = !drawModelBounds;
		worldMap.setDrawTileBounds(drawModelBounds);
		logComment("Debug bounds rendering %s.", (drawModelBounds ? "on" : "off"));
	}

	// Exit early when running a screen fade effect.
	//
	if (drawFadeScreen)
	{
		return;
	}

	// Bail out much earlier if inside the menu.
	// The game should not be updated in this case.
	//
	if (inMainMenu)
	{
		static bool upBtnDown   = false;
		static bool downBtnDown = false;

		if (gamePad->isDown(padlib::PAD_UP))
		{
			upBtnDown = true;
		}
		else if (gamePad->isUp(padlib::PAD_UP) && upBtnDown)
		{
			upBtnDown = false;
			--currMainMenuSel;
		}

		if (gamePad->isDown(padlib::PAD_DOWN))
		{
			downBtnDown = true;
		}
		else if (gamePad->isUp(padlib::PAD_DOWN) && downBtnDown)
		{
			downBtnDown = false;
			++currMainMenuSel;
		}

		// Wrap the menu selection:
		if (currMainMenuSel >= LEVEL_COUNT)
		{
			currMainMenuSel = 0;
		}
		else if (currMainMenuSel < 0)
		{
			currMainMenuSel = LEVEL_COUNT - 1;
		}

		// [X] loads the current selected level (any current game is lost!).
		if (gamePad->isDown(padlib::PAD_CROSS))
		{
			loadLevel(scast<LevelId>(currMainMenuSel));
			beginFadeOut();
		}

		return;
	}

	// Back to main menu with [TRIANGLE]:
	//
	static bool triangleBtnDown = false;
	if (gamePad->isDown(padlib::PAD_TRIANGLE) && gamePad->isDown(padlib::PAD_TRIANGLE))
	{
		triangleBtnDown = true;
	}
	else if (gamePad->isUp(padlib::PAD_R3) && gamePad->isUp(padlib::PAD_TRIANGLE) && triangleBtnDown)
	{
		triangleBtnDown = false;
		beginFadeOut();
	}

	// Toggle cameras with [R3]+[L3] pressed simultaneously:
	//
	static bool r3l3BtnDown = false;
	if (gamePad->isDown(padlib::PAD_R3) && gamePad->isDown(padlib::PAD_L3))
	{
		r3l3BtnDown = true;
	}
	else if (gamePad->isUp(padlib::PAD_R3) && gamePad->isUp(padlib::PAD_L3) && r3l3BtnDown)
	{
		r3l3BtnDown = false;
		if (currCamera == &thirdPersonCamera) // First person mode is meant mainly for debugging.
		{
			currCamera = &firstPersonCamera;
			Vector pos = player.getWorldPosition();
			pos.y += 2.0f; pos.z -= 3.0f;
			currCamera->setEyePosition(pos);
		}
		else if (currCamera == &firstPersonCamera)
		{
			currCamera = &thirdPersonCamera;
		}
	}

	// Stop moving the player if we enter the first-person debug mode.
	//
	if (currCamera->isThirdPerson())
	{
		player.update();
	}

	// Camera refresh:
	//
	currCamera->update(*gamePad, player.getWorldPosition());

	// Enemy entity refresh:
	//
	for (uint e = 0; e < enemyEntitiesInUse; ++e)
	{
		if (worldMap.inActiveArea(enemyEntities[e].getWorldPosition()))
		{
			enemyEntities[e].update();
		}
	}

	PROFILE_END(prof_update);
}

// ========================================================
// GameWorld::renderFrame3d():
// ========================================================

void GameWorld::renderFrame3d()
{
	// Main menu is just 2D.
	if (inMainMenu)
	{
		return;
	}

	PROFILE_BEGIN(prof_render3d);
	gRenderer.begin3d();

	const Color4f fadedColorTint = worldMap.getFadedColorTint();
	const Vector  eyePosition    = currCamera->getEyePosition();

	// Render matrices / frustum update:
	//
	viewMatrix = currCamera->getViewMatrix();
	frustum.update(viewMatrix);

	gRenderer.setEyePosition(eyePosition);
	gRenderer.setViewProjMatrix(viewMatrix * projectionMatrix);

	// Tile map rendering:
	//
	PROFILE_BEGIN(prof_mapDraw);
	worldMap.drawMap(frustum);
	PROFILE_END(prof_mapDraw);

	// 3D object / props rendering:
	//
	PROFILE_BEGIN(prof_entDraw);
	for (uint e = 0; e < renderEntitiesInUse; ++e)
	{
		bool inEdge = false;
		const RenderEntity & renderEnt = renderEntities[e];

		if (worldMap.inActiveArea(renderEnt.getWorldPosition(), &inEdge)
			&& renderEnt.isVisible(frustum))
		{
			renderEnt.draw(inEdge ? &fadedColorTint : nullptr);
			if (drawModelBounds)
			{
				renderEnt.drawBounds();
			}
			++entitiesDrawn;
		}
	}
	PROFILE_END(prof_entDraw);

	//
	// Render objects with transparency last:
	//

	gRenderer.setPrimBlending(true);

	PROFILE_BEGIN(prof_lightShadowDraw);

	// Shadow blobs:
	//
	gRenderer.setTexture(shadowTexture);
	for (uint s = 0; s < shadowBlobsInUse; ++s)
	{
		bool inEdge = false;
		const LightShadowBlob & shadow = shadowBlobs[s];

		if (shadow.type != LightShadowBlob::NONE
			&& worldMap.inActiveArea(shadow.position, &inEdge)
			&& !inEdge /* No need to shade objects in the faded edge */
			&& shadow.isVisible(frustum))
		{
			shadow.draw();
			++shadowsDrawn;
		}
	}

	// Light-maps:
	//
	gRenderer.setTexture(lightmapTexture);
	for (uint l = 0; l < lightmapsInUse; ++l)
	{
		bool inEdge = false;
		const LightShadowBlob & lightmap = lightmaps[l];

		if (lightmap.type != LightShadowBlob::NONE
			&& worldMap.inActiveArea(lightmap.position, &inEdge)
			&& lightmap.isVisible(frustum))
		{
			lightmap.draw(inEdge ? &fadedColorTint : nullptr);
			++lightmapsDrawn;
		}
	}

	PROFILE_END(prof_lightShadowDraw);

	// Particle emitters, update and render:
	//
	if (prtEmittersInUse != 0)
	{
		PROFILE_BEGIN(prof_prtDraw);

		gRenderer.disableDepthWriting();
		gRenderer.setModelMatrix(IDENTITY_MATRIX);
		for (uint p = 0; p < prtEmittersInUse; ++p)
		{
			ParticleEmitter & prt = prtEmitters[p];
			if (prt.isEnabled()
				&& worldMap.inActiveArea(prt.getEmitterOrigin())
				&& prt.isVisible(frustum))
			{
				prt.updateParticles();
				prt.drawParticles(viewMatrix, eyePosition);
				++prtsDrawn;
			}
		}
		gRenderer.enableDepthWriting();

		PROFILE_END(prof_prtDraw);
	}

	gRenderer.setPrimBlending(false);

	gRenderer.end3d();
	PROFILE_END(prof_render3d);
}

// ========================================================
// GameWorld::renderMisc2d():
// ========================================================

void GameWorld::renderMisc2d()
{
	if (!inMainMenu && !showDevConsole && !drawModelBounds && !drawFadeScreen)
	{
		return; // No 2D drawing to be done here.
	}

	gRenderer.begin2d();

	// Basic main menu logic:
	if (inMainMenu)
	{
		Vec2f pos = { 20.0f, 25.0f };
		const float oldTextScale = gRenderer.getGlobalTextScale(); // Restored at the end

		// Title:
		//
		gRenderer.setGlobalTextScale(1.4f);
		gRenderer.drawText(pos, makeColor4b(80, 40, 5),
			FONT_CONSOLAS_24, "A Dungeon Game - Created by Lampert\n\n");

		// Level list:
		//
		gRenderer.setGlobalTextScale(1.2f);
		gRenderer.drawText(pos, makeColor4b(80, 40, 5),
			FONT_CONSOLAS_24, "+ Choose a level and press [X]\n\n");

		for (int l = 0; l < LEVEL_COUNT; ++l)
		{
			drawMainMenuOpt(pos, allLevels[l].mapName, (l == currMainMenuSel) ? true : false);
		}

		// Restore previous text scale:
		gRenderer.setGlobalTextScale(oldTextScale);
	}

	// Screen fade-in/out effect:
	//
	if (drawFadeScreen)
	{
		if (fadeOut)
		{
			if (fadeAlpha == 255)
			{
				fadeOut = false;
				logComment("Screen fade-out finished!");
				onFadeOutFinished();
			}
			else
			{
				++fadeAlpha;
			}
		}
		else if (fadeIn)
		{
			if (fadeAlpha == 0)
			{
				fadeIn = false;
				logComment("Screen fade-in finished!");
				onFadeInFinished();
			}
			else
			{
				--fadeAlpha;
			}
		}

		const Rect4i scr = { 0, 0, gRenderer.getScreenWidth(), gRenderer.getScreenHeight() };
		gRenderer.drawRectFilled(scr, makeColor4b(0, 0, 0, fadeAlpha));
	}

	//
	// Lastly, draw any developer overlays and debug text:
	//

	if (showDevConsole)
	{
		gConsole.draw();
	}
	else if (drawModelBounds)
	{
		Vec2f pos;
		const Color4b white = { 255, 255, 255, 255 };

		// Frame times and memory stats:
		//
		#if PROFILE_ENABLED
		{
			pos.x = 225.0f;
			pos.y = gRenderer.getScreenHeight() - 168.0f;
			gRenderer.drawText(pos, white, FONT_CONSOLAS_24, "Memory estimates:\n");
			gRenderer.drawText(pos, white, FONT_CONSOLAS_24, getMemTagsStr());

			pos.x = 5.0f;
			pos.y = 68.0f;
			gRenderer.drawText(pos, white, FONT_CONSOLAS_24, "--------------------------\nFrame times:\n");
			PROFILE_PRINT(pos, &prof_update, &prof_render3d, &prof_mapDraw, &prof_entDraw, prof_lightShadowDraw, prof_prtDraw);
		}
		#else // !PROFILE_ENABLED
		pos.x = 5.0f;
		pos.y = 200.0f;
		#endif // PROFILE_ENABLED

		// Misc render stats:
		//
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24,        "--------------------------\n");
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Num tiles         : %d\n", worldMap.getTotalTiles()));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Tiles drawn       : %d\n", worldMap.getTileDrawCount()));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Num entities      : %d\n", renderEntitiesInUse));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Entities drawn    : %d\n", entitiesDrawn));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Shadows drawn     : %d\n", shadowsDrawn));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Lightmaps drawn   : %d\n", lightmapsDrawn));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Prt emitters draw : %d\n", prtsDrawn));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24, format("Draw fade screen  : %s\n", (drawFadeScreen ? "yes" : "no")));
		gRenderer.drawText(pos, white, FONT_CONSOLAS_24,        "--------------------------\n");

		// Reset for next frame.
		entitiesDrawn  = 0;
		shadowsDrawn   = 0;
		lightmapsDrawn = 0;
		prtsDrawn      = 0;
	}

	// Add some extra stats about the renderer when
	// visualizing model bounding boxes.
	//
	if (drawModelBounds)
	{
		gRenderer.drawFrameStats();
		gRenderer.drawFpsCounter();
	}

	gRenderer.end2d();
}

// ========================================================
// GameWorld::drawMainMenuOpt():
// ========================================================

void GameWorld::drawMainMenuOpt(Vec2f & pos, const char * entryName, const bool checked)
{
	if (checked)
	{
		gRenderer.drawText(pos, makeColor4b(110, 65, 10),
			FONT_CONSOLAS_24, format("> %s\n", entryName));
	}
	else
	{
		gRenderer.drawText(pos, makeColor4b(80, 40, 5),
			FONT_CONSOLAS_24, format("  %s\n", entryName));
	}
}

// ========================================================
// GameWorld::renderUiOverlays():
// ========================================================

void GameWorld::renderUiOverlays()
{
	const uint scrW = gRenderer.getScreenWidth();
	const uint scrH = gRenderer.getScreenHeight();

	// Draw the main menu background and bail.
	if (inMainMenu)
	{
		gRenderer.begin2d(&menuBgTexture);
		{
			const Rect4i rc = { 0, 0, scrW, scrH };
			gRenderer.drawRectTextured(rc, makeColor4b(100, 100, 100));
		}
		gRenderer.end2d();
		return;
	}

	// Health bar:
	gRenderer.begin2d(&healthBarTexture);
	{
		Rect4i rc;

		// The actual bar (if hp > 0):
		if (player.getHealthAmount() > 0)
		{
			rc.x      = scrW - 205;
			rc.y      = scrH - 25;
			rc.height = 5;
			rc.width  = scast<int>(mapValueRange(scast<float>(player.getHealthAmount()),
			            0.0f, scast<float>(player.getMaxHealthAmount()), 0.0f, 170.0f));
			gRenderer.drawRectFilled(rc, makeColor4b(200, 0, 0));
		}

		// Background frame sprite:
		rc.x      = scrW - 230;
		rc.y      = scrH - 35;
		rc.width  = 220;
		rc.height = 25;
		gRenderer.drawRectTextured(rc, makeColor4b(255, 255, 255));
	}
	gRenderer.end2d();

	if (!showDevConsole)
	{
		// In-game menu overlays:
		gRenderer.begin2d(&inGameMenuTexture);
		{
			const Rect4i rc = { 10, 5, 100, 40 };
			gRenderer.drawRectTextured(rc, makeColor4b(255, 255, 255));
		}
		gRenderer.end2d();

		// Draw end game message if the player has died:
		if (player.isDead())
		{
			gRenderer.begin2d(&endGameOverlayTexture);
			{
				Rect4i rc;
				rc.x      = (gRenderer.getScreenWidth() / 2) - (endGameOverlayTexture.getWidth() / 2);
				rc.y      = 0;
				rc.width  = endGameOverlayTexture.getWidth();
				rc.height = endGameOverlayTexture.getHeight();
				gRenderer.drawRectTextured(rc, makeColor4b(255, 255, 255));
			}
			gRenderer.end2d();
		}
	}
}

// ========================================================
// GameWorld::beginFadeOut():
// ========================================================

void GameWorld::beginFadeOut()
{
	if (fadeOut == true) { return; }

	drawFadeScreen = true;
	fadeIn         = false;
	fadeOut        = true;
	fadeAlpha      = 0;
}

// ========================================================
// GameWorld::beginFadeIn():
// ========================================================

void GameWorld::beginFadeIn()
{
	if (fadeIn == true) { return; }

	drawFadeScreen = true;
	fadeIn         = true;
	fadeOut        = false;
	fadeAlpha      = 255;
}

// ========================================================
// GameWorld::onFadeOutFinished():
// ========================================================

void GameWorld::onFadeOutFinished()
{
	if (inMainMenu)
	{
		inMainMenu = false;
	}
	else
	{
		inMainMenu = true;
	}

	beginFadeIn();
}

// ========================================================
// GameWorld::onFadeInFinished():
// ========================================================

void GameWorld::onFadeInFinished()
{
	drawFadeScreen = false;
}

// ========================================================
// GameWorld::unloadLevel():
// ========================================================

void GameWorld::unloadLevel()
{
	if (currLevel == NO_LEVEL)
	{
		return; // Nothing loaded.
	}

	logComment("Unloading current level...");

	for (uint e = 0; e < enemyEntitiesInUse; ++e)
	{
		enemyEntities[e].reset();
	}

	for (uint e = 0; e < renderEntitiesInUse; ++e)
	{
		renderEntities[e].reset();
	}

	for (uint s = 0; s < shadowBlobsInUse; ++s)
	{
		shadowBlobs[s].reset(LightShadowBlob::SHADOW_BLOB);
	}

	for (uint l = 0; l < lightmapsInUse; ++l)
	{
		lightmaps[l].reset(LightShadowBlob::LIGHTMAP);
	}

	for (uint p = 0; p < prtEmittersInUse; ++p)
	{
		prtEmitters[p].setDefaults();
	}

	gTime.reset();
	player.reset();
	thirdPersonCamera.reset();
	firstPersonCamera.reset();
	viewMatrix.makeIdentity();
	worldMap.unload();

	currCamera          = &thirdPersonCamera;
	currLevel           = NO_LEVEL;
	entitiesDrawn       = 0;
	shadowsDrawn        = 0;
	lightmapsDrawn      = 0;
	prtsDrawn           = 0;
	enemyEntitiesInUse  = 0;
	renderEntitiesInUse = 0;
	shadowBlobsInUse    = 0;
	lightmapsInUse      = 0;
	prtEmittersInUse    = 0;

	logComment("Level unloaded / GameWorld reset!");
}

// ========================================================
// GameWorld::loadLevel():
// ========================================================

void GameWorld::loadLevel(const char * mapName, const TileId * tileMap, PropDesc * const * propMap,
                          const uint mapWidth, const uint mapHeight)
{
	// If a level is currently loaded, dispose it first.
	unloadLevel();

	if (!worldMap.initFromMemory(tileMap, mapWidth, mapHeight))
	{
		fatalError("Failed to init game map '%s'!", mapName);
	}

	if (propMap == nullptr)
	{
		logComment("Map '%s' has no props!", mapName);
		return;
	}

	Array<PropDesc *> propList;
	propList.reserve(256); // Preallocate some space

	for (uint z = 0; z < mapHeight; ++z)
	{
		for (uint x = 0; x < mapWidth; ++x)
		{
			PropDesc * prop = propMap[x + (z * mapWidth)];
			if (prop != nullptr)
			{
				prop->tx = x * TILE_SIZE;
				prop->tz = z * TILE_SIZE;
				propList.pushBack(prop);
			}
		}
	}

	// Group the ones with same model type to reduce texture changes when drawing:
	propList.sort(&PropDesc::sortByModelType);

	// Actually instantiate the scene props:
	const uint propCount = propList.size();
	for (uint p = 0; p < propCount; ++p)
	{
		const PropDesc * prop = propList[p];
		RenderEntity * renderEnt = allocRenderEntity();

		const Vector renderEntWPos(prop->tx + prop->mpx, prop->mpy, prop->tz + prop->mpz, 1.0f);
		renderEnt->setModel(prop->modelId);
		renderEnt->setWorldPosition(renderEntWPos);

		Matrix modelMatrix;
		modelMatrix.makeTranslation(renderEntWPos);
		renderEnt->setModelMatrix(modelMatrix);

		if (prop->mrx != 0.0f)
		{
			Matrix rotation;
			rotation.makeRotationX(degToRad(prop->mrx));
			renderEnt->setModelMatrix(rotation * renderEnt->getModelMatrix());
		}
		if (prop->mry != 0.0f)
		{
			Matrix rotation;
			rotation.makeRotationY(degToRad(prop->mry));
			renderEnt->setModelMatrix(rotation * renderEnt->getModelMatrix());
		}
		if (prop->mrz != 0.0f)
		{
			Matrix rotation;
			rotation.makeRotationZ(degToRad(prop->mrz));
			renderEnt->setModelMatrix(rotation * renderEnt->getModelMatrix());
		}

		if (prop->lightOrShadow == LightShadowBlob::LIGHTMAP)
		{
			renderEnt->setLightShadowRenderer(allocLightmap());
			renderEnt->setLightShadowPosition(Vector(prop->tx + prop->lsx, prop->lsy, prop->tz + prop->lsz, 1.0f));
			renderEnt->setLightShadowSize(prop->lsSize);
		}
		else if (prop->lightOrShadow == LightShadowBlob::SHADOW_BLOB)
		{
			renderEnt->setLightShadowRenderer(allocShadowBlob());
			renderEnt->setLightShadowPosition(Vector(prop->tx + prop->lsx, prop->lsy, prop->tz + prop->lsz, 1.0f));
			renderEnt->setLightShadowSize(prop->lsSize);
		}

		// Handle special cases:
		//
		if (prop->modelId == MDL_PLAYER)
		{
			// Player carries a weapon with it:
			RenderEntity * weaponRenderEnt = allocRenderEntity();
			weaponRenderEnt->setModel(MDL_PLAYER_WEAPON);
			weaponRenderEnt->setModelMatrix(renderEnt->getModelMatrix());
			weaponRenderEnt->setWorldPosition(renderEntWPos);

			player.init(this, &worldMap, gamePad, currCamera, renderEnt, weaponRenderEnt, renderEntWPos);
			worldMap.updateActiveArea(renderEntWPos);

			logComment("Player entity placed on map.");
		}
		else
		{
			// Enemies also count as props!
			if (prop->modelId == MDL_ENEMY)
			{
				EnemyEntity * enemyEnt = allocEnemyEntity();
				enemyEnt->init(this, renderEnt, renderEntWPos, prop->mry);
			}
			else if (prop->modelId == MDL_BOSS)
			{
				// Enemy bosses have big bad ass weapons!
				RenderEntity * weaponRenderEnt = allocRenderEntity();
				weaponRenderEnt->setModel(MDL_BOSS_WEAPON);
				weaponRenderEnt->setModelMatrix(renderEnt->getModelMatrix());
				weaponRenderEnt->setWorldPosition(renderEntWPos);

				EnemyEntity * bossEnemyEnt = allocEnemyEntity();
				bossEnemyEnt->initBoss(this, renderEnt, weaponRenderEnt, renderEntWPos, prop->mry);
			}
			else if (prop->modelId == MDL_TORCH)
			{
				// Add a fire emitter on top of the torch:
				ParticleEmitter * prt = allocParticleEmitter();
				prt->setMinMaxParticles(300, 400);
				prt->setParticleReleaseAmount(10);
				prt->setParticleReleaseInterval(50);
				prt->setParticleLifeCycle(700);
				prt->setParticleMinSize(0.8f);
				prt->setParticleMaxSize(0.9f);
				prt->setParticleVelocity(Vector(0.0f, 0.1f, 0.0f, 1.0f));
				prt->setWindVelocity(Vector(0.0f, 2.0f, 0.0f, 1.0f));
				prt->setAirResistance(true);
				prt->setVelocityVar(0.8f);
				prt->setParticleBaseColor(makeColor4f(1.0f, 0.9f, 0.9f));
				prt->setParticleSpriteTexture(&particleFireTexture);
				prt->setEmitterOrigin(Vector(renderEntWPos.x, renderEntWPos.y + 1.5f, renderEntWPos.z, 1.0f));
				prt->allocateParticles();
			}

			// Place the prop over a map tile (necessary for player => prop collision tests):
			worldMap.placeSceneProp(renderEnt, prop->tx, prop->tz);
		}
	}

	logComment("Successfully loaded map '%s' and props!", mapName);
}

// ========================================================
// GameWorld::loadLevel():
// ========================================================

void GameWorld::loadLevel(const LevelId lvlId)
{
	ps2assert(uint(lvlId) < LEVEL_COUNT);
	loadLevel(
		allLevels[lvlId].mapName,
		allLevels[lvlId].tileMap,
		allLevels[lvlId].propMap,
		allLevels[lvlId].mapWidth,
		allLevels[lvlId].mapHeight);
	currLevel = lvlId;
}

// ========================================================
// GameWorld::getBuiltInTexture():
// ========================================================

const Texture * GameWorld::getBuiltInTexture(const char * id) const
{
	if (strcmp(id, "particle_fire") == 0) { return &particleFireTexture; }
	if (strcmp(id, "particle_star") == 0) { return &particleStarTexture; }
	if (strcmp(id, "power_circle" ) == 0) { return &powerCircleTexture;  }
	if (strcmp(id, "shadow_blob"  ) == 0) { return &shadowTexture;       }
	if (strcmp(id, "lightmap"     ) == 0) { return &lightmapTexture;     }
	return nullptr;
}

// ========================================================
// GameWorld::allocEnemyEntity():
// ========================================================

EnemyEntity * GameWorld::allocEnemyEntity()
{
	ps2assert(enemyEntitiesInUse < MAX_ENEMY_ENTITIES && "No more EnemyEntity instances!");
	return &enemyEntities[enemyEntitiesInUse++];
}

// ========================================================
// GameWorld::allocRenderEntity():
// ========================================================

RenderEntity * GameWorld::allocRenderEntity()
{
	ps2assert(renderEntitiesInUse < MAX_RENDER_ENTITIES && "No more RenderEntity instances!");
	return &renderEntities[renderEntitiesInUse++];
}

// ========================================================
// GameWorld::allocShadowBlob():
// ========================================================

LightShadowBlob * GameWorld::allocShadowBlob()
{
	ps2assert(shadowBlobsInUse < MAX_SHADOW_BLOBS && "No more ShadowBlob instances!");
	return &shadowBlobs[shadowBlobsInUse++];
}

// ========================================================
// GameWorld::allocLightmap():
// ========================================================

LightShadowBlob * GameWorld::allocLightmap()
{
	ps2assert(lightmapsInUse < MAX_LIGHTMAPS && "No more LightMap instances!");
	return &lightmaps[lightmapsInUse++];
}

// ========================================================
// GameWorld::allocParticleEmitter():
// ========================================================

ParticleEmitter * GameWorld::allocParticleEmitter()
{
	ps2assert(prtEmittersInUse < MAX_PARTICLE_EMITTERS && "No more ParticleEmitter instances!");
	return &prtEmitters[prtEmittersInUse++];
}
