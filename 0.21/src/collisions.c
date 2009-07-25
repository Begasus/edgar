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

#include "headers.h"

#include "collisions.h"
#include "map.h"
#include "audio/audio.h"

extern Entity entity[MAX_ENTITIES], *self;
extern Entity player, playerShield, playerWeapon;

static Grid grid[GRID_HEIGHT][GRID_WIDTH];

static void addToList(int, int, Entity *);

void initCollisionGrid()
{
	int x, y;

	for (y=0;y<GRID_HEIGHT;y++)
	{
		for (x=0;x<GRID_WIDTH;x++)
		{
			grid[y][x].count = 0;
			grid[y][x].listHead.next = NULL;
		}
	}
}

void addToGrid(Entity *e)
{
	int left, right, top, bottom;

	if (e == &playerWeapon)
	{
		if (e->face == LEFT)
		{
			left = (e->x + e->parent->w - e->offsetX) / TILE_SIZE / GRID_COUNT;
			right = (e->x + e->parent->w - e->w - e->offsetX) / TILE_SIZE / GRID_COUNT;
		}

		else
		{
			left = (e->x + e->offsetX) / TILE_SIZE / GRID_COUNT;
			right = (e->x + e->offsetX + e->w - 1) / TILE_SIZE / GRID_COUNT;
		}

		top = e->y / TILE_SIZE / GRID_COUNT;
		bottom = (e->y + e->h - 1) / TILE_SIZE / GRID_COUNT;
	}

	else
	{
		left = (e->x + e->box.x) / TILE_SIZE / GRID_COUNT;
		right = (e->x + e->box.x + e->box.w - 1) / TILE_SIZE / GRID_COUNT;

		top = (e->y + e->box.y) / TILE_SIZE / GRID_COUNT;
		bottom = (e->y + e->box.y + e->box.h - 1) / TILE_SIZE / GRID_COUNT;
	}

	addToList(top, left, e);

	if (left != right)
	{
		addToList(top, right, e);
	}

	if (top != bottom)
	{
		addToList(bottom, left, e);
	}

	if (left != right && top != bottom)
	{
		addToList(bottom, right, e);
	}
}

static void addToList(int y, int x, Entity *e)
{
	EntityList *listHead, *list;

	if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
	{
		return;
	}

	listHead = &grid[y][x].listHead;

	while (listHead->next != NULL)
	{
		listHead = listHead->next;
	}

	list = (EntityList *)malloc(sizeof(EntityList));

	list->entity = e;
	list->next = NULL;

	listHead->next = list;

	grid[y][x].count++;
}

void freeCollisionGrid()
{
	int x, y;
	EntityList *p, *q;

	for (y=0;y<GRID_HEIGHT;y++)
	{
		for (x=0;x<GRID_WIDTH;x++)
		{
			for (p=grid[y][x].listHead.next;p!=NULL;p=q)
			{
				q = p->next;

				free(p);
			}

			grid[y][x].count = 0;
			grid[y][x].listHead.next = NULL;
		}
	}
}

