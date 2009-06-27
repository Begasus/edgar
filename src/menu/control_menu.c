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
#include "../audio/audio.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void showOptionsMenu(void);
static void doMenu(void);
static void redefineKey(void);
static void realignGrid(void);

void drawControlMenu()
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

		if (w->rightAction != NULL)
		{
			w->rightAction();
		}

		menuInput.left = FALSE;
		input.left = FALSE;

		playSound("sound/common/click.ogg");
	}

	else if (input.right == TRUE || menuInput.right == TRUE)
	{
		w = menu.widgets[menu.index];

		if (w->leftAction != NULL)
		{
			w->leftAction();
		}

		menuInput.right = FALSE;
		input.right = FALSE;

		playSound("sound/common/click.ogg");
	}
}

static void loadMenuLayout()
{
	char filename[MAX_LINE_LENGTH], *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i;

	i = 0;

	snprintf(filename, sizeof(filename), _("data/menu/control_menu.dat"));

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

				if (strcmpignorecase(menuID, "UP") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_UP], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_UP]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "DOWN") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_DOWN], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_DOWN]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "LEFT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_LEFT], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_LEFT]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "RIGHT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_RIGHT], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_RIGHT]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "ATTACK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_ATTACK], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_ATTACK]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "BLOCK") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_BLOCK], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_BLOCK]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "JUMP") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_JUMP], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_JUMP]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "INTERACT") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_INTERACT], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_INTERACT]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "USE") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_ACTIVATE], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_ACTIVATE]), menu.widgets[i]->x, y);
				}
				
				else if (strcmpignorecase(menuID, "PREV_ITEM") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_PREVIOUS], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_PREVIOUS]), menu.widgets[i]->x, y);
				}
				
				else if (strcmpignorecase(menuID, "NEXT_ITEM") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_NEXT], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_NEXT]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "INVENTORY") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_INVENTORY], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_INVENTORY]), menu.widgets[i]->x, y);
				}

				else if (strcmpignorecase(menuID, "PAUSE") == 0)
				{
					menu.widgets[i] = createWidget(menuName, &control.button[CONTROL_PAUSE], NULL, NULL, &redefineKey, x, y, TRUE);

					menu.widgets[i]->label = createLabel(SDL_GetKeyName(control.button[CONTROL_PAUSE]), menu.widgets[i]->x, y);
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

Menu *initControlMenu()
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

void freeControlMenu()
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

static void redefineKey()
{
	int key, oldKey;
	Widget *w = menu.widgets[menu.index];

	key = -2;

	updateLabelText(w->label, "?");

	oldKey = (*w->value);

	flushInputs();

	while (key == -2)
	{
		key = getSingleInput();

		draw();
	}

	if (key != -1)
	{
		(*w->value) = key;
	}

	else
	{
		key = oldKey;
	}

	updateLabelText(w->label, SDL_GetKeyName(key));

	realignGrid();
}

static void showOptionsMenu()
{
	game.menu = initOptionsMenu();

	game.drawMenu = &drawOptionsMenu;
}