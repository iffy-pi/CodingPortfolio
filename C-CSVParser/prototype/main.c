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

	struct csv_row *r1 = new_csv_row();
	add_str_to_csv_row(r1, "a");
	add_str_to_csv_row(r1, "b");
	add_str_to_csv_row(r1, "c");
	add_str_to_csv_row(r1, "d");
	add_str_to_csv_row(r1, "e");


	print_csv_row(r1);

	struct csv_cell *c = get_cell_ptr_in_csv_row(r1, 2);

	print_csv_cell(c);


	return 0;
}