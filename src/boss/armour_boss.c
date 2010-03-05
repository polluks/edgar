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

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../game.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../player.h"
#include "../geometry.h"
#include "../system/error.h"
#include "../event/script.h"

extern Entity *self, player;

static void die(void);
static void dieFinish(void);
static void initialise(void);
static void wakeUp(void);
static void doIntro(void);
static void introWait(void);
static void attackFinished(void);
static void takeDamage(Entity *, int);
static void regenerateHealth(void);
static void wait(void);
static void armourTakeDamage(Entity *, int);
static void armourDie(void);
static void armourWait(void);
static void sawWait(void);
static void regenerateArmour(void);
static void lookForPlayer(void);
static void growl(void);
static void addYellowGem(void);
static void gemWait(void);
static void zMove(void);
static void zVanish(void);
static void tongueAttackStart(void);
static void tongueAttack(void);
static void tongueAttackFinish(void);
static void tongueMove(void);
static void tongueReturn(void);
static void tongueTouch(Entity *);
static void tongueTakeDamage(Entity *, int);
static void tongueReturn(void);
static int drawTongue(void);
static void tongueEat(void);
static void panic(void);
static void sawAttackWait(void);
static void sawAttackStart(void);
static void sawAttack(void);
static void sawAttackFinish(void);
static void chargeAttack(void);
static void stunned(void);
static void stunWake(void);
static void stunTouch(Entity *);
static void activate(int);
static void chargeAttackStart(void);
static void chargeAttack(void);
static void chargeAttackTouch(Entity *);

Entity *addArmourBoss(int x, int y, char *name)
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Armour Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = NULL;

	e->type = ENEMY;

	e->active = FALSE;

	e->die = &die;

	e->reactToBlock = &changeDirection;

	e->resumeNormalFunction = &attackFinished;

	setEntityAnimation(e, CUSTOM_1);

	return e;
}

static void initialise()
{
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		setCustomAction(&player, &helpless, 2, 0, 0);

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, CUSTOM_2);

			self->animationCallback = &wakeUp;

			self->touch = &entityTouch;

			self->takeDamage = &takeDamage;
		}
	}

	else if (prand() % 120 == 0)
	{
		e = addBasicDecoration(self->x + self->w - 30, self->y + 30, "decoration/z");

		if (e != NULL)
		{
			e->face = RIGHT;

			e->startX = e->x;

			e->action = &zMove;
			e->animationCallback = &zVanish;
		}
	}

	if (self->endY == 0)
	{
		addYellowGem();
	}

	checkToMap(self);
}

static void wakeUp()
{
	setCustomAction(&player, &helpless, 2, 0, 0);

	setEntityAnimation(self, CUSTOM_3);
	
	playSoundToMap("sound/boss/armour_boss/growl.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime = 60;

	self->action = &doIntro;

	checkToMap(self);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		initBossHealthBar();

		playBossMusic();

		self->action = &introWait;

		self->startX = 360;

		self->endX = 0;

		self->endY = 1;

		self->mental = 0;

		runScript("armour_boss_start");
	}

	else
	{
		setCustomAction(&player, &helpless, 2, 0, 0);
	}

	checkToMap(self);
}

static void introWait()
{
	checkToMap(self);
}

static void wait()
{
	regenerateHealth();

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 120 + (prand() % 120);

			self->dirX = self->face == RIGHT ? self->speed : -self->speed;

			self->action = &lookForPlayer;
		}
	}
}

static void lookForPlayer()
{
	setEntityAnimation(self, WALK);

	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->thinkTime = 0;

		if (self->mental == 0)
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -200), self->y, 200, self->h, player.x, player.y, player.w, player.h) == 1)
			{
				self->thinkTime = 60;
	
				self->maxThinkTime = 3;
	
				self->action = &chargeAttackStart;
			}
		}

		/* Attack if up to 300 pixels away */

		else if (collision(self->x + (self->face == RIGHT ? self->w : -300), self->y, 300, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			if (self->mental > 0 && prand() % 2 == 0)
			{
				self->dirX = 0;

				setEntityAnimation(self, STAND);

				self->maxThinkTime = 3;

				self->action = &sawAttackWait;
			}
			
			else
			{
				self->dirX = 0;

				setEntityAnimation(self, STAND);

				self->thinkTime = 30;

				self->action = &tongueAttackStart;
			}
		}
		
		else if (prand() % 300 == 0)
		{
			setEntityAnimation(self, STAND);
			
			self->thinkTime = 120;
			
			self->action = &growl;
			
			playSoundToMap("sound/boss/armour_boss/growl.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		}
	}

	regenerateHealth();
}

static void growl()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->action = &lookForPlayer;
	}
}

