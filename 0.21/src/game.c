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

#include "game.h"
#include "weather.h"
#include "graphics/font.h"
#include "system/random.h"
#include "audio/music.h"
#include "graphics/graphics.h"
#include "system/resources.h"
#include "map.h"
#include "system/load_save.h"
#include "entity.h"
#include "player.h"
#include "menu/menu.h"
#include "menu/main_menu.h"
#include "menu/io_menu.h"

extern Game game;

static void shake(void);
static void wipeOutRightToLeft(void);
static void wipeOutLeftToRight(void);
static void wipeInRightToLeft(void);
static void wipeInLeftToRight(void);
static void wipeInCircleToLarge(void);
static void wipeInCircleToSmall(void);
static void wipeOutCircleToLarge(void);
static void wipeOutCircleToSmall(void);

void initGame()
{
	game.drawScreen = TRUE;

	if (game.paused == TRUE)
	{
		pauseGame();
	}

	game.status = IN_GAME;

	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}

void doGame()
{
	/* Decrease game's thinktime */

	game.thinkTime--;

	if (game.thinkTime <= 0)
	{
		/* Execute the action if there is one */

		if (game.action != NULL)
		{
			game.action();
		}

		game.thinkTime = 0;
	}

	if (game.shakeThinkTime > 0 || game.shakeThinkTime == -1)
	{
		shake();

		if (game.shakeThinkTime > 0)
		{
			game.shakeThinkTime--;

			if (game.shakeThinkTime <= 0)
			{
				game.offsetX = game.offsetY = 0;
			}
		}
	}

	if (game.weatherType != NO_WEATHER)
	{
		game.weatherAction();
	}

	game.playTime++;
}

void freeGame()
{
	if (game.pauseSurface != NULL)
	{
		SDL_FreeSurface(game.pauseSurface);

		game.pauseSurface = NULL;
	}

	if (game.gameOverSurface != NULL)
	{
		SDL_FreeSurface(game.gameOverSurface);

		game.gameOverSurface = NULL;
	}

	if (game.pauseSurface != NULL)
	{
		SDL_FreeSurface(game.pauseSurface);

		game.pauseSurface = NULL;
	}
}

void drawGame()
{
	if (game.weatherType != NO_WEATHER)
	{
		game.weatherDraw();
	}

	if (game.transition != NULL)
	{
		game.transition();
	}

}

void setTransition(int type, void (*func)(void))
{
	if (type == TRANSITION_OUT)
	{
		switch (prand() % MAX_OUT_TRANSITIONS)
		{
			case WIPE_OUT_RIGHT_TO_LEFT:
				game.transitionX = SCREEN_WIDTH;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeOutRightToLeft;
			break;

			case WIPE_OUT_CIRCLE_TO_LARGE:
				game.transitionX = 0;
				game.transition = &wipeOutCircleToLarge;
			break;

			case WIPE_OUT_CIRCLE_TO_SMALL:
				game.transitionX = SCREEN_WIDTH;
				game.transition = &wipeOutCircleToSmall;
			break;

			default:
				game.transitionX = 0;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeOutLeftToRight;
			break;
		}
	}

	else
	{
		switch (prand() % MAX_IN_TRANSITIONS)
		{
			case WIPE_IN_RIGHT_TO_LEFT:
				game.transitionX = SCREEN_WIDTH;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeInRightToLeft;
			break;

			case WIPE_IN_CIRCLE_TO_LARGE:
				game.transitionX = 0;
				game.transition = &wipeInCircleToLarge;
			break;

			case WIPE_IN_CIRCLE_TO_SMALL:
				game.transitionX = SCREEN_WIDTH;
				game.transition = &wipeInCircleToSmall;
			break;

			default:
				game.transitionX = 0;
				game.transitionY = SCREEN_HEIGHT;
				game.transition = &wipeInLeftToRight;
			break;
		}
	}

	game.transitionCallback = func;
}

void shakeScreen(int shakeStrength, int time)
{
	game.shakeThinkTime = time;
	game.shakeStrength = shakeStrength;
}

