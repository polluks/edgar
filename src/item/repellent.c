/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "../hud.h"
#include "../system/error.h"
#include "../collisions.h"

extern Entity *self, player;
extern Game game;

static void sprayRepellent(int);
static void sprayMove(void);
static void sprayTouch(Entity *);

Entity *addRepellent(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Repellent");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &sprayRepellent;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void sprayRepellent(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Repellent Spray");
		}

		loadProperties("item/repellent_spray", e);

		e->x = player.x + (player.face == RIGHT ? player.w : 0);
		e->y = player.y + player.h / 2;

		e->type = ITEM;

		e->face = RIGHT;

		e->action = &sprayMove;
		e->touch = &sprayTouch;

		e->draw = &drawLoopingAnimationToMap;

		e->active = FALSE;

		setEntityAnimation(e, STAND);

		self->thinkTime = self->maxThinkTime;

		e->dirX = player.face == RIGHT ? e->speed : -e->speed;
	}
}

static void sprayTouch(Entity *other)
{
	Entity *temp;

	if (other->type == ENEMY && strcmpignorecase(other->name, "huge_spider") == 0)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}
}

static void sprayMove()
{
	float dirY;

	self->dirX *= 0.95;

	if (fabs(self->dirX) <= 0.05)
	{
		self->dirX = 0;

		self->dirY = -self->speed;
	}

	dirY = self->dirY;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}