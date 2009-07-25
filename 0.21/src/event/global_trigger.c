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

#include "../entity.h"
#include "objective.h"
#include "trigger.h"
#include "global_trigger.h"
#include "../event/script.h"
#include "../hud.h"
#include "../inventory.h"

static Trigger trigger[MAX_TRIGGERS];

static void addGlobalTrigger(char *, int, int, int, char *);

void freeGlobalTriggers()
{
	/* Clear the list */

	memset(trigger, 0, sizeof(Trigger) * MAX_TRIGGERS);
}

void addGlobalTriggerFromScript(char *line)
{
	char triggerName[MAX_VALUE_LENGTH], targetName[MAX_VALUE_LENGTH], targetType[MAX_VALUE_LENGTH], count[MAX_VALUE_LENGTH];
	int currentValue;
	Entity *e;

	currentValue = 0;

	sscanf(line, "\"%[^\"]\" %s %s \"%[^\"]\"", triggerName, count, targetType, targetName);

	e = getInventoryItem(triggerName);

	if (e != NULL)
	{
		printf("Found an item in the inventory with name %s\n", triggerName);
		
		currentValue = (e->flags & STACKABLE) ? e->health : 1;
	}

	addGlobalTrigger(triggerName, currentValue, atoi(count), getTriggerTypeByName(targetType), targetName);
}

void addGlobalTriggerFromResource(char *key[], char *value[])
{
	int i, triggerName, count, targetType, targetName, total;

	total = triggerName = count = targetType = targetName = -1;

	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		if (strcmpignorecase("TRIGGER_NAME", key[i]) == 0)
		{
			triggerName = i;
		}

		else if (strcmpignorecase("TRIGGER_COUNT", key[i]) == 0)
		{
			count = i;
		}

		else if (strcmpignorecase("TRIGGER_TOTAL", key[i]) == 0)
		{
			total = i;
		}

		else if (strcmpignorecase("TRIGGER_TYPE", key[i]) == 0)
		{
			targetType = i;
		}

		else if (strcmpignorecase("TRIGGER_TARGET", key[i]) == 0)
		{
			targetName = i;
		}
	}

	if (total == -1 && count != -1)
	{
		total = count;

		count = 0;
	}

	if (triggerName == -1 || count == -1 || targetType == -1 || targetName == -1 || total == -1)
	{
		printf("Trigger is missing resources\n");

		exit(1);
	}

	addGlobalTrigger(value[triggerName], atoi(value[count]), atoi(value[total]), getTriggerTypeByName(value[targetType]), value[targetName]);
}

static void addGlobalTrigger(char *triggerName, int count, int total, int targetType, char *targetName)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == FALSE)
		{
			trigger[i].inUse = TRUE;

			trigger[i].count = count;
			trigger[i].total = total;
			trigger[i].targetType = targetType;

			STRNCPY(trigger[i].triggerName, triggerName, sizeof(trigger[i].triggerName));
			STRNCPY(trigger[i].targetName, targetName, sizeof(trigger[i].targetName));

			printf("Added Global Trigger \"%s\" with count %d\n", trigger[i].triggerName, trigger[i].total);
			
			if (count == total)
			{
				printf("Already got all the items needed for this trigger!\n");
				
				fireGlobalTrigger(triggerName);
			}

			return;
		}
	}

	printf("No free slots to add global trigger \"%s\"\n", triggerName);

	exit(1);
}

void fireGlobalTrigger(char *name)
{
	int i;
	char message[MAX_MESSAGE_LENGTH];

	if (strlen(name) == 0)
	{
		return;
	}

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE && strcmpignorecase(trigger[i].triggerName, name) == 0)
		{
			trigger[i].count++;

			if (trigger[i].targetType == UPDATE_OBJECTIVE)
			{
				snprintf(message, MAX_MESSAGE_LENGTH, _("%s (%d / %d)"), trigger[i].targetName, trigger[i].count, trigger[i].total);

				freeMessageQueue();

				setInfoBoxMessage(60, message);
			}

			printf("Updating Trigger \"%s\", %d / %d\n", trigger[i].triggerName, trigger[i].count, trigger[i].total);

			if (trigger[i].count >= trigger[i].total)
			{
				printf("Firing global trigger %s\n", trigger[i].triggerName);

				switch (trigger[i].targetType)
				{
					case UPDATE_OBJECTIVE:
						updateObjective(trigger[i].targetName);
					break;
					
					case UPDATE_TRIGGER:
						printf("Updating trigger %s\n", trigger[i].targetName);
						fireGlobalTrigger(trigger[i].targetName);
					break;

					case ACTIVATE_ENTITY:
						activateEntitiesWithRequiredName(trigger[i].targetName, TRUE);
					break;

					case RUN_SCRIPT:
						runScript(trigger[i].targetName);
					break;

					default:

					break;
				}

				trigger[i].inUse = FALSE;

				return;
			}
		}
	}
}

void updateGlobalTrigger(char *name, int value)
{
	int i;

	if (strlen(name) == 0)
	{
		return;
	}

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE && strcmpignorecase(trigger[i].triggerName, name) == 0)
		{
			printf("Modifying global trigger value from %d to %d\n", trigger[i].count, (trigger[i].count - value));
			
			trigger[i].count -= value;
		}
	}
}

void writeGlobalTriggersToFile(FILE *fp)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE GLOBAL_TRIGGER\n");
			fprintf(fp, "TRIGGER_NAME %s\n", trigger[i].triggerName);
			fprintf(fp, "TRIGGER_COUNT %d\n", trigger[i].count);
			fprintf(fp, "TRIGGER_TOTAL %d\n", trigger[i].total);
			fprintf(fp, "TRIGGER_TYPE %s\n", getTriggerTypeByID(trigger[i].targetType));
			fprintf(fp, "TRIGGER_TARGET %s\n", trigger[i].targetName);
			fprintf(fp, "}\n\n");
		}
	}
}