static void tongueAttackStart()
{
	self->dirX = 0;

	self->maxThinkTime = 0;

	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &tongueAttack;

	checkToMap(self);

	regenerateHealth();
}

static void tongueAttack()
{
	Entity *e;

	self->action = &tongueAttack;

	setEntityAnimation(self, ATTACK_2);

	if (self->maxThinkTime == 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Armour Boss Tongue");
		}

		loadProperties("boss/armour_boss_tongue", e);

		setEntityAnimation(e, STAND);

		e->face = self->face;

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - e->offsetX;
		}

		else
		{
			e->x = self->x + e->offsetX;
		}

		e->y = self->y + e->offsetY;

		e->startX = e->x;
		e->startY = e->y;

		e->endY = getMapFloor(e->x, e->y);

		e->thinkTime = 15;

		e->mental = 0;

		e->action = &tongueMove;

		e->touch = &tongueTouch;

		e->draw = &drawTongue;

		e->takeDamage = &tongueTakeDamage;

		e->type = ENEMY;

		e->head = self;

		self->maxThinkTime = 1;

		playSoundToMap("sound/boss/armour_boss/tongue_start.ogg", BOSS_CHANNEL, self->x, self->y, 0);
	}

	checkToMap(self);

	regenerateHealth();
}

static void tongueAttackFinish()
{
	self->frameSpeed = -1;

	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &attackFinished;

	checkToMap(self);

	regenerateHealth();
}

static void tongueMove()
{
	if (abs(self->mental) < 320)
	{
		self->x += self->face == LEFT ? -self->speed : self->speed;

		self->mental += self->speed;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &tongueReturn;
		}
	}
}

static void tongueReturn()
{
	if (self->face == LEFT)
	{
		self->x += self->speed;

		if (self->x > self->startX)
		{
			self->head->action = &tongueAttackFinish;

			self->inUse = FALSE;
		}
	}

	else
	{
		self->x += -self->speed;

		if (self->x < self->startX)
		{
			self->head->action = &tongueAttackFinish;

			self->inUse = FALSE;
		}
	}
}

static void tongueTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		self->x = other->x + other->w / 2 - self->w / 2;

		self->targetY = self->endY - other->h;

		self->target = other;

		self->thinkTime = 15;

		self->action = &tongueEat;

		self->touch = &entityTouch;

		playSoundToMap("sound/boss/armour_boss/tongue_hit.ogg", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static void tongueEat()
{
	Entity *temp;

	self->target->dirX = 0;
	self->target->dirY = 0;

	self->target->x = self->x + self->w / 2 - self->target->w / 2;

	self->target->y = self->targetY;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else
	{
		self->x += self->face == RIGHT ? -1 : 1;

		if (fabs(self->x - self->startX) <= 32)
		{
			temp = self;

			self = self->target;

			playerGib();

			self = temp;

			self->head->action = &tongueAttackFinish;

			self->inUse = FALSE;
		}
	}
}

static void tongueTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		self->health -= 1;

		if (self->health <= 0)
		{
			if (self->target != NULL)
			{
				self->target = NULL;
			}

			self->touch = NULL;

			self->action = &tongueReturn;
		}

		else
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

			enemyPain();
		}
	}
}

