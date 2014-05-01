#define MAX_NAME_SIZE 50
#define MAX_ENTRIES 100
#define ERROR -1

typedef struct {
   int dest_Id; /* Physical Id */
   char name[MAX_NAME_SIZE]; /* Corresponding Name */
} NameEntry;

typedef struct {
   NameEntry entries[MAX_ENTRIES];
   int size;
} NameTable;

void InitNTable(NameTable * table);
int FindNTableName(NameTable * table, char name[]);
int FindNTableIndex(NameTable * table, int addr);
void AddNTable(NameTable * table, int addr, char name[]);
void UpdateNTableByAddress(NameTable * table, int addr, char name[]);
void DNSDebugTable(NameTable * table);
void UpdateNTableByIndex(NameTable * table, int index, int addr, char name[]);
