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

#include "widget.h"
#include "../draw.h"
#include "label.h"
#include "../init.h"
#include "../graphics/graphics.h"
#include "options_menu.h"
#include "../system/pak.h"
#include "../input.h"
#include "../audio/music.h"
#include "../audio/audio.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static char *getVolumePercent(int);
static void toggleSound(void);
static void realignGrid(void);
static void changeVolume(int *, int *, Widget *, int);
static void lowerSFXVolume(void);
static void raiseSFXVolume(void);
static void lowerMusicVolume(void);
static void raiseMusicVolume(void);

void drawSoundMenu()
{
	int i;

	drawImage(menu.background, menu.x, menu.y, FALSE);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], &menu, menu.index == i);
	}
}

static void doMenu()
{
	Widget *w;

	if (input.down == TRUE || menuInput.down == TRUE)
	{
		menu.index++;

		if (menu.index == menu.widgetCount)
		{
			menu.index = 0;
		}

		menuInput.down = FALSE;
		input.down = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.up == TRUE || menuInput.up == TRUE)
	{
		menu.index--;

		if (menu.index < 0)
		{
			menu.index = menu.widgetCount - 1;
		}

		menuInput.up = FALSE;
		input.up = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->clickAction != NULL)
		{
			w->clickAction();
		}

		menuInput.attack = FALSE;
		input.attack = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.left == TRUE || menuInput.left == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			w->leftAction();
		}

		menuInput.left = FALSE;
		input.left = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.right == TRUE || menuInput.right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->rightAction != NULL)
		{
			w->rightAction();
		}

		menuInput.right = FALSE;
		input.right = FALSE;

		playSound("sound/common/click.ogg");
	}
}

static void loadMenuLayout()
{
	char filename[MAX_LINE_LENGTH], *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2, *text;
	unsigned char *buffer;
	int x, y, i;

	i = 0;

	snprintf(filename, sizeof(filename), _("data/menu/sound_menu.dat"));

	buffer = loadFileFromPak(filename);

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	while (line != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr1);

			continue;
		}

		token = strtok_r(line, " ", &savePtr2);

		if (strcmpignorecase(token, "WIDTH") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.w = atoi(token);
		}

		else if (strcmpignorecase(token, "HEIGHT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.h = atoi(token);
		}

		else if (strcmpignorecase(token, "WIDGET_COUNT") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr2);

			menu.widgetCount = atoi(token);

			menu.widgets = (Widget **)malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				printf("Ran out of memory when creating Control Menu\n");

				exit(1);
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			if (menu.widgets != NULL)
			{
				token = strtok_r(NULL, "\0", &savePtr2);

				sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

				if (strcmpignorecase(menuID, "SOUND") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_UP], &toggleSound, &toggleSound, &toggleSound, x, y, TRUE);

					menu.widgets[i]->label = createLabel(game.audio == TRUE ? _("Yes") : _("No"), menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);
				}

				else if (strcmpignorecase(menuID, "SFX_VOLUME") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &game.sfxDefaultVolume, &lowerSFXVolume, &raiseSFXVolume, NULL, x, y, TRUE);

					text = getVolumePercent(game.sfxDefaultVolume);

					menu.widgets[i]->label = createLabel(text, menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					free(text);
				}

				else if (strcmpignorecase(menuID, "MUSIC_VOLUME") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &game.musicDefaultVolume, &lowerMusicVolume, &raiseMusicVolume, NULL, x, y, TRUE);

					text = getVolumePercent(game.musicDefaultVolume);

					menu.widgets[i]->label = createLabel(text, menu.widgets[i]->x + menu.widgets[i]->normalState->w + 10, y);

					free(text);
				}

				else if (strcmpignorecase(menuID, "MENU_BACK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, &showOptionsMenu, x, y, TRUE);
				}

				else
				{
					printf("Unknown widget %s\n", menuID);

					exit(1);
				}

				i++;
			}

			else
			{
				printf("Widget Count must be defined!\n");

				exit(1);
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w <= 0 || menu.h <= 0)
	{
		printf("Menu dimensions must be greater than 0\n");

		exit(1);
	}

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	SDL_SetAlpha(menu.background, SDL_SRCALPHA|SDL_RLEACCEL, 196);

	free(buffer);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;

	realignGrid();
}

Menu *initSoundMenu()
{
	menu.action = &doMenu;

	if (menu.widgets == NULL)
	{
		loadMenuLayout();
	}

	menu.returnAction = &showOptionsMenu;

	return &menu;
}

static void realignGrid()
{
	int i, maxWidth = 0;

	if (menu.widgets != NULL)
	{
		for (i=0;i<menu.widgetCount;i++)
		{
			if (menu.widgets[i]->label != NULL && menu.widgets[i]->normalState->w > maxWidth)
			{
				maxWidth = menu.widgets[i]->normalState->w;
			}
		}

		for (i=0;i<menu.widgetCount;i++)
		{
			if (menu.widgets[i]->label != NULL)
			{
				menu.widgets[i]->label->x = menu.widgets[i]->x + maxWidth + 10;
			}
		}
	}
}

void freeSoundMenu()
{
	int i;

	if (menu.widgets != NULL)
	{
		for (i=0;i<menu.widgetCount;i++)
		{
			freeWidget(menu.widgets[i]);
		}

		free(menu.widgets);
	}

	if (menu.background != NULL)
	{
		SDL_FreeSurface(menu.background);

		menu.background = NULL;
	}
}

static void toggleSound()
{
	Widget *w = menu.widgets[menu.index];

	game.audio = game.audio == TRUE ? FALSE : TRUE;

	if (game.audio == FALSE)
	{
		stopMusic();
	}

	else
	{
		if (initAudio() == TRUE)
		{
			playMusic();
		}

		else
		{
			game.audio = FALSE;
		}
	}

	updateLabelText(w->label, game.audio == TRUE ? _("Yes") : _("No"));
}

static void lowerSFXVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.sfxDefaultVolume, &game.sfxVolume, w, -1);
}

static void raiseSFXVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.sfxDefaultVolume, &game.sfxVolume, w, 1);
}

static void lowerMusicVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.musicDefaultVolume, &game.musicVolume, w, -1);
}

static void raiseMusicVolume()
{
	Widget *w = menu.widgets[menu.index];

	changeVolume(&game.musicDefaultVolume, &game.musicVolume, w, 1);
}

static void changeVolume(int *maxVolume, int *currentVolume, Widget *w, int adjustment)
{
	int align = (*maxVolume) == (*currentVolume) ? TRUE : FALSE;
	char *text;

	*maxVolume += adjustment;

	if (*maxVolume < 0)
	{
		*maxVolume = 0;
	}

	else if (*maxVolume > 10)
	{
		*maxVolume = 10;
	}

	if (align == TRUE)
	{
		*currentVolume = *maxVolume;
	}

	setMusicVolume();

	text = getVolumePercent(*maxVolume);

	updateLabelText(w->label, text);

	free(text);
}

static char *getVolumePercent(int volume)
{
	char *text;

	text = (char *)malloc(3);

	if (text == NULL)
	{
		printf("Failed to allocate a whole 3 bytes for a volume label\n");

		exit(1);
	}

	snprintf(text, 3, "%d", volume);

	return text;
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}