void doCollisions()
{
	int i, j, x1, y1, x2, y2, w1, h1, w2, h2;
	Entity *e1, *e2, *temp;
	EntityList *list1, *list2;

	for (i=0;i<GRID_HEIGHT;i++)
	{
		for (j=0;j<GRID_WIDTH;j++)
		{
			for (list1=grid[i][j].listHead.next;list1!=NULL;list1=list1->next)
			{
				e1 = list1->entity;

				if (e1->inUse == TRUE && e1->touch != NULL)
				{
					for (list2=grid[i][j].listHead.next;list2!=NULL;list2=list2->next)
					{
						e2 = list2->entity;

						if ((e1->type != PLAYER && (e2->flags & PLAYER_TOUCH_ONLY)) ||
							((e1->flags & PLAYER_TOUCH_ONLY) && e2->type != PLAYER))
						{
							continue;
						}

						if (e1 != e2 && e2->inUse == TRUE && e2->touch != NULL)
						{
							if (e1->type == PROJECTILE)
							{
								if (e2->type == PROJECTILE || (e1->parent != NULL && e1->parent->type == ENEMY && e2->type == ENEMY))
								{
									continue;
								}
							}

							if ((e1 == &player && e2 == &playerWeapon) || (e1 == &playerWeapon && e2 == &player))
							{
								continue;
							}

							if (e1->face == RIGHT)
							{
								x1 = e1->x + e1->box.x;
							}

							else
							{
								x1 = e1->x + e1->w - e1->box.w - e1->box.x;
							}

							y1 = e1->y + e1->box.y;

							w1 = e1->box.w;
							h1 = e1->box.h;

							if (e2->face == RIGHT)
							{
								x2 = e2->x + e2->box.x;
							}

							else
							{
								x2 = e2->x + e2->w - e2->box.w - e2->box.x;
							}

							y2 = e2->y + e2->box.y;

							w2 = e2->box.w;
							h2 = e2->box.h;

							if (e1 == &playerWeapon)
							{
								x1 = e1->x + e1->box.x;
								y1 = e1->y + e1->box.y;

								if (e1->face == LEFT)
								{
									x1 += e1->parent->w - e1->w - e1->offsetX;
								}

								else
								{
									x1 += e1->offsetX;
								}


								y1 += e1->offsetY;
							}

							if (collision(x1, y1, w1, h1, x2, y2, w2, h2) == TRUE)
							{
								temp = self;

								self = e2;

								self->touch(e1);

								self = temp;
							}
						}
					}
				}
			}
		}
	}
}

void checkEntityToEntity(Entity *e)
{
	int i, j, x1, y1, x2, y2, w1, h1, w2, h2;
	Entity *e1, *e2, *temp;
	EntityList *list1, *list2;

	for (i=0;i<GRID_HEIGHT;i++)
	{
		for (j=0;j<GRID_WIDTH;j++)
		{
			for (list1=grid[i][j].listHead.next;list1!=NULL;list1=list1->next)
			{
				e1 = list1->entity;

				if (e1 != e)
				{
					continue;
				}

				if (e1->inUse == TRUE)
				{
					for (list2=grid[i][j].listHead.next;list2!=NULL;list2=list2->next)
					{
						e2 = list2->entity;

						if (e1 != e2 && e2->inUse == TRUE && e2->touch != NULL)
						{
							if (e1->type == ENEMY && e2->type == ENEMY)
							{
								continue;
							}

							if ((e1->type != PLAYER && (e2->flags & PLAYER_TOUCH_ONLY)) ||
								((e1->flags & PLAYER_TOUCH_ONLY) && e2->type != PLAYER))
							{
								continue;
							}

							if (e1->type == PROJECTILE)
							{
								if (e2->type == PROJECTILE || (e1->parent != NULL && e1->parent->type == ENEMY && e2->type == ENEMY))
								{
									continue;
								}
							}

							if ((e1 == &player && e2 == &playerWeapon) || (e1 == &playerWeapon && e2 == &player))
							{
								continue;
							}

							if (e1->face == RIGHT)
							{
								x1 = e1->x + e1->box.x;
							}

							else
							{
								x1 = e1->x + e1->w - e1->box.w - e1->box.x;
							}

							y1 = e1->y + e1->box.y;

							w1 = e1->box.w;
							h1 = e1->box.h;

							if (e2->face == RIGHT)
							{
								x2 = e2->x + e2->box.x;
							}

							else
							{
								x2 = e2->x + e2->w - e2->box.w - e2->box.x;
							}

							y2 = e2->y + e2->box.y;

							w2 = e2->box.w;
							h2 = e2->box.h;

							if (e1 == &playerWeapon)
							{
								x1 = e1->x + e1->box.x;
								y1 = e1->y + e1->box.y;

								if (e1->face == LEFT)
								{
									x1 += e1->parent->w - e1->w - e1->offsetX;
								}

								else
								{
									x1 += e1->offsetX;
								}


								y1 += e1->offsetY;
							}

							if (collision(x1, y1, w1, h1, x2, y2, w2, h2) == TRUE)
							{
								temp = self;

								self = e1;

								self->touch(e2);

								self = temp;
							}
						}
					}
				}
			}
		}
	}
}

