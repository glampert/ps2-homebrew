
// ================================================================================================
// -*- C++ -*-
// File: render_entity.cpp
// Author: Guilherme R. Lampert
// Created on: 29/03/15
// Brief: Basic rendering data for a game entity.
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

#include "render_entity.hpp"
#include "tile_map.hpp"

#include "framework/game_time.hpp"
#include "framework/ps2_math/frustum.hpp"

// ================================================================================================
// Built-in MD2 model data (include files generate with `bin2c`):
// ================================================================================================

namespace
{

// ========================================================
// Local helper macros:
// ========================================================

#define DECLARE_MDL_DATA(modelName) \
	Md2Model md2obj_ ## modelName;  \
	Texture  texobj_ ## modelName

#define REGISTER_MDL_DATA(modelName, scale, texSizeX, texSizeY) \
	{ &md2obj_ ## modelName, &texobj_ ## modelName, scale, { texSizeX, texSizeY } },

#define INIT_MDL_DATA(mdlId, modelName) \
do { \
	if (!md2obj_ ## modelName.initFromMemory(modelName, size_ ## modelName, \
		md2Models[mdlId].mdlScale)) \
	{ \
		fatalError("Failed to load model \'" # modelName "\'!"); \
	} \
	loadPermanentTexture(texobj_ ## modelName, modelName ## _texture, \
		size_ ## modelName ## _texture, TEXTURE_FUNCTION_MODULATE); \
} while (0)

// ========================================================
// MD2 model data and textures:
// ========================================================

#include "models/player_model.h"
#include "models/player_model_texture.h"
DECLARE_MDL_DATA(player_model);

#include "models/player_weapon_model.h"
#include "models/player_weapon_model_texture.h"
DECLARE_MDL_DATA(player_weapon_model);

#include "models/enemy_model.h"
#include "models/enemy_model_texture.h"
DECLARE_MDL_DATA(enemy_model);

#include "models/boss_model.h"
#include "models/boss_model_texture.h"
DECLARE_MDL_DATA(boss_model);

#include "models/boss_weapon_model.h"
#include "models/boss_weapon_model_texture.h"
DECLARE_MDL_DATA(boss_weapon_model);

#include "models/barrel_model.h"
#include "models/barrel_model_texture.h"
DECLARE_MDL_DATA(barrel_model);

#include "models/torch_model.h"
#include "models/torch_model_texture.h"
DECLARE_MDL_DATA(torch_model);

#include "models/skeleton_model.h"
#include "models/skeleton_model_texture.h"
DECLARE_MDL_DATA(skeleton_model);

#include "models/hanging_corpse_model.h"
#include "models/hanging_corpse_model_texture.h"
DECLARE_MDL_DATA(hanging_corpse_model);

#include "models/spiral_staircase_model.h"
#include "models/spiral_staircase_model_texture.h"
DECLARE_MDL_DATA(spiral_staircase_model);

#include "models/tombstone1_model.h"
#include "models/tombstone1_model_texture.h"
DECLARE_MDL_DATA(tombstone1_model);

#include "models/tombstone2_model.h"
#include "models/tombstone2_model_texture.h"
DECLARE_MDL_DATA(tombstone2_model);

#include "models/guillotine_model.h"
#include "models/guillotine_model_texture.h"
DECLARE_MDL_DATA(guillotine_model);

#include "models/dead_tree_model.h"
#include "models/dead_tree_model_texture.h"
DECLARE_MDL_DATA(dead_tree_model);

#include "models/banner_model.h"
#include "models/banner_model_texture.h"
DECLARE_MDL_DATA(banner_model);

#include "models/statue_model.h"
#include "models/statue_model_texture.h"
DECLARE_MDL_DATA(statue_model);

// REGISTER ALL MODELS & TEXTURES HERE!
const struct
{
	const Md2Model * mdl;
	const Texture  * tex;
	float mdlScale;
	Vec2i texSize;
}
md2Models[MODEL_COUNT] ATTRIBUTE_ALIGNED(16) =
{
	//                 model                    scale      texSizeX,  texSizeY
	REGISTER_MDL_DATA( player_model,            0.025f,    256,       256     ) // MDL_PLAYER
	REGISTER_MDL_DATA( player_weapon_model,     0.027f,    64,        128     ) // MDL_PLAYER_WEAPON
	REGISTER_MDL_DATA( enemy_model,             0.045f,    256,       256     ) // MDL_ENEMY
	REGISTER_MDL_DATA( boss_model,              0.055f,    256,       256     ) // MDL_BOSS
	REGISTER_MDL_DATA( boss_weapon_model,       0.05f,     256,       256     ) // MDL_BOSS_WEAPON
	REGISTER_MDL_DATA( barrel_model,            0.03f,     128,       128     ) // MDL_BARREL
	REGISTER_MDL_DATA( torch_model,             0.03f,     512,       512     ) // MDL_TORCH
	REGISTER_MDL_DATA( skeleton_model,          0.04f,     256,       256     ) // MDL_SKELETON
	REGISTER_MDL_DATA( hanging_corpse_model,    0.037f,    1024,      1024    ) // MDL_HANGING_CORPSE
	REGISTER_MDL_DATA( spiral_staircase_model,  0.025f,    1024,      1024    ) // MDL_SPIRAL_STAIRCASE
	REGISTER_MDL_DATA( tombstone1_model,        0.02f,     184,       197     ) // MDL_TOMBSTONE_1
	REGISTER_MDL_DATA( tombstone2_model,        0.03f,     204,       128     ) // MDL_TOMBSTONE_2
	REGISTER_MDL_DATA( guillotine_model,        0.04f,     1024,      1024    ) // MDL_GUILLOTINE
	REGISTER_MDL_DATA( dead_tree_model,         0.035f,    128,       128     ) // MDL_DEAD_TREE
	REGISTER_MDL_DATA( banner_model,            0.022f,    128,       256     ) // MDL_BANNER
	REGISTER_MDL_DATA( statue_model,            0.036f,    512,       512     ) // MDL_STATUE
};

// ========================================================
// initAllModels():
// ========================================================

void initAllModels()
{
	static bool modelsInitialized = false;
	if (modelsInitialized)
	{
		return; // Do this only once.
	}

	INIT_MDL_DATA( MDL_PLAYER,           player_model           );
	INIT_MDL_DATA( MDL_PLAYER_WEAPON,    player_weapon_model    );
	INIT_MDL_DATA( MDL_ENEMY,            enemy_model            );
	INIT_MDL_DATA( MDL_BOSS,             boss_model             );
	INIT_MDL_DATA( MDL_BOSS_WEAPON,      boss_weapon_model      );
	INIT_MDL_DATA( MDL_BARREL,           barrel_model           );
	INIT_MDL_DATA( MDL_TORCH,            torch_model            );
	INIT_MDL_DATA( MDL_SKELETON,         skeleton_model         );
	INIT_MDL_DATA( MDL_HANGING_CORPSE,   hanging_corpse_model   );
	INIT_MDL_DATA( MDL_SPIRAL_STAIRCASE, spiral_staircase_model );
	INIT_MDL_DATA( MDL_TOMBSTONE_1,      tombstone1_model       );
	INIT_MDL_DATA( MDL_TOMBSTONE_2,      tombstone2_model       );
	INIT_MDL_DATA( MDL_GUILLOTINE,       guillotine_model       );
	INIT_MDL_DATA( MDL_DEAD_TREE,        dead_tree_model        );
	INIT_MDL_DATA( MDL_BANNER,           banner_model           );
	INIT_MDL_DATA( MDL_STATUE,           statue_model           );

	modelsInitialized = true;
	logComment("MD2 models initialized!");
}

} // namespace {}

// ================================================================================================
// LightShadowBlob implementation:
// ================================================================================================

// ========================================================
// LightShadowBlob::LightShadowBlob():
// ========================================================

LightShadowBlob::LightShadowBlob()
{
	reset(NONE);
}

// ========================================================
// LightShadowBlob::reset():
// ========================================================

void LightShadowBlob::reset(const Type t)
{
	position  = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	rotationY = 0.0f;
	sizeMax   = 0.0f;
	sizeCurr  = 0.0f;
	texture   = nullptr;
	type      = t;
}

// ========================================================
// LightShadowBlob::draw():
// ========================================================

void LightShadowBlob::draw(const Color4f * tint) const
{
	float yPos;

	if (type == LIGHTMAP)
	{
		yPos = -0.5f;

		// Wobble the lightmaps to make it look more interesting.
		const float time = gTime.currentTimeSeconds + randomFloat(-0.5f, 0.5f);
		sizeCurr = (ps2math::sin(time) + 6.0f) * sizeMax;
	}
	else // SHADOW_BLOB or CUSTOM
	{
		yPos = -0.4f;
		sizeCurr = sizeMax;
	}

	Vector rgba;
	if (tint != nullptr)
	{
		rgba.x = tint->r;
		rgba.y = tint->g;
		rgba.z = tint->b;
		rgba.w = tint->a;
	}
	else
	{
		rgba.x = 1.0f;
		rgba.y = 1.0f;
		rgba.z = 1.0f;
		rgba.w = 1.0f;
	}

	// CW winding:
	//
	// (-1, +1) +--------+ (+1, +1)
	//          |        |
	//          |        |
	//          |        |
	// (-1, -1) +--------+ (+1, -1)
	//
	// TRI_0 (-1,+1) (+1,+1) (-1,-1)
	// TRI_1 (+1,+1) (+1,-1) (-1,-1)
	//
	const DrawVertex quadVerts[6] =
	{
		// First triangle <xyzw, uv, rgba>:
		{ Vector(-sizeCurr, yPos, sizeCurr, 1.0f),
		  Vector(0.0f, 0.0f, 0.0f, 1.0f), rgba },

		{ Vector(sizeCurr, yPos, sizeCurr, 1.0f),
		  Vector(0.0f, 1.0f, 0.0f, 1.0f), rgba },

		{ Vector(-sizeCurr, yPos, -sizeCurr, 1.0f),
		  Vector(1.0f, 0.0f, 0.0f, 1.0f), rgba },

		// Second triangle <xyzw, uv, rgba>:
		{ Vector(sizeCurr, yPos, sizeCurr, 1.0f),
		  Vector(0.0f, 1.0f, 0.0f, 1.0f), rgba },

		{ Vector(sizeCurr, yPos, -sizeCurr, 1.0f),
		  Vector(1.0f, 1.0f, 0.0f, 1.0f), rgba },

		{ Vector(-sizeCurr, yPos, -sizeCurr, 1.0f),
		  Vector(1.0f, 0.0f, 0.0f, 1.0f), rgba }
	};

	// NOTE: Caller must set proper render states
	// and texture before calling this!

	Matrix modelTranslation, modelRotation;

	if (rotationY != 0.0f)
	{
		modelRotation.makeRotationY(rotationY);
		modelTranslation.makeTranslation(position);
		gRenderer.setModelMatrix(modelRotation * modelTranslation);
	}
	else
	{
		modelTranslation.makeTranslation(position);
		gRenderer.setModelMatrix(modelTranslation);
	}

	gRenderer.drawUnindexedTriangles(quadVerts, arrayLength(quadVerts));
}

// ========================================================
// LightShadowBlob::isVisible():
// ========================================================

bool LightShadowBlob::isVisible(const Frustum & frustum) const
{
	return frustum.testSphere(position, sizeCurr * sizeCurr);
}

// ================================================================================================
// RenderEntity implementation:
// ================================================================================================

// ========================================================
// RenderEntity shared data:
// ========================================================

DrawVertex * RenderEntity::tempMd2Verts = nullptr;
static const uint TEMP_MD2_VERT_COUNT   = 4096; // Default Quake MD2 max was 2048

// ========================================================
// RenderEntity::RenderEntity():
// ========================================================

RenderEntity::RenderEntity()
{
	reset();

	// Init once (never deallocated):
	initAllModels();
	if (tempMd2Verts == nullptr)
	{
		tempMd2Verts = memClearedAlloc<DrawVertex>(MEM_TAG_GEOMETRY, TEMP_MD2_VERT_COUNT);
		logComment("Allocated space for temp MD2 vertexes...");
	}

	vbPtr = tempMd2Verts;
}

// ========================================================
// RenderEntity::reset():
// ========================================================

void RenderEntity::reset()
{
	model         = nullptr;
	texture       = nullptr;
	lightOrShadow = nullptr;
	isAnimated    = false;
	worldPos      = Vector(0.0f, 0.0f, 0.0f, 1.0f);

	colorTint.r = 1.0f;
	colorTint.g = 1.0f;
	colorTint.b = 1.0f;
	colorTint.a = 1.0f;

	originalTexSize.x = Texture::MAX_SIZE;
	originalTexSize.y = Texture::MAX_SIZE;

	animState.clear();
	modelMatrix.makeIdentity();
}

// ========================================================
// RenderEntity::draw():
// ========================================================

void RenderEntity::draw(const Color4f * tint) const
{
	if (model == nullptr)
	{
		return;
	}

	const uint mdlVertCount = model->getTriangleCount() * 3;
	ps2assert(mdlVertCount < TEMP_MD2_VERT_COUNT);
	ps2assert(texture != nullptr);

	if (isAnimated)
	{
		animState.update(gTime.currentTimeSeconds, model->getKeyframeCount());

		model->assembleFrameInterpolated(animState.currFrame, animState.nextFrame, animState.interp,
			originalTexSize.x, originalTexSize.y, (tint != nullptr) ? (*tint) : colorTint, vbPtr);
	}
	else // Simpler path for static models:
	{
		model->assembleFrame(animState.endFrame, originalTexSize.x, originalTexSize.y,
			(tint != nullptr) ? (*tint) : colorTint, vbPtr);
	}

	gRenderer.setTexture(*texture);
	gRenderer.setModelMatrix(modelMatrix);
	gRenderer.drawUnindexedTriangles(vbPtr, mdlVertCount);

	// Objects that have a custom lightmap or shadow won't be rendered
	// by the game world as a batch, so we render them here.
	if (lightOrShadow != nullptr && lightOrShadow->type == LightShadowBlob::CUSTOM)
	{
		gRenderer.setPrimBlending(true);

		if (lightOrShadow->texture != nullptr)
		{
			gRenderer.setTexture(*lightOrShadow->texture);
		}
		lightOrShadow->draw();

		gRenderer.setPrimBlending(false);
	}
}

// ========================================================
// RenderEntity::drawBounds():
// ========================================================

void RenderEntity::drawBounds() const
{
	if (model != nullptr)
	{
		const Aabb & modelAabb = model->getBoundsForFrame(animState.currFrame);
		gRenderer.drawAabb(modelAabb, makeColor4f(0.0f, 1.0f, 1.0f));
	}
}

// ========================================================
// RenderEntity::getBounds():
// ========================================================

const Aabb & RenderEntity::getBounds() const
{
	if (model != nullptr)
	{
		return model->getBoundsForFrame(animState.currFrame);
	}

	static const Aabb defaultAabb(
		Vector(-1.0f, -1.0f, -1.0f, 1.0f),
		Vector( 1.0f,  1.0f,  1.0f, 1.0f)
	);
	return defaultAabb;
}

// ========================================================
// RenderEntity::setModel():
// ========================================================

void RenderEntity::setModel(const ModelId mdlId)
{
	ps2assert(uint(mdlId) < MODEL_COUNT);
	model           = md2Models[mdlId].mdl;
	texture         = md2Models[mdlId].tex;
	originalTexSize = md2Models[mdlId].texSize;
}

// ========================================================
// RenderEntity::setModel():
// ========================================================

void RenderEntity::setModel(const Md2Model * mdl, const Texture * tex)
{
	model   = mdl;
	texture = tex;

	if (texture != nullptr)
	{
		originalTexSize.x = texture->getWidth();
		originalTexSize.y = texture->getHeight();
	}
}

// ========================================================
// RenderEntity::setAnimation():
// ========================================================

void RenderEntity::setAnimation(const StdMd2AnimId animId)
{
	uint start, end, frameRate;

	Md2Model::getStdAnim(animId, start, end, frameRate);
	animState.startFrame = start;
	animState.endFrame   = end;
	animState.nextFrame  = start + 1;
	animState.fps        = frameRate;

	isAnimated = true;
}

// ========================================================
// RenderEntity::setColorTint():
// ========================================================

void RenderEntity::setColorTint(const Color4f & tint)
{
	colorTint = tint;
}

// ========================================================
// RenderEntity::translateLightShadow():
// ========================================================

void RenderEntity::translateLightShadow(const float x, const float z)
{
	ps2assert(lightOrShadow != nullptr);
	lightOrShadow->position.x = x;
	lightOrShadow->position.z = z;
}

// ========================================================
// RenderEntity::setLightShadowRenderer():
// ========================================================

void RenderEntity::setLightShadowRenderer(LightShadowBlob * renderer)
{
	lightOrShadow = renderer;
}

// ========================================================
// RenderEntity::setLightShadowPosition():
// ========================================================

void RenderEntity::setLightShadowPosition(const Vector & pos)
{
	ps2assert(lightOrShadow != nullptr);
	lightOrShadow->position = pos;
}

// ========================================================
// RenderEntity::setLightShadowSize():
// ========================================================

void RenderEntity::setLightShadowSize(const float blobSize)
{
	ps2assert(lightOrShadow != nullptr);
	lightOrShadow->sizeMax = blobSize;
}

// ========================================================
// RenderEntity::isVisible():
// ========================================================

bool RenderEntity::isVisible(const Frustum & frustum) const
{
	if (model == nullptr)
	{
		return false;
	}

	Aabb modelAabb = model->getBoundsForFrame(animState.currFrame);
	modelAabb.mins += worldPos;
	modelAabb.maxs += worldPos;
	return frustum.testAabb(modelAabb);
}
