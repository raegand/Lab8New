#include <string.h>
#include <stdio.h>
#include "ntable.h"

void InitNTable(NameTable * table)
{
   table->size = 0;
}

int isNTableEmpty(NameTable * table)
{
   if(table->size == 0) {
      return 1;
   } else {
      return 0;
   }
}

int FindNTableIndex(NameTable * table, int addr)
{
   int i = 0;
   for(i; i< table->size; i++) {
      if(table->entries[i].dest_Id == addr) {
         return i;
      }
   }
   return ERROR;
}

void UpdateNTableByAddress(NameTable * table, int addr, char name[])
{
   int index = FindNTableIndex(table, addr);
   if(index == ERROR) {
      return;
   }
   UpdateNTableByIndex(table, index, addr, name);
}

void UpdateNTableByIndex(NameTable * table, int index, int addr, char name[])
{
   if(index > table->size) {
      return;
   }
   table->entries[index].dest_Id = addr;
   strcpy(table->entries[index].name, name);
}

void AddNTable(NameTable * table, int addr, char name[])
{
   if(FindNTableIndex(table, addr) == ERROR) {
      NameEntry new_entry;
      new_entry.dest_Id = addr;
      strcpy(new_entry.name, name);
      table->entries[table->size] = new_entry;
      table->size++;
   } else {
      UpdateNTableByAddress(table, addr, name); 
   }
}

void DisplayNTable(NameTable* table) {
	int i = 0;
	for (i; i < table->size; i++) {
		NameEntry temp = table->entries[i];
		printf("name: %s  |  addr: %d \n", temp.name, temp.dest_Id);
	}
}
