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

#include "system/random.h"
#include "map.h"
#include "graphics/graphics.h"
#include "draw.h"
#include "weather.h"

static Droplet droplet[MAX_DROPS];
extern Game game;

static void initLightRain(void);
static void initHeavyRain(void);
static void initStorm(void);
static void initSnow(void);
static void rain(void);
static void snow(void);
static void storm(void);
static void drawRain(void);
static void drawSnow(void);

static Type weatherType[] = {
					{NO_WEATHER, "NO_WEATHER"},
					{LIGHT_RAIN, "LIGHT_RAIN"},
					{HEAVY_RAIN, "HEAVY_RAIN"},
					{STORMY, "STORMY"},
					{SNOW, "SNOW"},
					};
static int weatherLength = sizeof(weatherType) / sizeof(Type);

void setWeather(int weatherType)
{
	switch (weatherType)
	{
		case LIGHT_RAIN:
			game.weatherAction = &initLightRain;
			game.weatherDraw = &drawRain;
		break;

		case HEAVY_RAIN:
			game.weatherAction = &initHeavyRain;
			game.weatherDraw = &drawRain;
		break;

		case STORMY:
			game.weatherAction = &initStorm;
			game.weatherDraw = &drawRain;
		break;

		case SNOW:
			game.weatherAction = &initSnow;
			game.weatherDraw = &drawSnow;
		break;

		default:
			game.weatherAction = NULL;
			game.weatherDraw = NULL;
		break;

	}

	game.weatherType = weatherType;
	game.weatherThinkTime = 60;
}

static void initLightRain()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS/2;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0;
		droplet[i].dirY = 8 + prand() % 8;

		droplet[i].active = TRUE;
	}

	game.weatherAction = &rain;
}

static void initHeavyRain()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0;
		droplet[i].dirY = 8 + prand() % 8;

		droplet[i].active = TRUE;
	}

	game.weatherAction = &rain;
}

static void rain()
{
	int i, maxY, startX;

	maxY = maxMapY();

	startX = getMapStartX();

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			droplet[i].y += droplet[i].dirY;

			if (droplet[i].y >= maxY || mapTileAt((startX + droplet[i].x) / TILE_SIZE, droplet[i].y / TILE_SIZE) != BLANK_TILE)
			{
				droplet[i].x = prand() % SCREEN_WIDTH;
				droplet[i].y = -8 - prand() % 20;

				droplet[i].dirX = 0;
				droplet[i].dirY = 8 + prand() % 8;
			}
		}
	}
}

static void initStorm()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0;
		droplet[i].dirY = 8 + prand() % 8;

		droplet[i].active = TRUE;
	}

	game.weatherAction = &storm;
}

static void storm()
{
	rain();

	game.weatherThinkTime--;

	if (game.weatherThinkTime < -60)
	{
		game.weatherThinkTime = 600 + prand() % 1200;
	}
}

static void initSnow()
{
	int i;

	memset(droplet, 0, sizeof(Droplet) * MAX_DROPS);

	for (i=0;i<MAX_DROPS;i++)
	{
		droplet[i].x = prand() % SCREEN_WIDTH;
		droplet[i].y = prand() % SCREEN_HEIGHT;

		droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
		droplet[i].dirY = 01.f + 0.1f * (prand() % 10);

		droplet[i].active = TRUE;
	}

	game.weatherAction = &snow;
}

static void snow()
{
	int i, startX, maxY;

	maxY = maxMapY();

	startX = getMapStartX();

	for (i=0;i<MAX_DROPS;i++)
	{
		droplet[i].x += droplet[i].dirX;
		droplet[i].y += droplet[i].dirY;

		droplet[i].x = (int)droplet[i].x % SCREEN_WIDTH;

		if (prand() % 30 == 0)
		{
			droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
		}

		if (droplet[i].y >= maxY || mapTileAt((droplet[i].x + startX) / TILE_SIZE, droplet[i].y / TILE_SIZE) != BLANK_TILE)
		{
			droplet[i].x = prand() % SCREEN_WIDTH;
			droplet[i].y = -8 - prand() % 20;

			droplet[i].dirX = 0.1f * (prand() % 20) - 0.1f * (prand() % 20);
			droplet[i].dirY = 01.f + 0.1f * (prand() % 10);
		}
	}
}

static void drawRain()
{
	int i, startY;

	if (game.weatherThinkTime >= -10 && game.weatherThinkTime < 0)
	{
		clearScreen(255, 255, 255);
	}

	else
	{
		startY = getMapStartY();

		for (i=0;i<MAX_DROPS;i++)
		{
			if (droplet[i].active == TRUE)
			{
				drawBox(droplet[i].x, droplet[i].y - startY, 1, 8, 220, 220, 220);
			}
		}
	}
}

static void drawSnow()
{
	int i, startY;

	startY = getMapStartY();

	for (i=0;i<MAX_DROPS;i++)
	{
		if (droplet[i].active == TRUE)
		{
			drawBox(droplet[i].x, droplet[i].y - startY, 2, 2, 255, 255, 255);
		}
	}
}

int getWeatherTypeByName(char *name)
{
	int i;

	for (i=0;i<weatherLength;i++)
	{
		if (strcmpignorecase(name, weatherType[i].name) == 0)
		{
			return weatherType[i].id;
		}
	}

	printf("Unknown Weather Type %s\n", name);

	exit(1);
}

char *getWeatherTypeByID(int id)
{
	int i;

	for (i=0;i<weatherLength;i++)
	{
		if (id == weatherType[i].id)
		{
			return weatherType[i].name;
		}
	}

	printf("Unknown Weather ID %d\n", id);

	exit(1);
}

char *getWeather()
{
	return getWeatherTypeByID(game.weatherType);
}