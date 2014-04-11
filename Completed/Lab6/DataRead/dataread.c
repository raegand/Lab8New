#include <stdio.h>

int main() {

	static const char filename[] = "file.txt";
	FILE *file = fopen ( filename, "r" );
	if ( file != NULL )
	{
		char line [ 128 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			printf("%c\n",line[0]);
			if (line[2] != '\0') {
				printf("%c\n",line[2]);
			}
			printf("\n");
		}
		fclose ( file );
	}
	else
	{
		perror ( filename ); /* why didn't the file open? */
	}
	return 0;
}