static int drawTongue()
{
	int startX;

	startX = self->x;

	/* Draw the tongue first */

	self->x = self->x + (self->face == RIGHT ? 0 : self->w);

	setEntityAnimation(self, WALK);

	if (self->face == RIGHT)
	{
		while (self->x >= self->startX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	else
	{
		while (self->x <= self->startX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	/* Draw the tip */

	setEntityAnimation(self, STAND);

	self->x = startX;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void attackFinished()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, STAND);

	self->thinkTime = 0;

	self->maxThinkTime = 0;

	self->action = &wait;

	self->touch = &entityTouch;

	regenerateHealth();

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (self->mental > 0)
		{
			/* The armour will take the damage instead */

			armourTakeDamage(other, damage);

			setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		}

		else
		{
			self->health -= damage;

			if (other->type != ENEMY && self->mental == 0 && (prand() % 10 == 0))
			{
				setInfoBoxMessage(60, _("Its wounds are already healing..."));

				/* Don't get killed by anything except the slimes */

				if (self->health <= 0)
				{
					self->health = 1;
				}
			}

			if (strcmpignorecase(other->name, "enemy/red_baby_slime") != 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);

				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

				enemyPain();
			}

			else if (self->health > 0)
			{
				self->startX = 1200;

				if (self->endX == 0)
				{
					self->endX = 20;

					setCustomAction(self, &flashWhite, 6, 0, 0);

					if (prand() % 5 == 0)
					{
						setCustomAction(self, &helpless, 10, 0, 0);

						self->dirX = other->face == RIGHT ? 6 : -6;
					}

					enemyPain();
				}
			}

			else
			{
				self->damage = 0;

				self->dirX = 0;

				self->startX = self->x;

				self->startY = 0;

				self->thinkTime = 180;

				self->action = &die;
			}
		}

		/* Jump away in panic */

		if ((prand() % 6 == 0) && self->face == other->face && self->health > 0 && other->type != ENEMY && self->maxThinkTime == 0)
		{
			self->dirX = self->x < player.x ? -10 : 10;

			self->dirY = -6;

			self->face = self->face == RIGHT ? LEFT : RIGHT;

			self->action = &panic;
		}
	}
}

static void panic()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->action = &lookForPlayer;
	}
}

static void regenerateHealth()
{
	if (strcmpignorecase(self->name, "boss/armour_boss") != 0)
	{
		printf("%s shouldn't regenerate!\n", self->name);

		abort();
	}

	self->health += 3;

	if (self->health > self->maxHealth)
	{
		self->health = self->maxHealth;
	}

	if (self->mental == 0)
	{
		self->startX--;

		if (self->startX <= 0)
		{
			regenerateArmour();
		}
	}

	self->endX--;

	if (self->endX <= 0)
	{
		self->endX = 0;
	}
}

static void die()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		self->action = &dieFinish;

		for (i=0;i<60;i++)
		{
			e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

			if (e != NULL)
			{
				e->y -= prand() % e->h;
			}
		}

		setEntityAnimation(self, DIE);
	}

	else
	{
		self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

		self->startY += 90;

		if (self->startY >= 360)
		{
			self->startY = 0;
		}
	}
}

static void dieFinish()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();

		entityDieNoDrop();
	}
}

static void regenerateArmour()
{
	int i;
	Entity *e, *prev;

	if (self->target == NULL)
	{
		prev = self;

		for (i=0;i<7;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Armour Boss's Armour");
			}

			loadProperties("boss/armour_boss_armour", e);

			e->x = self->x;
			e->y = self->y;

			e->action = &armourWait;

			e->draw = &drawLoopingAnimationToMap;

			e->takeDamage = NULL;

			e->type = ENEMY;

			e->die = &armourDie;

			e->thinkTime = 60 + prand() % 120;

			e->head = self;

			prev->target = e;

			prev = e;

			setEntityAnimation(e, i);

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - e->offsetX;
			}

			else
			{
				e->x = self->x + e->offsetX;
			}

			e->y = self->y + e->offsetY;

			self->mental++;
		}
	}

	else
	{
		i = 0;

		for (e=self->target;e!=NULL;e=e->target)
		{
			loadProperties("boss/armour_boss_armour", e);

			e->x = self->x;
			e->y = self->y;

			e->action = &armourWait;

			e->draw = &drawLoopingAnimationToMap;

			e->takeDamage = NULL;

			e->type = ENEMY;

			e->die = &armourDie;

			e->thinkTime = 60 + prand() % 120;

			setEntityAnimation(e, i);

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - e->offsetX;
			}

			else
			{
				e->x = self->x + e->offsetX;
			}

			e->y = self->y + e->offsetY;

			self->mental++;

			i++;
		}
	}

	/* Add the buzzsaw */
	
	for (i=0;i<3;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Armour Boss's Buzzsaw");
		}

		loadProperties("boss/armour_boss_saw", e);

		e->x = self->x;
		e->y = self->y;

		e->head = self;

		e->action = &sawWait;

		e->draw = &drawLoopingAnimationToMap;

		e->takeDamage = NULL;

		e->type = ENEMY;
		
		e->maxThinkTime = i;

		setEntityAnimation(e, STAND);

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - e->offsetX;
		}

		else
		{
			e->x = self->x + e->offsetX;
		}

		e->y = self->y + e->offsetY;
	}
}