Entity *isSpaceEmpty(Entity *e)
{
	int i;

	if (player.inUse == TRUE && collision(e->x, e->y, e->w, e->h, player.x, player.y, player.w, player.h) == 1)
	{
		return &player;
	}

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && e != &entity[i]
			&& collision(e->x, e->y, e->w, e->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
		{
			return &entity[i];
		}
	}

	return NULL;
}

void checkToMap(Entity *e)
{
	int i, x1, x2, y1, y2, previousEnvironment;
	int topLeft, topRight, bottomLeft, bottomRight, previousY2, previous;
	int tempX, tempY, wasOnGround;

	wasOnGround = (e->flags & ON_GROUND);

	/* Remove the entity from the ground */

	e->flags &= ~ON_GROUND;

	/* Set environment to air */

	previousEnvironment = e->environment;

	e->environment = AIR;

	/* Test the horizontal movement first */

	i = e->h > TILE_SIZE ? TILE_SIZE : e->h;

	for (;;)
	{
		x1 = (e->x + e->dirX) / TILE_SIZE;
		x2 = (e->x + e->dirX + e->w - 1) / TILE_SIZE;

		y1 = (e->y) / TILE_SIZE;
		y2 = (e->y + i - 1) / TILE_SIZE;

		if (x1 >= 0 && x2 < MAX_MAP_X && y1 >= 0 && y2 < MAX_MAP_Y)
		{
			topLeft     = mapTileAt(x1, y1);
			topRight    = mapTileAt(x2, y1);
			bottomLeft  = mapTileAt(x1, y2);
			bottomRight = mapTileAt(x2, y2);

			if (e->dirX > 0)
			{
				/* Trying to move right */

				previous = (e->x + e->w - 1) / TILE_SIZE;

				previous = mapTileAt(previous, y2);

				if (previous >= SLOPE_UP_START && previous <= SLOPE_UP_END)
				{
					if (i == e->h)
					{
						if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
						{
							e->y -= e->dirX;

							e->dirY = 0;

							e->flags |= ON_GROUND;
						}

						else if (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START)
						{
							e->y = (int)((e->y + e->h - 1) / TILE_SIZE);

							e->y *= TILE_SIZE;

							e->y -= e->h;

							e->dirY = 0;

							e->flags |= ON_GROUND;

							previous = mapTileAt(x2, (e->y + e->h - 1) / TILE_SIZE);

							if (previous >= SLOPE_UP_START && previous <= SLOPE_UP_END)
							{
								e->y -= e->dirX;

								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}
				}

				else if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					if (i == e->h)
					{
						if (!(e->flags & FLY))
						{
							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}

						else
						{
							e->x = x2 * TILE_SIZE;

							e->x -= e->w;

							e->dirX = 0;
						}

						if (e->type == PROJECTILE)
						{
							e->die();

							return;
						}
					}
				}

				else if ((previous >= SLOPE_UP_START && previous <= SLOPE_UP_END) &&
					!(bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END))
				{
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					e->flags |= ON_GROUND;
				}

				else if ((topRight >= JUMP_THROUGH_TILE_START && topRight <= JUMP_THROUGH_TILE_END) ||
					((bottomRight >= JUMP_THROUGH_TILE_START && bottomRight <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topRight != BLANK_TILE && topRight < BACKGROUND_TILE_START) || (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->x = x2 * TILE_SIZE;

					e->x -= e->w;

					e->dirX = (e->flags & BOUNCES) ? -e->dirX : 0;

					if ((e->flags & GRABBING) && e->target != NULL)
					{
						e->target->dirX = 0;
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}
			}

			else if (e->dirX < 0)
			{
				/* Trying to move left */

				previous = (e->x) / TILE_SIZE;

				previous = mapTileAt(previous, y2);

				if (previous >= SLOPE_DOWN_START && previous <= SLOPE_DOWN_END)
				{
					if (i == e->h)
					{
						if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
						{
							e->y += e->dirX;

							e->dirY = 0;

							e->flags |= ON_GROUND;
						}

						else if (bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START)
						{
							e->y = (int)((e->y + e->h - 1) / TILE_SIZE);

							e->y *= TILE_SIZE;

							e->y -= e->h;

							e->dirY = 0;

							e->flags |= ON_GROUND;

							previous = mapTileAt(x1, (e->y + e->h - 1) / TILE_SIZE);

							if (previous >= SLOPE_DOWN_START && previous <= SLOPE_DOWN_END)
							{
								e->y += e->dirX;

								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					if (i == e->h)
					{
						if (!(e->flags & FLY))
						{
							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}

						else
						{
							e->x = (x1 + 1) * TILE_SIZE;

							e->dirX = 0;
						}

						if (e->type == PROJECTILE)
						{
							e->die();

							return;
						}
					}
				}

				else if ((previous >= SLOPE_DOWN_START && previous <= SLOPE_DOWN_END) &&
					!(bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END))
				{
				}

				else if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					e->flags |= ON_GROUND;
				}

				else if ((topLeft >= JUMP_THROUGH_TILE_START && topLeft <= JUMP_THROUGH_TILE_END) ||
					((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topLeft != BLANK_TILE && topLeft < BACKGROUND_TILE_START) || (bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->x = (x1 + 1) * TILE_SIZE;

					e->dirX = (e->flags & BOUNCES) ? -e->dirX : 0;

					if ((e->flags & GRABBING) && e->target != NULL)
					{
						e->target->dirX = 0;
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}
			}
		}

		/* Exit this loop if we have tested all of the body */

		if (i == e->h || e->h <= TILE_SIZE)
		{
			break;
		}

		/* Test the next block */

		i += TILE_SIZE;

		if (i > e->h)
		{
			i = e->h;
		}
	}

	/* Now test the vertical movement */

	i = e->w > TILE_SIZE ? TILE_SIZE : e->w;

	for (;;)
	{
		x1 = (e->x) / TILE_SIZE;
		x2 = (e->x + i - 1) / TILE_SIZE;

		y1 = (e->y + e->dirY) / TILE_SIZE;
		y2 = (e->y + e->dirY + e->h - 1) / TILE_SIZE;

		if (x1 >= 0 && x2 < MAX_MAP_X && y1 >= 0 && y2 < MAX_MAP_Y)
		{
			topLeft     = mapTileAt(x1, y1);
			topRight    = mapTileAt(x2, y1);
			bottomLeft  = mapTileAt(x1, y2);
			bottomRight = mapTileAt(x2, y2);

			if (e->dirY > 0)
			{
				/* Trying to move down */

				if (bottomRight >= SLOPE_UP_START && bottomRight <= SLOPE_UP_END)
				{
					if (i == e->w)
					{
						tempX = (int)(e->x + i - 1) % TILE_SIZE;
						tempY = (int)(e->y + e->dirY + e->h - 1) % TILE_SIZE;

						tempX = TILE_SIZE - tempX;

						if (tempY > tempX || wasOnGround != 0)
						{
							e->y = y2 * TILE_SIZE;
							e->y -= e->h;
							e->y += tempX + 1;

							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}

				else if (bottomLeft >= SLOPE_DOWN_START && bottomLeft <= SLOPE_DOWN_END)
				{
					if (i == (e->w > TILE_SIZE ? TILE_SIZE : e->w))
					{
						tempX = (int)(e->x) % TILE_SIZE;
						tempY = (int)(e->y + e->dirY + e->h - 1) % TILE_SIZE;

						if (tempY > tempX || wasOnGround != 0)
						{
							e->y = y2 * TILE_SIZE;
							e->y -= e->h;
							e->y += tempX + 1;

							if ((e->flags & BOUNCES) && e->dirY > 4)
							{
								e->dirY = -e->dirY * 2 / 3;
							}

							else
							{
								e->dirY = 0;

								e->flags |= ON_GROUND;
							}
						}
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}

				else if ((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END) ||
					((bottomRight >= JUMP_THROUGH_TILE_START && bottomRight <= JUMP_THROUGH_TILE_END)))
				{
					previousY2 = y2;

					x1 = (e->x) / TILE_SIZE;
					x2 = (e->x + i - 1) / TILE_SIZE;

					y2 = (e->y + e->h - 1) / TILE_SIZE;

					bottomLeft  = mapTileAt(x1, y2);
					bottomRight = mapTileAt(x2, y2);

					if (!((bottomLeft >= JUMP_THROUGH_TILE_START && bottomLeft <= JUMP_THROUGH_TILE_END) ||
						((bottomRight >= JUMP_THROUGH_TILE_START && bottomRight <= JUMP_THROUGH_TILE_END))))
					{
						y2 = previousY2;

						/* Place the player as close to the solid tile as possible */

						e->y = y2 * TILE_SIZE;
						e->y -= e->h;

						if ((e->flags & BOUNCES) && e->dirY > 4)
						{
							e->dirY = -e->dirY * 2 / 3;
						}

						else
						{
							e->dirY = 0;

							e->flags |= ON_GROUND;
						}
					}
				}

				else if ((bottomLeft != BLANK_TILE && bottomLeft < BACKGROUND_TILE_START) || (bottomRight != BLANK_TILE && bottomRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->y = y2 * TILE_SIZE;
					e->y -= e->h;

					if ((e->flags & BOUNCES) && e->dirY > 4)
					{
						e->dirY = -e->dirY * 2 / 3;
					}

					else
					{
						e->dirY = 0;

						e->flags |= ON_GROUND;
					}

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}
			}

			else if (e->dirY < 0)
			{
				/* Trying to move up */

				if ((topLeft >= JUMP_THROUGH_TILE_START && topLeft <= JUMP_THROUGH_TILE_END) ||
					((topRight >= JUMP_THROUGH_TILE_START && topRight <= JUMP_THROUGH_TILE_END)))
				{

				}

				else if ((topLeft != BLANK_TILE && topLeft < BACKGROUND_TILE_START) || (topRight != BLANK_TILE && topRight < BACKGROUND_TILE_START))
				{
					/* Place the player as close to the solid tile as possible */

					e->y = (y1 + 1) * TILE_SIZE;

					e->dirY = (e->flags & BOUNCES) ? -e->dirY : 0;

					if (e->type == PROJECTILE)
					{
						e->die();

						return;
					}
				}
			}
		}

		if (i == e->w || e->w <= TILE_SIZE)
		{
			break;
		}

		i += TILE_SIZE;

		if (i > e->w)
		{
			i = e->w;
		}
	}

	/* Now apply the movement */

	e->x += e->dirX;
	e->y += e->dirY;

	x1 = e->type == PLAYER ? getMinMapX() : 0;
	y1 = e->type == PLAYER ? getMinMapY() : 0;

	if (e->x < x1)
	{
		e->x = x1;

		e->dirX = 0;

		if ((e->flags & GRABBING) && e->target != NULL)
		{
			e->target->dirX = 0;
		}

		if (e->type == PROJECTILE)
		{
			e->die();
		}

	}

	else if (e->x + e->w >= maxMapX())
	{
		e->x = maxMapX() - e->w - 1;

		e->dirX = 0;

		if ((e->flags & GRABBING) && e->target != NULL)
		{
			e->target->dirX = 0;
		}

		if (e->type == PROJECTILE)
		{
			e->die();
		}
	}

	if (e->y > maxMapY() && e->y - e->dirY <= maxMapY())
	{
		e->flags &= ~HELPLESS|INVULNERABLE;

		e->fallout();
	}

	else if (e->y < -300)
	{
		/* Way too high... */

		e->dirY = 0;
	}

	x1 = (e->x) / TILE_SIZE;
	x2 = (e->x + e->w - 1) / TILE_SIZE;

	y1 = (e->y) / TILE_SIZE;
	y2 = (e->y + e->h - 1) / TILE_SIZE;

	topLeft     = mapTileAt(x1, y1);
	topRight    = mapTileAt(x2, y1);
	bottomLeft  = mapTileAt(x1, y2);
	bottomRight = mapTileAt(x2, y2);

	if ((topLeft >= LAVA_TILE_START && topLeft <= LAVA_TILE_END) ||
		(bottomLeft >= LAVA_TILE_START && bottomLeft <= LAVA_TILE_END) ||
		(topRight >= LAVA_TILE_START && topRight <= LAVA_TILE_END) ||
		(bottomRight >= LAVA_TILE_START && bottomRight <= LAVA_TILE_END))
	{
		e->environment = LAVA;

		if (previousEnvironment != LAVA && e->fallout != NULL)
		{
			/* Fire based entities won't die */

			if (e->element != FIRE)
			{
				playSoundToMap("sound/common/lava.ogg", -1, self->x, self->y, 0);

				e->flags &= ~(HELPLESS|INVULNERABLE);

				e->fallout();
			}
		}
	}

	else if ((topLeft >= SLIME_TILE_START && topLeft <= SLIME_TILE_END) ||
		(bottomLeft >= SLIME_TILE_START && bottomLeft <= SLIME_TILE_END) ||
		(topRight >= SLIME_TILE_START && topRight <= SLIME_TILE_END) ||
		(bottomRight >= SLIME_TILE_START && bottomRight <= SLIME_TILE_END))
	{
		e->environment = SLIME;

		if (previousEnvironment != SLIME && e->fallout != NULL)
		{
			/* Slime based entities won't die */

			if (e->element != SLIME)
			{
				playSoundToMap("sound/common/slime.ogg", -1, self->x, self->y, 0);

				e->flags &= ~(HELPLESS|INVULNERABLE);

				e->fallout();
			}
		}
	}

	else
	{
		y2 = (e->y + (e->h / 2)) / TILE_SIZE;

		bottomLeft  = mapTileAt(x1, y2);
		bottomRight = mapTileAt(x2, y2);

		if ((topLeft >= WATER_TILE_START && topLeft <= WATER_TILE_END) &&
			(bottomLeft >= WATER_TILE_START && bottomLeft <= WATER_TILE_END) &&
			(topRight >= WATER_TILE_START && topRight <= WATER_TILE_END) &&
			(bottomRight >= WATER_TILE_START && bottomRight <= WATER_TILE_END))
		{
			e->environment = WATER;

			if (previousEnvironment != WATER && e->fallout != NULL)
			{
				playSoundToMap("sound/common/splash.ogg", -1, self->x, self->y, 0);

				if (!(e->flags & FLOATS))
				{
					e->flags &= ~(HELPLESS|INVULNERABLE);

					e->fallout();
				}
			}
		}
	}
}

int isAtEdge(Entity *e)
{
	int i, tile;
	int x = e->x + (e->dirX > 0 ? e->w : 0);
	int y = e->y + e->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	tile = mapTileAt(x, y);

	/* Return immediately if the tile isn't blank */

	if (!(e->flags & ON_GROUND) || (tile != BLANK_TILE && tile < BACKGROUND_TILE_START))
	{
		return FALSE;
	}

	if (e->w > TILE_SIZE)
	{
		if (e->dirX > 0)
		{
			for (i=0;;)
			{
				x = e->x + i;

				x /= TILE_SIZE;

				tile = mapTileAt(x, y);

				if (tile >= SLOPE_DOWN_START && tile <= SLOPE_DOWN_END)
				{
					return FALSE;
				}

				if (i == e->w)
				{
					break;
				}

				i += TILE_SIZE;

				if (i > e->w)
				{
					i = e->w;
				}
			}
		}

		else
		{
			for (i=e->w;;)
			{
				x = e->x + i;

				x /= TILE_SIZE;

				tile = mapTileAt(x, y);

				if (tile >= SLOPE_UP_START && tile <= SLOPE_UP_END)
				{
					return FALSE;
				}

				if (i == 0)
				{
					break;
				}

				i -= TILE_SIZE;

				if (i < 0)
				{
					i = 0;
				}
			}
		}
	}

	/* There might still be Entities that can be walked on */

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (e != &entity[i] && entity[i].inUse == TRUE && ((entity[i].flags & PUSHABLE) || (entity[i].type == WEAK_WALL)))
		{
			if (collision(e->x + (e->face == LEFT ? 0 : e->w), e->y + e->h, 1, 128, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == TRUE)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

int isValidOnMap(Entity *e)
{
	int i, x1, x2, y1, y2;

	i = e->w > TILE_SIZE ? TILE_SIZE : e->w;

	for (;;)
	{
		x1 = (e->x) / TILE_SIZE;
		x2 = (e->x + i - 1) / TILE_SIZE;

		y1 = (e->y) / TILE_SIZE;
		y2 = (e->y + e->h - 1) / TILE_SIZE;

		if ((mapTileAt(x1, y1) < BACKGROUND_TILE_START && mapTileAt(x1, y1) > BLANK_TILE) ||
			(mapTileAt(x2, y1) < BACKGROUND_TILE_START && mapTileAt(x2, y1) > BLANK_TILE) ||
			(mapTileAt(x1, y2) < BACKGROUND_TILE_START && mapTileAt(x1, y2) > BLANK_TILE) ||
			(mapTileAt(x2, y2) < BACKGROUND_TILE_START && mapTileAt(x2, y2) > BLANK_TILE))
		{
			return FALSE;
		}

		if (i == e->w)
		{
			break;
		}

		i += e->w;

		if (i > e->w)
		{
			i = e->w;
		}
	}

	i = e->h > TILE_SIZE ? TILE_SIZE : e->h;

	for (;;)
	{
		x1 = (e->x) / TILE_SIZE;
		x2 = (e->x + e->w - 1) / TILE_SIZE;

		y1 = (e->y) / TILE_SIZE;
		y2 = (e->y + i - 1) / TILE_SIZE;

		if ((mapTileAt(x1, y1) < BACKGROUND_TILE_START && mapTileAt(x1, y1) > BLANK_TILE) ||
			(mapTileAt(x2, y1) < BACKGROUND_TILE_START && mapTileAt(x2, y1) > BLANK_TILE) ||
			(mapTileAt(x1, y2) < BACKGROUND_TILE_START && mapTileAt(x1, y2) > BLANK_TILE) ||
			(mapTileAt(x2, y2) < BACKGROUND_TILE_START && mapTileAt(x2, y2) > BLANK_TILE))
		{
			return FALSE;
		}

		if (i == e->h)
		{
			break;
		}

		i += e->h;

		if (i > e->h)
		{
			i = e->h;
		}
	}

	return TRUE;
}

/* Very standard 2D collision detection routine */

int collision(int x0, int y0, int w0, int h0, int x2, int y2, int w1, int h1)
{
	int x1 = x0 + w0 - 1;
	int y1 = y0 + h0 - 1;

	int x3 = x2 + w1 - 1;
	int y3 = y2 + h1 - 1;

	return !(x1<x2 || x3<x0 || y1<y2 || y3<y0);
}