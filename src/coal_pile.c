#include "headers.h"

#include "properties.h"
#include "animation.h"
#include "entity.h"
#include "random.h"
#include "decoration.h"
#include "inventory.h"
#include "hud.h"
#include "item.h"
#include "trigger.h"
#include "custom_actions.h"

extern Entity *self;

static void pileWait(void);
static void pileTouch(Entity *);
static Entity *addCoal(int, int);

Entity *addCoalPile(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Coal Pile\n");

		exit(1);
	}

	loadProperties("item/coal_pile", e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &pileWait;
	e->touch = &pileTouch;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	addTrigger("Coal Bag", 10, UPDATE_OBJECTIVE, "Collect 10 pieces of Coal");
	addTrigger("LandSlide Trigger", 1, ACTIVATE_ENTITY, "LANDSLIDE_TRIGGER");

	return e;
}

static Entity *addCoal(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Coal\n");

		exit(1);
	}

	loadProperties("item/coal", e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->thinkTime = 600;
	e->type = ITEM;

	e->face = RIGHT;

	e->action = &generalItemAction;
	e->touch = &addRequiredToInventory;
	e->draw = &drawLoopingAnimationToMap;

	setCustomAction(e, &invulnerableNoFlash, 60);

	setEntityAnimation(e, STAND);

	return e;
}

static void pileWait()
{
	if (prand() % 90 == 0)
	{
		addSparkle(self->x + (prand() % self->w), self->y + (prand() % self->h));
	}
}

static void pileTouch(Entity *other)
{
	Entity *e;

	pushEntity(other);

	if ((other->flags & ATTACKING) && strcmpignorecase(other->name, self->requires) == 0)
	{
		if (prand() % 10 == 0)
		{
			e = addCoal(self->x + self->w / 2, self->y);

			e->y -= e->h + 1;

			e->dirX = (4 + (prand() % 2)) * (prand() % 2 == 0 ? -1 : 1);
		}
	}
}