static void armourWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->flags |= INVULNERABLE;

		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void sawWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->flags |= INVULNERABLE;

		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX + (self->w * self->maxThinkTime);
	}

	else
	{
		self->x = self->head->x + self->offsetX - (self->w * self->maxThinkTime);
	}

	self->y = self->head->y + self->offsetY;

	self->startX = self->x;
	self->startY = self->y;

	if (self->head->maxThinkTime == 3)
	{
		playSoundToMap("sound/boss/armour_boss/saw_start.ogg", -1, self->x, self->y, 0);
		
		printf("Assigned sound to channel %d\n", self->health);
		
		self->thinkTime = 60 + (self->maxThinkTime * 30);

		setEntityAnimation(self, WALK);

		self->action = &sawAttackStart;
	}

	if (self->head->mental == 0)
	{
		stopSound(self->targetX);
		
		self->action = &entityDieNoDrop;
	}
}

static void sawAttackWait()
{
	if (self->maxThinkTime == 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);

	regenerateHealth();
}

static void sawAttackStart()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = player.x + player.w / 2 - self->w / 2;
		self->targetY = player.y + player.h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 12;
		self->dirY *= 12;

		self->action = &sawAttack;

		self->touch = &entityTouch;

		self->thinkTime = 120;

		self->endX = self->dirX;
		self->endY = self->dirY;

		printf("Moving to player with %f %f (%f %f)\n", self->dirX, self->dirY, self->endX, self->endY);
	}
}

static void sawAttack()
{
	Entity *e;

	if (self->dirX == 0 && self->dirY == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->targetX = self->startX;
			self->targetY = self->startY;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 6;
			self->dirY *= 6;

			self->action = &sawAttackFinish;
		}

		switch (self->mental)
		{
			case 1:
				e = addPixelDecoration(self->x, self->y + self->h / 2);
			break;

			case 2:
				e = addPixelDecoration(self->x + self->w, self->y + self->h / 2);
			break;

			case 3:
				e = addPixelDecoration(self->x + self->w / 2, self->y);
			break;

			default:
				e = addPixelDecoration(self->x + self->w / 2, self->y + self->h);
			break;
		}
		
		if (e != NULL)
		{
			e->dirX = prand() % 20;
			e->dirY = prand() % 20;

			if (prand() % 2 == 0)
			{
				e->dirX *= -1;
			}

			e->dirX /= 10;
			e->dirY /= 10;

			e->thinkTime = 20 + (prand() % 30);

			e->health = 255;

			e->maxHealth = 0;

			e->mental = 255;
		}
	}

	else if (self->dirX != self->endX || self->dirY != self->endY)
	{
		self->health = playSoundToMap("sound/boss/armour_boss/saw_spin.ogg", -1, self->x, self->y, -1);

		if (self->dirX != self->endX)
		{
			self->mental = self->endX < 0 ? 1 : 2;
		}

		else if (self->dirY != self->endY)
		{
			self->mental = self->endY < 0 ? 3 : 4;
		}

		self->dirX = 0;
		self->dirY = 0;
	}

	else
	{
		checkToMap(self);
	}
}

