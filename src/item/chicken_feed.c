#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../item/key_items.h"
#include "../item/item.h"

extern Entity *self, player;

static void dropChickenFeed(int);

Entity *addChickenFeedBag(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add chicken feed bag\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &dropChickenFeed;
	e->die = &keyItemRespawn;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void dropChickenFeed(int val)
{
	Entity *e;
	
	if (self->thinkTime <= 0)
	{
		e = addTemporaryItem("item/chicken_feed", player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -10 : 10, ITEM_JUMP_HEIGHT);
		
		e->touch = &entityTouch;

		playSound("sound/common/throw.ogg", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, player.x, player.y);

		self->thinkTime = self->maxThinkTime;
	}
}
