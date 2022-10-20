
// ================================================================================================
// -*- C++ -*-
// File: game_entity.hpp
// Author: Guilherme R. Lampert
// Created on: 03/04/15
// Brief: Base game entity and specializations for player and enemies.
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

#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP

#include "render_entity.hpp"
class TileMap;
class GamePad;
class GameWorld;
class CameraBase;
class ParticleEmitter;

// ========================================================
// class GameEntity:
// ========================================================

class GameEntity
{
public:

	GameEntity();
	virtual ~GameEntity();

	//
	// Overridable methods:
	//
	virtual void update() = 0;
	virtual void reset();

	//
	// Get/set helpers:
	//
	void setRenderEntity(RenderEntity * re)   { renderEnt = re;   }
	RenderEntity * getRenderEntity() const    { return renderEnt; }

	void setWorldPosition(const Vector & pos) { worldPosition = pos;  }
	const Vector & getWorldPosition() const   { return worldPosition; }

	void setHealthAmount(const int amount)    { health = amount;    }
	int  getHealthAmount() const              { return health;      }
	bool isDead() const                       { return health <= 0; }

	//
	// Animation control:
	//
	void setAnimation(StdMd2AnimId animId);
	bool currentAnimationFinished() const;
	void resetAnimationStates();

protected:

	// Internal helpers:
	bool orientModel();

	// Common data:
	GameWorld    * world;
	RenderEntity * renderEnt;
	Vector         worldPosition;
	int            health;
	float          yawTotalDegrees;
	float          targetYawDegrees;
	StdMd2AnimId   currAnim;
};

// ========================================================
// class PlayerEntity:
// ========================================================

class PlayerEntity
	: public GameEntity
{
public:

	PlayerEntity();

	void init(GameWorld * gw, TileMap * map, GamePad * pad, CameraBase * cam,
	          RenderEntity * playerRe, RenderEntity * weaponRe, const Vector & pos);

	void update();
	void reset();
	void takeEnemyHit();

	// Player accessors:
	bool isAttacking() const  { return attacking; }
	bool hasWeapon()   const  { return weaponEnt != nullptr; }
	int  getMaxHealthAmount() const { return maxHealth; }
	const Aabb   & getPlayerBounds()   const { return renderEnt->getBounds(); }
	const Aabb   & getWeaponBounds()   const { return weaponEnt->getBounds(); }
	const Vector & getWeaponPosition() const { return weaponEnt->getWorldPosition(); }

private:

	// Copy/assign disallowed.
	PlayerEntity(const PlayerEntity &);
	PlayerEntity & operator = (const PlayerEntity &);

	// Internal helpers:
	bool gamePadInput();
	bool playerMovement(float movementAmount);
	void playerSetAnimation(StdMd2AnimId animId);

	// Player-specific data (not owned by this class):
	TileMap      * worldMap;
	GamePad      * gamePad;
	CameraBase   * camera;
	RenderEntity * weaponEnt;

	// Misc:
	float moveSpeed;
	int   maxHealth;
	bool  attacking;
};

// ========================================================
// class EnemyEntity:
// ========================================================

class EnemyEntity
	: public GameEntity
{
public:

	 EnemyEntity();
	~EnemyEntity();

	void init(GameWorld * gw, RenderEntity * re,
	          const Vector & pos, float rotationY);

	void initBoss(GameWorld * gw, RenderEntity * re, RenderEntity * weaponRe,
	              const Vector & pos, float rotationY);

	void update();
	void reset();

private:

	// Copy/assign disallowed.
	EnemyEntity(const EnemyEntity &);
	EnemyEntity & operator = (const EnemyEntity &);

	// Internal helpers:
	bool playerInRange(const PlayerEntity & player) const;
	bool takeHit(const PlayerEntity & player) const;
	bool attackPosition(const Vector & pos);
	void enemySetAnimation(StdMd2AnimId animId);

	// Enemy properties:
	RenderEntity    * weaponEnt;           // Currently only used by bosses.
	LightShadowBlob * customLightOrShadow; // Currently only used by bosses.
	ParticleEmitter * attackPrt;
	Vector attackTarget;     // In world coords.
	Vector attackDir;        // A vector, not a point.
	float  initialYawOffset; // In degrees.
};

#endif // GAME_ENTITY_HPP
