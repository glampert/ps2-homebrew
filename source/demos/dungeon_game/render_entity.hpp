
// ================================================================================================
// -*- C++ -*-
// File: render_entity.hpp
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

#ifndef RENDER_ENTITY_HPP
#define RENDER_ENTITY_HPP

#include "framework/common.hpp"
#include "framework/renderer.hpp"
#include "framework/md2_model.hpp"
class Frustum;

// ========================================================
// enum ModelId:
// ========================================================

enum ModelId
{
	// Player and weapon(s):
	MDL_PLAYER,
	MDL_PLAYER_WEAPON,

	// Enemy, bosses and enemy weapons:
	MDL_ENEMY,
	MDL_BOSS,
	MDL_BOSS_WEAPON,

	// Scene props:
	MDL_BARREL,
	MDL_TORCH,
	MDL_SKELETON,
	MDL_HANGING_CORPSE,
	MDL_SPIRAL_STAIRCASE,
	MDL_TOMBSTONE_1,
	MDL_TOMBSTONE_2,
	MDL_GUILLOTINE,
	MDL_DEAD_TREE,
	MDL_BANNER,
	MDL_STATUE,

	// Internal use.
	MODEL_COUNT
};

// ========================================================
// class LightShadowBlob:
// ========================================================

class LightShadowBlob
{
public:

	// This type represents either a pseudo-lightmap or a shadow-blob plane.
	enum Type { NONE, LIGHTMAP, SHADOW_BLOB, CUSTOM };

	Vector position;
	mutable float rotationY;
	mutable float sizeMax;
	mutable float sizeCurr;
	const Texture * texture;
	Type type;

	// Sets all members to defaults.
	LightShadowBlob();
	void reset(Type t);

	// Rendering & visibility test:
	void draw(const Color4f * tint = nullptr) const;
	bool isVisible(const Frustum & frustum) const;
};

// ========================================================
// class RenderEntity:
// ========================================================

class RenderEntity
{
public:

	RenderEntity();
	void reset();
	void draw(const Color4f * tint = nullptr) const;

	// Access the model's AABB or draw it:
	void drawBounds() const;
	const Aabb & getBounds() const;

	// Misc render states:
	void setModel(ModelId mdlId);
	void setModel(const Md2Model * mdl, const Texture * tex);
	void setAnimation(StdMd2AnimId animId);
	void setColorTint(const Color4f & tint);

	// LightMap / ShadowBlob:
	void setLightShadowRenderer(LightShadowBlob * renderer);
	void setLightShadowPosition(const Vector & pos);
	void setLightShadowSize(float blobSize);
	void translateLightShadow(float x, float z);
	void disableLightShadowRenderer() { lightOrShadow->type = LightShadowBlob::NONE; }
	bool hasLightShadowRenderer() const { return lightOrShadow != nullptr; }

	// Model matrix:
	void setModelMatrix(const Matrix & m) { modelMatrix = m;    }
	const Matrix & getModelMatrix() const { return modelMatrix; }

	// World position:
	void setWorldPosition(const Vector & pos) { worldPos = pos;  }
	const Vector & getWorldPosition() const   { return worldPos; }

	// Query animation states:
	bool isStaticModel() const    { return !isAnimated; }
	void setStaticModel()         { isAnimated = false; }
	Md2AnimState & getAnimState() { return animState;   }

	// Visibility test of the model's AABB against the view frustum.
	bool isVisible(const Frustum & frustum) const;

private:

	// Copy/assign disallowed.
	RenderEntity(const RenderEntity &);
	RenderEntity & operator = (const RenderEntity &);

	// Ref to global model and texture data:
	const Md2Model * model;
	const Texture  * texture;

	// Pointer to `tempMd2Verts`, stored locally in each
	// instance to avoid accessing the global when rendering.
	DrawVertex * vbPtr;

	// Size of the original texture.
	// Some textures are re-scaled to fit out 256^2 limit
	// so we need the original size to ensure the MD2 is
	// texture-mapped properly.
	Vec2i originalTexSize;

	// Color modulated with the model's texture (AKA tint color).
	Color4f colorTint;

	// MD2 animation state. Relevant only if the model has animations.
	mutable Md2AnimState animState;
	bool isAnimated;

	// Pseudo-light-map or shadow blob renderer (not owned by RenderEntity):
	LightShadowBlob * lightOrShadow;

	// World position. Currently only used for culling.
	Vector worldPos;

	// Model-to-world matrix, used for rendering.
	Matrix modelMatrix;

	// This pool of temporary vertexes is shared by all MD2 model instances.
	// They are used to store the vertexes generated by animation blending.
	static DrawVertex * tempMd2Verts;
};

#endif // RENDER_ENTITY_HPP
