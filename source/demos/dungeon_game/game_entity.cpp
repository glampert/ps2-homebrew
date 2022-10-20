
// ================================================================================================
// -*- C++ -*-
// File: game_entity.cpp
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

#include "game_entity.hpp"
#include "game_world.hpp"

#include "framework/camera_base.hpp"
#include "framework/game_time.hpp"
#include "framework/game_pad.hpp"

// ========================================================
// A few hardcoded game constants:
// ========================================================

namespace
{

// Squared distance from enemy to player to be eligible for enemy attack.
const float ENEMRY_ATTACK_PLAYER_SQR_DIST = 30.0f;
// Squared distance from player to enemy for the enemy to take player damage.
const float PLAYER_ATTACK_ENEMY_SQR_DIST  = 3.7f;

// Damage player inflicts on enemies:
const int PLAYER_INFLICTED_DAMAGE = 5;
// Damage inflicted on player by enemies:
const int ENEMY_INFLICTED_DAMAGE  = 5;

// Initial and max player health points:
const int PLAYER_MAX_HP = 300;
// Initial and max enemy health points:
const int ENEMY_MAX_HP  = 100;

// The player recovers some health points per enemy kill.
const int PLAYER_HP_RECOVERY_PER_KILL = 10;

} // namespace {}

// ================================================================================================
// GameEntity implementation:
// ================================================================================================

// ========================================================
// GameEntity::GameEntity():
// ========================================================

GameEntity::GameEntity()
	: world(nullptr)
	, renderEnt(nullptr)
	, worldPosition(0.0f, 0.0f, 0.0f, 1.0f)
	, health(0)
	, yawTotalDegrees(0.0f)
	, targetYawDegrees(0.0f)
	, currAnim(MD2ANIM_STAND)
{ }

// ========================================================
// GameEntity::~GameEntity():
// ========================================================

GameEntity::~GameEntity()
{ }

// ========================================================
// GameEntity::reset();
// ========================================================

void GameEntity::reset()
{
	world            = nullptr;
	renderEnt        = nullptr;
	health           = 0;
	yawTotalDegrees  = 0.0f;
	targetYawDegrees = 0.0f;
	currAnim         = MD2ANIM_STAND;
	worldPosition    = Vector(0.0f, 0.0f, 0.0f, 1.0f);
}

// ========================================================
// GameEntity::setAnimation();
// ========================================================

void GameEntity::setAnimation(const StdMd2AnimId animId)
{
	currAnim = animId;
	renderEnt->setAnimation(currAnim);
}

// ========================================================
// GameEntity::currentAnimationFinished():
// ========================================================

bool GameEntity::currentAnimationFinished() const
{
	const Md2AnimState & animState = renderEnt->getAnimState();
	return animState.currFrame >= animState.endFrame;
}

// ========================================================
// GameEntity::resetAnimationStates();
// ========================================================

void GameEntity::resetAnimationStates()
{
	renderEnt->getAnimState().clear();
}

// ========================================================
// GameEntity::orientModel():
// ========================================================

bool GameEntity::orientModel()
{
	if (!ps2math::floatEquals(yawTotalDegrees, targetYawDegrees, 0.9f))
	{
		// Ensure the player doesn't wrap around the longest
		// way when we reach one end of the circle (0 or 360).
		const float delta = targetYawDegrees - yawTotalDegrees;
		if (delta > 180.0f)
		{
			yawTotalDegrees += 360.0f;
		}
		else if (delta < -180.0f)
		{
			yawTotalDegrees -= 360.0f;
		}
		yawTotalDegrees = lerp(yawTotalDegrees, targetYawDegrees, gTime.deltaTimeMillis * 0.01f);
		return true;
	}
	return false;
}

// ================================================================================================
// PlayerEntity implementation:
// ================================================================================================

// ========================================================
// PlayerEntity::PlayerEntity():
// ========================================================

