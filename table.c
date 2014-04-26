#include <stdio.h>
#include "table.h"
#define PARENT 1
#define CHILD 0
#define BAD -1

void InitTable(Table* table) {
	table->size = 0;
}

void UpdateTable(Table* table, int valid, int dst_addr, int out_link_id) {
	int index = FindTableIndex(table, dst_addr);
	if (index == ERROR) {
		AddTable(table, valid, dst_addr, out_link_id);
	} else {
		UpdateTableByIndex(table, index, valid, out_link_id);
	}

}

void AddTable(Table* table, int valid, int dst_addr, int out_link_id) {
	TableEntry new_entry;
	new_entry.dst_addr = dst_addr;
	new_entry.out_link_id = out_link_id;
	new_entry.valid = valid;
	new_entry.parent = CHILD;
   table->entries[table->size] = new_entry;
	table->size++;
}

int FindTableIndex(Table* table, int dst_addr) {
	int i = 0;
	for (i; i < table->size; i++) {
		if (table->entries[i].dst_addr == dst_addr) {
			return i;
		}
	}
	return ERROR;
}

int GetOutLink(Table* table, int dst_addr) {
	int index = FindTableIndex(table, dst_addr);
	if (index == ERROR) {
		return ERROR;
	}

	return table->entries[index].out_link_id;
}

void UpdateTableEntry(Table* table, int dst_addr, int valid, int new_out_link) {
	int index = FindTableIndex(table, dst_addr);
	if (index == ERROR) {
		return;
	}

   table->entries[index].parent = CHILD;
	table->entries[index].out_link_id = new_out_link;
	table->entries[index].valid = valid;
}

void UpdateTableByIndex(Table* table, int index, int valid, int new_out_link) {
	if (index > table->size) {
		return;
	}

   table->entries[index].parent = CHILD;
	table->entries[index].out_link_id = new_out_link;
	table->entries[index].valid = valid;
}

int IsBad(Table * table, int dst_addr)
{
   int index = FindTableIndex(table, dst_addr);
   if (index == ERROR) {
      return 0; /* If entry is missing, it is likely a host, thus CHILD */
   }
   if(table->entries[index].parent != BAD) {
      return 0;
   } else {
      return 1;
   }

}

void UpdateParentData(Table * table, int dst_addr)
{
   int index = FindTableIndex(table, dst_addr);
   if (index == ERROR) {
      return;
   }
   table->entries[index].parent = PARENT;
}


void UpdateBad(Table * table, int dst_addr)
{
   int index = FindTableIndex(table, dst_addr);
   if (index == ERROR) {
      return;
   }
   table->entries[index].parent = BAD;
}

void UpdateChildData(Table * table, int dst_addr)
{
   int index = FindTableIndex(table, dst_addr);
   if (index == ERROR) {
      return;
   }
   table->entries[index].parent = CHILD;
}

void SwitchDebugTable(Table * table, int id)
{
   FILE * debug = fopen("DEBUG_SWITCH", "a");
   int i;
   for(i = 0; i < table->size; i++) {
      TableEntry temp = table->entries[i];
      
      /* 
		fprintf(debug,"valid: %d  |  dst_addr: %d  |  out_link: %d\n",
				temp.valid, temp.dst_addr, temp.out_link_id);
      */

//     if(temp.parent == PARENT) {
//        fprintf(debug,"SwitchID: %d - Parent: %d \n", id, temp.dst_addr);
      if(temp.parent == BAD) {
         fprintf(debug,"SwitchID: %d - Bad: %d \n", id, temp.dst_addr);
     } 
//        fprintf(debug,"SwitchID: %d - Child: %d \n", id, temp.dst_addr);
//      }
   }
   fclose(debug);
}


void DisplayTable(Table* table) {
	int i = 0;
	for (i; i < table->size; i++) {
		TableEntry temp = table->entries[i];
		printf("valid: %d  |  dst_addr: %d  |  out_link: %d\n",
				temp.valid, temp.dst_addr, temp.out_link_id);
	}
}
