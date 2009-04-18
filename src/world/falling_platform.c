#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../collisions.h"
#include "../entity.h"

static void wait(void);
static void touch(Entity *);
static void initFall(void);
static void resetWait(void);
static void resetPlatform(void);
static void initialize(void);

extern Entity *self;

Entity *addFallingPlatform(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Falling Platform!\n");

		exit(1);
	}

	loadProperties(name, e);

	e->type = FALLING_PLATFORM;
	e->action = &wait;

	e->touch = &touch;

	e->action = &initialize;

	e->draw = &drawLoopingAnimationToMap;

	e->x = e->startX = x;
	e->y = e->startY = y;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{

}

static void touch(Entity *other)
{
	/* Test the horizontal movement */

	if (other->type == PLAYER && other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y - other->dirY, other->w, other->h, self->x, self->y, self->w, self->h) == 0
		&& collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			/* Place the player as close to the solid tile as possible */

			other->y = self->y;
			other->y -= other->h;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;

			if (!(self->flags & ON_GROUND))
			{
				self->action = &initFall;
			}
		}
	}
}

static void initFall()
{
	self->thinkTime--;

	if (self->thinkTime > 0)
	{
		if (self->x == self->startX || (self->thinkTime % 4 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}
	}

	else
	{
		self->thinkTime = 0;

		self->x = self->startX;

		checkToMap(self);

		if (self->flags & ON_GROUND)
		{
			self->thinkTime = 120;

			self->action = &resetWait;
		}
	}
}

static void resetWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &resetPlatform;
	}
}

static void resetPlatform()
{
	self->dirY = -self->speed;

	checkToMap(self);

	if (self->y <= self->startY)
	{
		self->thinkTime = self->maxThinkTime;

		self->action = &wait;
	}
}

static void initialize()
{
	self->startX = self->x;
	self->startY = self->y;

	self->action = &wait;
}