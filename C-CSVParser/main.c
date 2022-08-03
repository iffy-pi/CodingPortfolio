#include "csvparser.h"


int main(){

	// struct csv_table *table = open_and_parse_file_to_csv_table("space_table.txt", ' ', FALSE, TRUE);

	// print_csv_table(table);

	// int bufflen = 200;
	// char buffer[200];

	// FILE *csv_file = fopen("tab_table.txt", "r");

	// if ( !csv_file ) {
	// 	printf("Could not open file!\n");
	// 	exit(1);
	// }

	// fgets(buffer, bufflen, csv_file);

	// int i=0;
	// while (TRUE){

	// 	if ( buffer[i] == '\t' ) printf("\\t");
	// 	else if ( buffer[i] == '\n' ) printf("\\n");
	// 	else if ( buffer[i] == '\r' ) printf("\\r");
	// 	else if ( buffer[i] == '\0' ) printf("\\0");
	// 	else printf("%c", buffer[i]);

	// 	if ( buffer[i] == '\0' ) {
	// 		printf("\n");
	// 		break;
	// 	}
	// 	i++;
	// }

	// fclose(csv_file);

	// struct csv_table *table = open_and_parse_file_to_csv_table("tab_table.txt", '\t', '"', FALSE, TRUE);

	// print_csv_table(table);

	struct csv_cell *c1 = new_csv_cell_from_str("Cell1");

	struct csv_cell *c2 = new_csv_cell_from_str("Cell2");

	struct csv_row *r1 = new_csv_row();
	map_cell_into_csv_row(r1, c1);
	map_cell_into_csv_row(r1, c2);

	struct csv_cell *c3 = new_csv_cell_from_str("Cell3");
	struct csv_cell *c4 = new_csv_cell_from_str("Cell4");

	struct csv_row *r2 = new_csv_row();
	map_cell_into_csv_row(r2, c3);
	map_cell_into_csv_row(r2, c4);

	struct csv_table* t1 = new_csv_table();
	map_row_into_csv_table(t1, r1);
	map_row_into_csv_table(t1, r2);

	struct csv_row *r3 = new_csv_row();
	add_cell_clone_to_csv_row(r3, c1);
	add_cell_clone_to_csv_row(r3, c2);
	add_str_to_csv_row(r3, "John", 4);

	struct csv_cell *c5 = new_csv_cell_from_str("Cell5");

	map_cell_to_coord_in_csv_row(r3, c5, 2);

	map_row_to_coord_in_csv_table(t1, r3, 1);

	pretty_print_csv_table(t1);

	return 0;
}