PlayerEntity::PlayerEntity()
	: worldMap(nullptr)
	, gamePad(nullptr)
	, camera(nullptr)
	, weaponEnt(nullptr)
	, moveSpeed(0.004f)
	, attacking(false)
{
	// Manual adjustment required for the player:
	yawTotalDegrees  = 180.0f;
	targetYawDegrees = 180.0f;

	maxHealth = health = PLAYER_MAX_HP;
}

// ========================================================
// PlayerEntity::reset():
// ========================================================

void PlayerEntity::reset()
{
	GameEntity::reset();

	worldMap  = nullptr;
	gamePad   = nullptr;
	camera    = nullptr;
	weaponEnt = nullptr;
	moveSpeed = 0.004f;
	attacking = false;

	// Manual adjustment required for the player:
	yawTotalDegrees  = 180.0f;
	targetYawDegrees = 180.0f;

	maxHealth = health = PLAYER_MAX_HP;
}

// ========================================================
// PlayerEntity::init():
// ========================================================

void PlayerEntity::init(GameWorld * gw, TileMap * map, GamePad * pad, CameraBase * cam,
                        RenderEntity * playerRe, RenderEntity * weaponRe, const Vector & pos)
{
	ps2assert(gw  != nullptr);
	ps2assert(map != nullptr);
	ps2assert(pad != nullptr);
	ps2assert(cam != nullptr);
	ps2assert(playerRe != nullptr);

	world         = gw;
	worldMap      = map;
	gamePad       = pad;
	camera        = cam;
	renderEnt     = playerRe;
	weaponEnt     = weaponRe;
	worldPosition = pos;

	playerSetAnimation(MD2ANIM_STAND);
}

// ========================================================
// PlayerEntity::update():
// ========================================================

void PlayerEntity::update()
{
	if (isDead())
	{
		if (currentAnimationFinished())
		{
			// Once the death animation is finished. Freeze the model.
			renderEnt->setStaticModel();
			if (renderEnt->hasLightShadowRenderer())
			{
				renderEnt->disableLightShadowRenderer();
			}

			// Hide the weapon, if any.
			if (weaponEnt != nullptr)
			{
				weaponEnt->setModel(nullptr, nullptr);
				weaponEnt = nullptr;
			}
		}
		return; // Already dead. No further update.
	}

	// Move/attack/etc:
	bool movedOrRotated = gamePadInput();

	// Adjust yaw angles:
	if (orientModel())
	{
		movedOrRotated = true;
	}

	// Update the RenderEntity if there was a change on position or rotation.
	if (movedOrRotated)
	{
		if (currAnim != MD2ANIM_RUN)
		{
			playerSetAnimation(MD2ANIM_RUN);
		}

		Matrix modelTranslation;
		modelTranslation.makeTranslation(worldPosition);

		Matrix modelRotation;
		modelRotation.makeRotationY(degToRad(yawTotalDegrees));

		const Matrix modelMatrix = modelRotation * modelTranslation;

		renderEnt->setModelMatrix(modelMatrix);
		renderEnt->setWorldPosition(worldPosition);

		renderEnt->translateLightShadow(worldPosition.x, worldPosition.z);

		// Weapon must obviously follow the player.
		if (weaponEnt != nullptr)
		{
			weaponEnt->setModelMatrix(modelMatrix);
			weaponEnt->setWorldPosition(worldPosition);
		}
	}
	else
	{
		if (currAnim != MD2ANIM_STAND)
		{
			// If we are attacking, let it finish first.
			if (currAnim == MD2ANIM_ATTACK)
			{
				if (currentAnimationFinished())
				{
					playerSetAnimation(MD2ANIM_STAND);
				}
			}
			else
			{
				playerSetAnimation(MD2ANIM_STAND);
			}
		}
	}
}

// ========================================================
// PlayerEntity::gamePadInput():
// ========================================================

