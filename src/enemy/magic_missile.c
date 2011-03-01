/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../entity.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self, player;

static void move(void);
static void touch(Entity *);
static void reflectTouch(Entity *);
static void die(void);
static void addBreadCrumb(float, float, float, float);
static void init(void);
static void addTrail(void);
static void trailMove(void);
static void crumbWait(void);

Entity *addMagicMissile(int x, int y, char *name)
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Magic Missile");
	}

	loadProperties("enemy/magic_missile", e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &touch;

	e->type = PROJECTILE;

	setEntityAnimation(e, "STAND");

	e->head = &player;

	return e;
}

static void init()
{
	self->startX = self->x;
	self->startY = self->y;
	
	addBreadCrumb(self->x, self->y, self->dirX, self->dirY);

	self->thinkTime = self->maxThinkTime;

	self->action = &move;
}

static void move()
{
	int midX, midY;
	float dirX, dirY;

	self->thinkTime--;

	if (self->mental > 0)
	{
		self->health--;

		if (self->health <= 0)
		{
			addTrail();

			self->mental--;

			self->health = 2;
		}
	}
	
	if (self->head == NULL || self->head->health <= 0 || self->head->inUse == FALSE)
	{
		self->die();
		
		return;
	}

	if (self->thinkTime <= 0)
	{
		midX = self->head->x + self->head->w / 2 - self->w / 2;
		midY = self->head->y + self->head->h / 2 - self->h / 2;

		dirX = self->dirX;
		dirY = self->dirY;

		if (fabs(midX - self->x) > fabs(self->dirX))
		{
			if (midX < self->x)
			{
				self->dirX -= 1;

				if (self->dirX < -self->speed)
				{
					self->dirX = -self->speed;
				}
			}

			else
			{
				self->dirX += 1;

				if (self->dirX > self->speed)
				{
					self->dirX = self->speed;
				}
			}
		}

		if (fabs(midY - self->y) > fabs(self->dirY))
		{
			if (midY < self->y)
			{
				self->dirY -= 1;

				if (self->dirY < -self->speed)
				{
					self->dirY = -self->speed;
				}
			}

			else
			{
				self->dirY += 1;

				if (self->dirY > self->speed)
				{
					self->dirY = self->speed;
				}
			}
		}

		self->thinkTime = self->maxThinkTime;

		addBreadCrumb(self->x, self->y, dirX, dirY);
	}

	dirX = self->dirX;
	dirY = self->dirY;

	checkToMap(self);

	if ((dirX != 0 && self->dirX == 0) || (dirY != 0 && self->dirY == 0))
	{
		self->die();
	}
}

static void addTrail()
{
	char animName[MAX_VALUE_LENGTH];
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Magic Missile");
	}

	loadProperties("enemy/magic_missile", e);

	e->x = self->startX;
	e->y = self->startY;

	e->endX = self->target->x;
	e->endY = self->target->y;

	e->dirX = self->target->dirX;
	e->dirY = self->target->dirY;

	e->mental = self->target->mental;

	e->action = &trailMove;

	e->draw = &drawLoopingAnimationToMap;

	e->type = TEMP_ITEM;

	e->head = self;

	snprintf(animName, MAX_VALUE_LENGTH, "TRAIL_%d", self->mental);

	setEntityAnimation(e, animName);
}

static void trailMove()
{
	Entity *e;

	if (fabs(self->endX - self->x) <= fabs(self->dirX) && fabs(self->endY - self->y) <= fabs(self->dirY))
	{
		self->x = self->endX;
		self->y = self->endY;

		self->dirX = 0;
		self->dirY = 0;

		self->mental++;

		e = self->head->target;

		while (e->mental != self->mental)
		{
			e = e->target;

			if (e == NULL)
			{
				self->inUse = FALSE;

				return;
			}
		}

		self->endX = e->x;
		self->endY = e->y;

		self->dirX = e->dirX * 0.75;
		self->dirY = e->dirY * 0.75;
	}

	self->x += self->dirX;
	self->y += self->dirY;
}

static void touch(Entity *other)
{
	if (strcmpignorecase(other->name, "edgar/edgar_reflection_shield") == 0)
	{
		self->touch = &reflectTouch;
		
		self->head = getEntityByObjectiveName(self->objectiveName);
		
		setCustomAction(&player, &invulnerableNoFlash, 120, 0, 0);
		
		self->damage = 50;
	}
	
	else
	{
		entityTouch(other);
	}
}

static void reflectTouch(Entity *other)
{
	Entity *temp;

	if (other == self->target)
	{
		temp = self;

		self = self->target;

		self->takeDamage(temp, temp->damage);

		self = temp;

		self->inUse = FALSE;
	}
}

static void addBreadCrumb(float x, float y, float dirX, float dirY)
{
	Entity *crumb, *e, *prev;

	crumb = getFreeEntity();

	if (crumb == NULL)
	{
		showErrorAndExit("No free slots to add a Magic Missile BreadCrumb");
	}

	loadProperties("enemy/magic_missile", crumb);

	crumb->x = x;
	crumb->y = y;

	crumb->dirX = dirX;
	crumb->dirY = dirY;

	crumb->action = &crumbWait;

	crumb->flags |= NO_DRAW;

	crumb->draw = &drawLoopingAnimationToMap;

	crumb->type = TEMP_ITEM;

	crumb->mental = 0;
	
	crumb->thinkTime = 120;

	setEntityAnimation(crumb, "STAND");

	e = self;

	while (e->target != NULL)
	{
		e = e->target;
		
		crumb->mental = e->mental + 1;
	}

	e->target = crumb;
	
	e = self;
	
	while (e->target != NULL)
	{
		prev = e;
		
		e = e->target;
		
		if (e->thinkTime <= 0)
		{
			prev->target = e->target;
			
			e->inUse = FALSE;
		}
	}
}

static void crumbWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
}

static void die()
{
	Entity *e;

	e = self->target;

	while (e != NULL)
	{
		e->inUse = FALSE;

		e = e->target;
	}

	self->inUse = FALSE;
}