static void sawAttackFinish()
{
	if (atTarget())
	{
		printf("Stopping sound channel %d\n", self->health);
		
		stopSound(self->health);
		
		playSoundToMap("sound/boss/armour_boss/saw_stop.ogg", -1, self->x, self->y, 0);
		
		self->thinkTime = 0;

		setEntityAnimation(self, STAND);

		self->action = &sawWait;

		self->touch = NULL;

		self->head->maxThinkTime--;

		self->reactToBlock = NULL;
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void armourTakeDamage(Entity *other, int damage)
{
	Entity *e;

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->health <= 0)
		{
			continue;
		}

		if (!(e->flags & INVULNERABLE))
		{
			e->health -= damage;

			if (e->health <= 0)
			{
				e->thinkTime = 180;

				e->flags &= ~FLY;

				e->dirY = ITEM_JUMP_HEIGHT;

				e->action = e->die;

				self->mental--;

				if (self->mental == 0)
				{
					self->startX = 1200;
				}
			}

			else
			{
				setCustomAction(e, &flashWhite, 6, 0, 0);

				setCustomAction(e, &invulnerableNoFlash, 20, 0, 0);

				enemyPain();
			}
		}

		return;
	}
}

static void armourDie()
{
	long onGround = self->flags & ON_GROUND;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime < 90)
		{
			if (self->thinkTime % 3 == 0)
			{
				self->flags ^= NO_DRAW;
			}
		}
	}

	else
	{
		self->flags |= NO_DRAW;
	}

	checkToMap(self);

	if ((self->flags & ON_GROUND) && onGround == 0)
	{
		playSoundToMap("sound/boss/armour_boss/clang.ogg", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static void addYellowGem()
{
	Entity *e;

	e = addPermanentItem("item/yellow_gem", 0, 0);

	e->action = &gemWait;

	e->head = self;

	e->touch = NULL;

	e->thinkTime = 3600;

	self->endY = 1;
}

static void gemWait()
{
	self->face = self->head->face;

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	if (self->head->active == TRUE && self->mental > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setInfoBoxMessage(120, _("Find a way to remove the Gem..."));

			self->thinkTime = 3600;
		}
	}

	if (self->head->maxThinkTime == 99)
	{
		addPermanentItem(self->name, self->x, self->y);

		self->inUse = FALSE;
	}
}

static void chargeAttackTouch(Entity *other)
{
	entityTouch(other);

	if (other->type == PLAYER)
	{
		self->dirX = 0;

		setEntityAnimation(self, STAND);

		setPlayerStunned(30);

		other->dirX = (6 + prand() % 3) * (self->face == LEFT ? -1 : 1);
		other->dirY = -8;

		self->action = &attackFinished;
	}
}

static void chargeAttackStart()
{
	setEntityAnimation(self, ATTACK_3);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == LEFT ? -self->speed * 3 : self->speed * 3;

		setEntityAnimation(self, ATTACK_4);

		self->action = &chargeAttack;

		self->touch = &chargeAttackTouch;
	}

	checkToMap(self);
}

static void chargeAttack()
{
	int i;
	Entity *e;

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? 3 : 3;

		self->dirY = -6;

		setEntityAnimation(self, CUSTOM_1);

		self->action = &stunned;

		self->touch = &stunTouch;

		self->activate = &activate;

		self->thinkTime = 360;
	}

	checkToMap(self);

	for (i=0;i<10;i++)
	{
		e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

		if (e != NULL)
		{
			e->y -= prand() % e->h;
		}
	}
}

static void stunned()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->activate = NULL;

		setEntityAnimation(self, CUSTOM_2);

		self->animationCallback = &stunWake;
	}

	checkToMap(self);
}

static void stunWake()
{
	self->action = &attackFinished;
}

static void stunTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		setInfoBoxMessage(0, _("Press Action to retrieve the Yellow Gem"));
	}
}

static void activate(int val)
{
	self->maxThinkTime = 99;
}

static void zMove()
{
	self->health++;

	self->x = self->startX + sin(DEG_TO_RAD(self->health)) * 8;

	self->y -= 0.5;
}

static void zVanish()
{
	self->inUse = FALSE;
}
