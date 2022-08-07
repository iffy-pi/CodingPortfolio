#include "csvparser.h"


int main( int argc, char *argv[],  char *envp[] ){

	if ( argc < 2 ){
		printf("No filename passed in as argument\n");
		exit(1);
	}
	char * filename = argv[1];

	struct csv_table *table = open_and_parse_file_to_csv_table(filename, ',', '"', FALSE, FALSE);

	print_csv_table(table);

	free_csv_table(table);

	return 0;
}