bool PlayerEntity::gamePadInput()
{
	static bool xBtnDown = false;
	bool movedOrRotated  = false;
	attacking = false;

	// Player can "walk" the world using the arrow/directional
	// buttons or the left thumbstick.
	//
	if (gamePad->isDown(padlib::PAD_UP))
	{
		movedOrRotated = playerMovement(moveSpeed * gTime.deltaTimeMillis);
		targetYawDegrees = 90.0f;
	}
	else if (gamePad->isDown(padlib::PAD_DOWN))
	{
		movedOrRotated = playerMovement(moveSpeed * gTime.deltaTimeMillis);
		targetYawDegrees = 270.0f;
	}
	else if (gamePad->isDown(padlib::PAD_LEFT))
	{
		movedOrRotated = playerMovement(-(moveSpeed * gTime.deltaTimeMillis));
		targetYawDegrees = 180.0f;
	}
	else if (gamePad->isDown(padlib::PAD_RIGHT))
	{
		movedOrRotated = playerMovement(-(moveSpeed * gTime.deltaTimeMillis));
		targetYawDegrees = 360.0f;
	}
	else if (gamePad->isDown(padlib::PAD_CROSS) && !xBtnDown) // [X] to attach once
	{
		if (currAnim != MD2ANIM_ATTACK)
		{
			playerSetAnimation(MD2ANIM_ATTACK);
		}
		xBtnDown  = true;
		attacking = false;
	}
	else if (gamePad->isUp(padlib::PAD_CROSS) && xBtnDown)
	{
		xBtnDown  = false;
		attacking = true;
	}
	else // Main thumbstick movement:
	{
		const padlib::PadButtonStates & buttons = gamePad->getButtonStates();

		if (buttons.ljoy_v <= 50) // Forward/up
		{
			movedOrRotated = playerMovement(moveSpeed * gTime.deltaTimeMillis);
			targetYawDegrees = 90.0f;
		}
		else if (buttons.ljoy_v >= 200) // Back/down
		{
			movedOrRotated = playerMovement(moveSpeed * gTime.deltaTimeMillis);
			targetYawDegrees = 270.0f;
		}

		if (buttons.ljoy_h >= 200) // Right
		{
			movedOrRotated = playerMovement(-(moveSpeed * gTime.deltaTimeMillis));
			targetYawDegrees = 360.0f;
		}
		else if (buttons.ljoy_h <= 50) // Left
		{
			movedOrRotated = playerMovement(-(moveSpeed * gTime.deltaTimeMillis));
			targetYawDegrees = 180.0f;
		}
	}

	return movedOrRotated;
}

// ========================================================
// PlayerEntity::playerMovement():
// ========================================================

bool PlayerEntity::playerMovement(const float movementAmount)
{
	// Calculate direction from angle:
	const float angle = mapValueRange(yawTotalDegrees, 0.0f, 360.0f, -PS2MATH_PI, PS2MATH_PI);
	const float headingX = ps2math::cos(angle) * movementAmount;
	const float headingZ = ps2math::sin(angle) * movementAmount;

	// If the new position would collide with a wall or prop, we don't move.
	const float newX = worldPosition.x + headingX;
	const float newZ = worldPosition.z + headingZ;
	if (worldMap->collidesWithWallOrProp(renderEnt->getBounds(), Vector(newX, worldPosition.y, newZ, 1.0f)))
	{
		return false;
	}

	// Update player world pos:
	worldPosition.x += headingX;
	worldPosition.z += headingZ;

	// Also update camera pos to make it follow the player:
	Vector eyePos = camera->getEyePosition();
	eyePos.x += headingX;
	eyePos.z += headingZ;
	camera->setEyePosition(eyePos);

	// Refresh the world map's active area with the new player pos:
	worldMap->updateActiveArea(worldPosition);
	return true;
}

// ========================================================
// PlayerEntity::playerSetAnimation():
// ========================================================

void PlayerEntity::playerSetAnimation(const StdMd2AnimId animId)
{
	// Custom setAnimation for the player, which includes its weapon.
	GameEntity::setAnimation(animId);
	if (weaponEnt != nullptr)
	{
		weaponEnt->setAnimation(animId);
	}
}

// ========================================================
// PlayerEntity::takeEnemyHit():
// ========================================================

