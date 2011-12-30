/*
Copyright (C) 2009-2012 Parallel Realities

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

#include "../audio/audio.h"
#include "../draw.h"
#include "../game.h"
#include "../graphics/graphics.h"
#include "../init.h"
#include "../system/error.h"
#include "../system/pak.h"
#include "main_menu.h"
#include "stats_menu.h"
#include "widget.h"

extern Input input, menuInput;
extern Game game;
extern Control control;

static Menu menu;

static void loadMenuLayout(void);
static void doMenu(void);
static void showMainMenu(void);

void drawStatsMenu()
{
	int i;

	drawImage(menu.background, menu.x, menu.y, FALSE, 196);

	for (i=0;i<menu.widgetCount;i++)
	{
		drawWidget(menu.widgets[i], &menu, menu.index == i);
	}
}

static void doMenu()
{
	if (input.attack == TRUE || menuInput.attack == TRUE)
	{
		menuInput.attack = FALSE;
		input.attack = FALSE;

		playSound("sound/common/click.ogg");

		showMainMenu();
	}
}

static void loadMenuLayout()
{
	char *line, menuID[MAX_VALUE_LENGTH], menuName[MAX_VALUE_LENGTH], *token, *savePtr1, *savePtr2;
	unsigned char *buffer;
	int x, y, i, width;
	float distance;

	savePtr1 = NULL;

	i = 0;
	
	width = 0;

	buffer = loadFileFromPak("data/menu/stats_menu.dat");

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

			menu.widgetCount = atoi(token) + MAX_STATISTICS;

			menu.widgets = malloc(sizeof(Widget *) * menu.widgetCount);

			if (menu.widgets == NULL)
			{
				showErrorAndExit("Ran out of memory when creating Stats Menu");
			}
		}

		else if (strcmpignorecase(token, "WIDGET") == 0)
		{
			i = 0;
			
			menu.widgets[i] = createWidget(_("Statistics"), NULL, NULL, NULL, NULL, -1, 20 + i * 40, TRUE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			token = getPlayTimeAsString();
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Play Time: %s"), token);
			
			free(token);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Kills: %d"), game.kills);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Arrows Fired: %d"), game.arrowsFired);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Bats Drowned: %d"), game.batsDrowned);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Times Eaten: %d"), game.timesEaten);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			distance = game.distanceTravelled;
			
			distance /= 45000; /* 45 pixels is 1 metre */
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Distanced Travelled: %0.1fKM"), distance);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Attacks Blocked: %d"), game.attacksBlocked);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			token = getSlimeTimeAsString();
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Time Spent As A Slime: %s"), token);
			
			free(token);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			snprintf(menuName, MAX_VALUE_LENGTH, _("Secrets Found: %d / %d"), game.secretsFound, TOTAL_SECRETS);
			
			menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, 10, 20 + i * 40, FALSE, 255, 255, 255);
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
			
			i++;
			
			token = strtok_r(NULL, "\0", &savePtr2);

			sscanf(token, "%s \"%[^\"]\" %d %d", menuID, menuName, &x, &y);

			if (strcmpignorecase(menuID, "MENU_OK") == 0)
			{
				menu.widgets[i] = createWidget(menuName, NULL, NULL, NULL, NULL, x, 20 + i * 40, TRUE, 255, 255, 255);
			}

			else
			{
				showErrorAndExit("Unknown widget %s", menuID);
			}
			
			if (menu.widgets[i]->selectedState->w > width)
			{
				width = menu.widgets[i]->selectedState->w;
			}
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	if (menu.w <= 0 || menu.h <= 0)
	{
		showErrorAndExit("Menu dimensions must be greater than 0");
	}

	/* Resize */

	menu.w = width + 20;
	
	menu.h = 20 + i * 40 + menu.widgets[i]->selectedState->h;

	menu.background = addBorder(createSurface(menu.w, menu.h), 255, 255, 255, 0, 0, 0);

	free(buffer);

	menu.x = (SCREEN_WIDTH - menu.background->w) / 2;
	menu.y = (SCREEN_HEIGHT - menu.background->h) / 2;
}

Menu *initStatsMenu()
{
	menu.action = &doMenu;
	
	freeStatsMenu();

	loadMenuLayout();

	menu.index = MAX_STATISTICS;

	menu.returnAction = &showMainMenu;

	return &menu;
}

void freeStatsMenu()
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

static void showMainMenu()
{
	game.menu = initMainMenu();

	game.drawMenu = &drawMainMenu;
}