static void shake()
{
	switch (game.shakeStrength)
	{
		case LIGHT:
			game.offsetX = prand() % 2 * (prand() % 2 == 0 ? -1 : 1);
			game.offsetY = prand() % 2 * (prand() % 2 == 0 ? -1 : 1);
		break;

		case MEDIUM:
			game.offsetX = prand() % 4 * (prand() % 2 == 0 ? -1 : 1);
			game.offsetY = prand() % 4 * (prand() % 2 == 0 ? -1 : 1);
		break;

		case STRONG:
			game.offsetX = prand() % 6 * (prand() % 2 == 0 ? -1 : 1);
			game.offsetY = prand() % 6 * (prand() % 2 == 0 ? -1 : 1);
		break;
	}
}

static void wipeOutRightToLeft()
{
	fadeOutMusic(1000);

	if (game.transitionX <= -15)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(game.transitionX < 0 ? 0 : game.transitionX, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX -= 15;
}

static void wipeOutLeftToRight()
{
	fadeOutMusic(1000);

	if (game.transitionX > SCREEN_WIDTH + 15)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX += 15;
}

static void wipeInLeftToRight()
{
	if (game.transitionX >= SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(game.transitionX, 0, SCREEN_WIDTH - game.transitionX, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX += 15;
}

static void wipeInRightToLeft()
{
	if (game.transitionX < 0)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawBox(0, 0, game.transitionX, SCREEN_HEIGHT, 0, 0, 0);

	game.transitionX -= 15;
}

static void wipeInCircleToLarge()
{
	if (game.transitionX > SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircleFromSurface(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX);

	game.transitionX += 10;
}

static void wipeInCircleToSmall()
{
	if (game.transitionX < 0)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX, 0, 0, 0);

	game.transitionX -= 10;
}

static void wipeOutCircleToLarge()
{
	if (game.transitionX > SCREEN_WIDTH)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX, 0, 0, 0);

	game.transitionX += 10;
}

static void wipeOutCircleToSmall()
{
	if (game.transitionX <= -10)
	{
		game.transition = NULL;

		if (game.transitionCallback != NULL)
		{
			game.transitionCallback();
		}

		return;
	}

	drawCircleFromSurface(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, game.transitionX < 0 ? 0 : game.transitionX);

	game.transitionX -= 10;
}

void setNextLevel(char *name, char *playerStart)
{
	STRNCPY(game.nextMap, name, sizeof(game.nextMap));
	STRNCPY(game.playerStart, playerStart, sizeof(game.playerStart));
}

void setNextLevelFromScript(char *token)
{
	char *name, *playerStart, *savePtr;

	name = strtok_r(token, " ", &savePtr);
	playerStart = strtok_r(NULL, "\0", &savePtr);

	setNextLevel(name, playerStart);

	setTransition(TRANSITION_OUT, &goToNextMap);
}

void goToNextMap()
{
	Entity *start;

	saveTemporaryData();

	printf("Freeing Resources\n");

	freeLevelResources();

	printf("Loading Map\n");

	if (hasPersistance(game.nextMap) == FALSE)
	{
		loadMap(game.nextMap, TRUE);
	}

	else
	{
		printf("Loading persistance data instead...\n");

		loadMap(game.nextMap, FALSE);

		loadPersitanceData(game.nextMap);
	}

	printf("Done\n");

	if (strcmpignorecase(game.playerStart, "PLAYER_START") != 0)
	{
		start = getEntityByObjectiveName(game.playerStart);

		if (start == NULL)
		{
			printf("Could not find player start %s\n", game.playerStart);

			exit(1);
		}

		loadPlayer(start->x, start->y, NULL);
	}

	game.nextMap[0] = '\0';
	game.playerStart[0] = '\0';

	clearScreen(0, 0, 0);
}

void setCheckpoint(float x, float y)
{
	game.checkpointX = x;
	game.checkpointY = y;
}

void getCheckpoint(float *x, float *y)
{
	*x = game.checkpointX;
	*y = game.checkpointY;
}

void pauseGame()
{
	switch (game.status)
	{
		case IN_GAME:
			game.paused = TRUE;

			game.status = IN_MENU;

			if (game.pauseSurface == NULL)
			{
				game.pauseSurface = createSurface(game.screen->w, game.screen->h);

				SDL_BlitSurface(game.screen, NULL, game.pauseSurface, NULL);
			}
		break;

		default:
			if (game.menu->returnAction != NULL)
			{
				game.menu->returnAction();
			}

			else
			{
				if (game.pauseSurface != NULL)
				{
					SDL_FreeSurface(game.pauseSurface);

					game.pauseSurface = NULL;
				}

				game.paused = FALSE;

				game.status = IN_GAME;

				game.menu = initMainMenu();

				game.drawMenu = &drawMainMenu;
			}
		break;
	}
}

void showSaveDialog()
{
	pauseGame();

	game.status = IN_MENU;

	game.paused = TRUE;

	game.menu = initIOMenu(TRUE);

	game.drawMenu = &drawIOMenu;
}

void pauseGameInventory()
{
	switch (game.status)
	{
		case IN_GAME:
			game.status = IN_INVENTORY;

			if (game.pauseSurface == NULL)
			{
				game.pauseSurface = createSurface(game.screen->w, game.screen->h);

				SDL_BlitSurface(game.screen, NULL, game.pauseSurface, NULL);
			}
		break;

		case IN_INVENTORY:
			if (game.pauseSurface != NULL)
			{
				SDL_FreeSurface(game.pauseSurface);

				game.pauseSurface = NULL;
			}

			game.status = IN_GAME;
		break;
	}
}

void focusLost()
{
	#if DEV == 0
		if (game.paused == FALSE && game.status != IN_INVENTORY && game.status != IN_EDITOR)
		{
			pauseGame();
		}
	#endif
}

void showPauseDialog()
{
	drawImage(game.pauseSurface, 0, 0, FALSE);
}

void resetGameSettings()
{
	game.audio = TRUE;
	game.sfxDefaultVolume = 5;
	game.musicDefaultVolume = 10;
	game.showHints = TRUE;
	game.fullscreen = FALSE;
}

void writeGameSettingsToFile(FILE *fp)
{
	fprintf(fp, "GAME_SETTINGS\n");
	fprintf(fp, "AUDIO %d\n", game.audio);
	fprintf(fp, "SFX_VOLUME %d\n", game.sfxDefaultVolume);
	fprintf(fp, "MUSIC_VOLUME %d\n", game.musicDefaultVolume);
	fprintf(fp, "HINTS %d\n", game.showHints);
	fprintf(fp, "FULLSCREEN %d\n", game.fullscreen);
}

void readGameSettingsFromFile(char *buffer)
{
	char *line, *token, *savePtr;

	line = strtok_r(buffer, "\n", &savePtr);

	while (line != NULL)
	{
		token = strtok(line, " ");

		if (strcmpignorecase(token, "AUDIO") == 0)
		{
			token = strtok(NULL, "\0");

			game.audio = atoi(token);
		}

		else if (strcmpignorecase(token, "SFX_VOLUME") == 0)
		{
			token = strtok(NULL, "\0");

			game.sfxDefaultVolume = atoi(token);
		}

		else if (strcmpignorecase(token, "MUSIC_VOLUME") == 0)
		{
			token = strtok(NULL, "\0");

			game.musicDefaultVolume = atoi(token);
		}

		else if (strcmpignorecase(token, "HINTS") == 0)
		{
			token = strtok(NULL, "\0");

			game.showHints = atoi(token);
		}

		else if (strcmpignorecase(token, "FULLSCREEN") == 0)
		{
			token = strtok(NULL, "\0");

			game.fullscreen = atoi(token);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}
}

void doGameOver()
{
	if (game.gameOverSurface == NULL)
	{
		game.gameOverSurface = loadImage("gfx/common/gameover.png");
	}

	game.transitionX = 0;
}

void drawGameOver()
{
	game.transitionX += 3;

	if (game.transitionX >= game.gameOverSurface->w)
	{
		game.transitionX = game.gameOverSurface->w;
	}

	drawClippedImage(game.gameOverSurface, 0, 0, (SCREEN_WIDTH - game.gameOverSurface->w) / 2, (SCREEN_HEIGHT - game.gameOverSurface->h) / 2, game.transitionX, game.gameOverSurface->h);
}

char *getPlayTimeAsString()
{
	/* 1 second is 60 frames */

	int hours, minutes;
	long tempTime;
	char *timeString;

	timeString = (char *)malloc(15 * sizeof(char));

	if (timeString == NULL)
	{
		printf("Failed to allocate a whole %d bytes for Play Time string...\n", 15 * (int)sizeof(char));

		exit(0);
	}

	tempTime = game.playTime;

	hours = tempTime / (60 * 60 * 60);

	tempTime -= hours * 60 * 60 * 60;

	minutes = tempTime / (60 * 60);

	snprintf(timeString, 15, "%dH %dM", hours, minutes);

	return timeString;
}