void PlayerEntity::takeEnemyHit()
{
	health -= ENEMY_INFLICTED_DAMAGE;
	if (isDead())
	{
		if (currAnim != MD2ANIM_DEATH_FALL_FORWARD)
		{
			playerSetAnimation(MD2ANIM_DEATH_FALL_FORWARD);
			logWarning("You are dead! Better luck next time...");
		}
	}
}

// ================================================================================================
// EnemyEntity implementation:
// ================================================================================================

// ========================================================
// EnemyEntity::EnemyEntity():
// ========================================================

EnemyEntity::EnemyEntity()
	: weaponEnt(nullptr)
	, customLightOrShadow(nullptr)
	, attackPrt(nullptr)
	, attackTarget(0.0f, 0.0f, 0.0f, 1.0f)
	, attackDir(0.0f, 0.0f, 0.0f, 0.0f)
	, initialYawOffset(0.0f)
{
	health = ENEMY_MAX_HP;
}

// ========================================================
// EnemyEntity::~EnemyEntity():
// ========================================================

EnemyEntity::~EnemyEntity()
{
	// This is owned by the class if not null!
	deleteSingle(MEM_TAG_GENERIC, customLightOrShadow);
}

// ========================================================
// EnemyEntity::reset():
// ========================================================

void EnemyEntity::reset()
{
	GameEntity::reset();

	weaponEnt        = nullptr;
	attackPrt        = nullptr;
	attackTarget     = Vector(0.0f, 0.0f, 0.0f, 1.0f);
	attackDir        = Vector(0.0f, 0.0f, 0.0f, 0.0f);
	initialYawOffset = 0.0f;

	health = ENEMY_MAX_HP;

	// This is owned by the class if not null!
	deleteSingle(MEM_TAG_GENERIC, customLightOrShadow);
}

// ========================================================
// EnemyEntity::init():
// ========================================================

void EnemyEntity::init(GameWorld * gw, RenderEntity * re, const Vector & pos, const float rotationY)
{
	ps2assert(gw != nullptr);
	ps2assert(re != nullptr);

	world         = gw;
	renderEnt     = re;
	worldPosition = pos;

	initialYawOffset = clamp(ps2math::abs(rotationY), 0.0f, 90.0f);
	if (ps2math::floatEquals(initialYawOffset, 0.0f))
	{
		// Not sure exactly why this is needed... Worked on my tests...
		initialYawOffset = 90.0f;
	}

	// Default animation loop:
	enemySetAnimation(MD2ANIM_STAND);

	// Set up the particle emitter used for the attack effects:
	attackPrt = world->allocParticleEmitter();
	attackPrt->setEnabled(false);
	attackPrt->setMinMaxParticles(200, 300);
	attackPrt->setParticleReleaseAmount(100);
	attackPrt->setParticleReleaseInterval(20);
	attackPrt->setParticleLifeCycle(20);
	attackPrt->setParticleMinSize(0.4f);
	attackPrt->setParticleMaxSize(0.7f);
	attackPrt->setParticleVelocity(Vector(0.0f, 0.1f, 0.0f, 1.0f));
	attackPrt->setVelocityVar(0.1f);
	attackPrt->setParticleBaseColor(makeColor4f(0.0f, 1.0f, 0.5f));
	attackPrt->setParticleSpriteTexture(world->getBuiltInTexture("particle_star"));
	attackPrt->setEmitterOrigin(worldPosition);
	attackPrt->allocateParticles();
}

// ========================================================
// EnemyEntity::initBoss():
// ========================================================

