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

#include "../system/properties.h"
#include "../graphics/animation.h"
#include "../entity.h"
#include "../system/random.h"
#include "../graphics/decoration.h"
#include "../inventory.h"
#include "../hud.h"
#include "../item/item.h"
#include "../event/trigger.h"
#include "../custom_actions.h"
#include "../audio/audio.h"
#include "../collisions.h"

extern Entity *self;

static void wait(void);
static void touch(Entity *);
static Entity *addCoal(int, int);

Entity *addCoalPile(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Coal Pile\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->touch = &touch;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

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

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (prand() % 90 == 0)
	{
		addSparkle(self->x + (prand() % self->w), self->y + (prand() % self->h));
	}
	
	checkToMap(self);
}

static void touch(Entity *other)
{
	Entity *e;

	pushEntity(other);

	if ((other->flags & ATTACKING) && !(self->flags & INVULNERABLE))
	{
		if (strcmpignorecase(other->name, self->requires) == 0)
		{
			if ((prand() % 4) == 0)
			{
				e = addCoal(self->x + self->w / 2, self->y);

				e->y -= e->h + 1;

				e->dirX = (4 + (prand() % 2)) * (prand() % 2 == 0 ? -1 : 1);
			}

			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);
		}

		else
		{
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);
		}
	}
}