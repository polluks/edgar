/*
Copyright (C) 2009 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "enemies.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../player.h"

extern Entity *self, player;
extern Game game;

static void stickToPlayer(void);
static void attack(void);
static void grab(Entity *other);
static void fallOff(void);
static void stickToPlayerAndDrain(void);

Entity *addInvulnerableFlyer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Invulnerable Flyer\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &attack;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageFlinch;
	e->reactToBlock = &changeDirection;
	e->touch = &grab;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void fly()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}
}

static void die()
{
	self->action = &die;

	self->touch = &pushEntity;

	self->flags &= ~FLY;

	setEntityAnimation(self, ATTACK_1);

	self->takeDamage = &stoneTakeDamage;

	self->animationCallback = &stoneFinish;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		self->thinkTime = 300;

		self->action = &dieWait;
	}
}

static void stoneFinish()
{
	setEntityAnimation(self, ATTACK_2);
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, ATTACK_3);

		self->animationCallback = &reanimate;
	}

	checkToMap(self);
}

static void reanimate()
{
	self->flags |= FLY;

	self->touch = &entityTouch;

	self->takeDamage = &entityTakeDamageNoFlinch;

	self->health = self->maxHealth;

	checkToMap(self);

	self->dirY = -3.5f;

	self->action = &flyToTop;
}

static void flyToTop()
{
	checkToMap(self);

	if (self->dirY == 0 || self->y < self->startY)
	{
		self->dirY = 0;

		self->action = &fly;
	}
}

static void stoneTakeDamage(Entity *other, int damage)
{
	setCustomAction(self, &invulnerableNoFlash, 20, 0);

	playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);
}