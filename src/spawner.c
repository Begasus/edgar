#include "headers.h"

#include "animation.h"
#include "enemies.h"
#include "entity.h"
#include "properties.h"
#include "player.h"

extern Entity *self;

static void spawn(void);
static void init(void);

Entity *addSpawner(int x, int y, char *entityToSpawn)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spawner\n");

		exit(1);
	}

	loadProperties("common/spawner", e);

	e->x = x;
	e->y = y;

	strcpy(e->requires, entityToSpawn);

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->action = &init;

	e->type = SPAWNER;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->action = &spawn;

	if (self->active == FALSE)
	{
		self->thinkTime = 0;
	}

	if (self->health <= 0)
	{
		self->health = -1;
	}

	self->action();
}

static void spawn()
{
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->health == -1)
			{
				/* Don't spawn if the player is too close */

				if (getDistanceFromPlayer(self) > SCREEN_WIDTH)
				{
					e = addEnemy(self->objectiveName, self->x, self->y);

					e->x += (self->w - e->w) / 2;
					e->y += (self->h - e->h) / 2;
				}
				
				else
				{
					printf("Spawner not activating. Too close to player\n");
				}
			}

			else
			{
				e = addEnemy(self->objectiveName, self->x, self->y);

				e->x += (self->w - e->w) / 2;
				e->y += (self->h - e->h) / 2;

				self->health--;

				if (self->health == 0)
				{
					self->inUse = FALSE;
				}
			}

			self->thinkTime = self->maxThinkTime;
		}
	}
}