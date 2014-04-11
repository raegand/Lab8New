#include <stdio.h>
#include "table.h"

int main() {
	Table table;
	InitTable(&table);
	AddTable(&table, 1, 0, 1);
	AddTable(&table, 1, 1, 2);
	AddTable(&table, 1, 2, 3);
	DisplayTable(&table);
	printf("\n");
	int valid = 1;
	int dst_addr = 0;
	int out_link_id = 10;

	UpdateTable(&table, valid, dst_addr, out_link_id);
	DisplayTable(&table);
/*
	int index = FindTableIndex(&table, dst_addr);
	if (index == ERROR) {
		printf("inserting new entry to table\n");
		AddTable(&table, valid, dst_addr, out_link_id);
	} else {
		printf("updating entry %d\n", dst_addr);
		UpdateTableByIndex(&table, index, valid, out_link_id);
	}
*/
	printf("OutLink for dst_addr = %d is %d\n", dst_addr, GetOutLink(&table, dst_addr));

	return 0;
}