void EnemyEntity::initBoss(GameWorld * gw, RenderEntity * re, RenderEntity * weaponRe, const Vector & pos, const float rotationY)
{
	// Init as a generic enemy:
	init(gw, re, pos, rotationY);
	ps2assert(renderEnt != nullptr);

	//
	// Change a few params to make it a "boss" enemy:
	//

	// Bosses have 2x more hp.
	health *= 2;

	// A power circle around the boss enemy:
	customLightOrShadow = new(MEM_TAG_GENERIC) LightShadowBlob;
	customLightOrShadow->position = Vector(pos.x, -1.5f, pos.z, 1.0f);
	customLightOrShadow->texture  = gw->getBuiltInTexture("power_circle");
	customLightOrShadow->type     = LightShadowBlob::CUSTOM;
	customLightOrShadow->sizeMax  = 1.9f;
	renderEnt->setLightShadowRenderer(customLightOrShadow);

	// Make the attack a different color and size:
	attackPrt->setParticleBaseColor(makeColor4f(0.9f, 0.3f, 0.1f));
	attackPrt->setParticleReleaseAmount(200);
	attackPrt->setParticleMinSize(0.8f);
	attackPrt->setParticleMaxSize(1.2f);

	// Set weapon and ensure animations are in sync.
	weaponEnt = weaponRe;
	enemySetAnimation(MD2ANIM_STAND);
}

// ========================================================
// EnemyEntity::update():
// ========================================================

void EnemyEntity::update()
{
	if (isDead())
	{
		if (currentAnimationFinished())
		{
			// End the particle effects soon.
			attackPrt->setOneTimeEmission(true);

			// Once the death animation is finished. Freeze the model.
			renderEnt->setStaticModel();
			if (renderEnt->hasLightShadowRenderer())
			{
				renderEnt->disableLightShadowRenderer();
			}

			// Hide the weapon, if any.
			if (weaponEnt != nullptr)
			{
				weaponEnt->setModel(nullptr, nullptr);
				weaponEnt = nullptr;
			}

			// Remove from the scene prop list so that the player can now walk over it.
			world->getTileMap().removeSceneProp(worldPosition);
		}
		return; // Already dead.
	}

	// Start a fight if in range.
	PlayerEntity & player = world->getPlayer();
	if (playerInRange(player))
	{
		// Enemy should face the player when in range:
		const Vector & playerWorldPos = player.getWorldPosition();
		const float dx = playerWorldPos.x - worldPosition.x;
		const float dz = playerWorldPos.z - worldPosition.z;
		const float newYawDegrees = radToDeg(atan2f(dx, dz));

		if (!ps2math::floatEquals(targetYawDegrees, newYawDegrees, 0.5f))
		{
			targetYawDegrees = newYawDegrees;
		}

		if (takeHit(player))
		{
			health -= PLAYER_INFLICTED_DAMAGE;
			if (isDead()) // This enemy is down!
			{
				if (currAnim != MD2ANIM_DEATH_FALL_BACK)
				{
					enemySetAnimation(MD2ANIM_DEATH_FALL_BACK);
				}

				// Give the player some heath back with each kill.
				player.setHealthAmount(player.getHealthAmount() + PLAYER_HP_RECOVERY_PER_KILL);

				// Kill the enemy in a puff of fire.
				attackPrt->setEnabled(true);
				attackPrt->setParticleReleaseAmount(200);
				attackPrt->setParticleReleaseInterval(10);
				attackPrt->setParticleLifeCycle(400);
				attackPrt->setParticleMinSize(0.5f);
				attackPrt->setParticleMaxSize(1.0f);
				attackPrt->setParticleVelocity(Vector(0.0f, 1.0f, 0.0f, 1.0f));
				attackPrt->setVelocityVar(1.0f);
				attackPrt->setParticleBaseColor(makeColor4f(1.0f, 0.9f, 0.9f));
				attackPrt->setParticleSpriteTexture(world->getBuiltInTexture("particle_fire"));
				attackPrt->setEmitterOrigin(worldPosition);
				return;
			}
			else
			{
				if (currAnim != MD2ANIM_PAIN_A)
				{
					enemySetAnimation(MD2ANIM_PAIN_A);
				}
			}
		}
		else
		{
			// If we can start another attack, set the proper animation:
			if (attackPosition(playerWorldPos))
			{
				if (currAnim != MD2ANIM_ATTACK)
				{
					enemySetAnimation(MD2ANIM_ATTACK);
				}
			}
		}
	}

	// Change animation now and then to
	// make the enemies seem more alive.
	if (currAnim == MD2ANIM_STAND)
	{
		// Roughly 1% chance of changing animation
		const bool changeAnimation = (randomFloat() < 0.01f);
		if (changeAnimation)
		{
			if (randomFloat() < 0.5f)
			{
				enemySetAnimation(MD2ANIM_SALUTE);
			}
			else
			{
				enemySetAnimation(MD2ANIM_POINT);
			}
		}
	}
	else
	{
		// Revert to STAND when current animation is finished.
		if (currentAnimationFinished())
		{
			enemySetAnimation(MD2ANIM_STAND);
		}
	}

	// Move the particle system representing a projectile fired by the enemy:
	if (attackPrt->isEnabled())
	{
		const Vector newProjectilePos = attackPrt->getEmitterOrigin()
				+ attackDir * gTime.deltaTimeMillis * 0.003f;

		// Hit the player.
		if (player.getPlayerBounds().containsPoint(player.getWorldPosition(), newProjectilePos))
		{
			attackPrt->setEnabled(false);
			player.takeEnemyHit();
		}
		// Kill the projectile if too far from the origin.
		else if (distanceSqr(worldPosition, newProjectilePos) >= ENEMRY_ATTACK_PLAYER_SQR_DIST * 2.0f)
		{
			attackPrt->setEnabled(false);
		}
		// Continue moving the projectile.
		else
		{
			attackPrt->setEmitterOrigin(newProjectilePos);
		}
	}

	// Adjust model rotation:
	if (orientModel())
	{
		Matrix modelTranslation;
		modelTranslation.makeTranslation(worldPosition);

		Matrix modelRotation;
		modelRotation.makeRotationY(degToRad(yawTotalDegrees - initialYawOffset));

		renderEnt->setModelMatrix(modelRotation * modelTranslation);

		// Weapon must obviously follow the enemy.
		if (weaponEnt != nullptr)
		{
			weaponEnt->setModelMatrix(renderEnt->getModelMatrix());
			weaponEnt->setWorldPosition(worldPosition);
		}
	}

	// Only boss enemies have this.
	// Rotate the power circle a little bit every frame.
	if (customLightOrShadow != nullptr)
	{
		customLightOrShadow->rotationY += gTime.deltaTimeMillis * 0.0001f;
	}
}

