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

int addToInventory(Entity *);
void moveInventoryCursor(int);
void selectInventoryItem(void);
Entity *getInventoryItem(char *);
void dropInventoryItem(void);
void useInventoryItem(void);
void doInventory(void);
void drawSelectedInventoryItem(int, int, int, int);
int removeInventoryItem(char *);
void addRequiredToInventory(Entity *);
void loadInventoryItems(void);
void writeInventoryToFile(FILE *);
void getInventoryItemFromScript(char *);
void freeInventory(void);
void drawInventory(void);
void nextInventoryItem(int);
void setInventoryDialogMessage(char *, ...);