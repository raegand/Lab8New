#ifndef TABLE_H_
#define TABLE_H_

#define TRUE 1
#define FALSE 0
#define ERROR -1
#define MAX_TABLE_ENTRIES 100

typedef struct {
	int out_link_id;
   int dst_addr; 
	int valid;
   int parent;
} TableEntry;

typedef struct {
	TableEntry entries[MAX_TABLE_ENTRIES];
	int size;
} Table;

void InitTable(Table* table);
void UpdateTable(Table* table, int valid, int dst_addr, int out_link_id);
int  GetOutLink(Table* table, int dst_addr);


int IsBad(Table * table, int dst_addr);
void AddTable(Table* table, int valid, int dst_addr, int out_link_id);
int  FindTableIndex(Table* table, int dst_addr);
void UpdateTableEntry(Table* table, int dst_addr, int valid, int new_out_link);
void UpdateTableByIndex(Table* table, int index, int valid, int new_out_link);
void DisplayTable(Table* table);
void UpdateChildData(Table * table, int dst_addr);
void UpdateBad(Table * table, int dst_addr);
void UpdateParentData(Table * table, int dst_addr);
void SwitchDebugTable(Table * table, int id);

#endif