// ========================================================
// EnemyEntity::playerInRange():
// ========================================================

bool EnemyEntity::playerInRange(const PlayerEntity & player) const
{
	if (!player.isDead())
	{
		return distanceSqr(player.getWorldPosition(), worldPosition) <= ENEMRY_ATTACK_PLAYER_SQR_DIST;
	}
	return false;
}

// ========================================================
// EnemyEntity::takeHit():
// ========================================================

bool EnemyEntity::takeHit(const PlayerEntity & player) const
{
	// Very rudimentary attack / take damage logic:
	// - If the player is close enough to the enemy
	//   and swinging his sword, the enemy takes a hit.
	//
	if (player.isAttacking())
	{
		return distanceSqr(player.getWorldPosition(), worldPosition) <= PLAYER_ATTACK_ENEMY_SQR_DIST;
	}
	return false;
}

// ========================================================
// EnemyEntity::attackPosition():
// ========================================================

bool EnemyEntity::attackPosition(const Vector & pos)
{
	// Just took a hit, can't attack just now!
	if (currAnim == MD2ANIM_PAIN_A)
	{
		return false;
	}

	// An attack is already in flight.
	if (attackPrt->isEnabled())
	{
		return false;
	}

	// Fire a new one:
	attackPrt->setEmitterOrigin(worldPosition);
	attackPrt->setEnabled(true);

	attackTarget = pos;
	attackDir    = (pos - worldPosition).normalized();

	return true;
}

// ========================================================
// EnemyEntity::enemySetAnimation():
// ========================================================

void EnemyEntity::enemySetAnimation(const StdMd2AnimId animId)
{
	// Custom setAnimation for the enemy, which includes its weapon if it has one.
	GameEntity::setAnimation(animId);
	if (weaponEnt != nullptr)
	{
		weaponEnt->setAnimation(animId);
	}
}
