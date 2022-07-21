#include "csvparser.h"

int main(){

	/* CSV Cells are the base structure of parsed CSVs, they stored the parsed string for a given csv cell */
	/* New cells can be created using new_cell function */
	struct csv_cell * c1 = new_csv_cell();

	/* You can also use new_cell_str to initialize a csv cell with a string value */
	struct csv_cell * c2 = new_csv_cell_from_str("Apple");

	/* Print the value of a CSV cell */
	printf("The value of c2 cell:\n");
	print_csv_cell(c2);
	printf("\n");

	/* Use csv_cell_equals to compare the string for allocated CSV cells, returns true if they have the same string value */
	printf("\n");
	printf("c1 == c2: ");
	if (csv_cell_equals(c1, c2)){
		printf("True");
	} else {
		printf("False");
	}
	printf("\n");

	struct csv_cell * c3 = new_csv_cell_from_str("Apple");
	printf("c2 == c3: %s\n", ( csv_cell_equals(c2, c3)) ? "True" : "False" );

	/* All CSV structures are allocated from the heap, to de-allocate them use the free function */
	free_csv_cell(c1);
	free_csv_cell(c2);
	free_csv_cell(c3);

	/* The next level of the CSV structure is the CSV row */
	/* CSV rows also have the same new function */
	struct csv_row * r1 = new_csv_row();

	/* CSV rows are made to contain CSV cells in a doubly linked list */

	/* Add a new CSV cell using the map function */
	c1 = new_csv_cell_from_str("Cell1");
	map_cell_into_csv_row(r1, c1);

	/* Map function adds a pointer to the allocated CSV cell into the row */
	/* To allocate a new cell that takes the value of the passed in cell, you can use the add_row_clone */
	


	// char * expected[] = {
	// 	"<iffy1>",
	// 	"<iffy1>,<iffy2,iffy3>",
	// 	"<iffy1>,<>,<iffy2,iffy3>",
	// 	"<iffy1>,<iffy2,iffy3>,<>,<>",
	// 	NULL
	// };

	// char * strings[] = {
	// 	"iffy1", // 0
	// 	"iffy1\n", // 0
	// 	"iffy1\r\n", // 0 
	// 	"iffy1\niffy2,iffy3", // 1
	// 	"iffy1\niffy2,iffy3\n", // 1
	// 	"iffy1\niffy2,iffy3\r\n", // 1
	// 	"iffy1\n\niffy2,iffy3\n", // 2
	// 	"iffy1\n,,\niffy2,iffy3", // 2 *
	// 	"iffy1\niffy2,iffy3,,,\n\n\n", // 3
	// 	NULL
	// };

	// // index of expected output in expected[]
	// int expectedmappings[] = {
	// 	0, 0, 0, 1, 1, 1, 2, 2, 3
	// };

	// char * expected[] = {
	// 	"<iffy1,iffy2>",
	// 	"<iffy1,,iffy2>",
	// 	"<,iffy1,>",
	// 	"<,,iffy1,,>",
	// 	"<iffy1,,>,<iffy2>",
	// 	"<iffy1,,>,<,iffy2>",
	// 	"<>",
	// 	"<,,,>",
	// 	NULL
	// };

	// char * strings[] = {
	// 	"iffy1 ,iffy2", //0
	// 	"iffy1, ,iffy2", // 1
	// 	",iffy1 , ", // 2
	// 	",iffy1 , \n", // 2
	// 	",,iffy1 , ,", //3
	// 	"iffy1,  \niffy2", //4
	// 	"iffy1,,\n,iffy2", //5
	// 	"", //6
	// 	" , , ", // 7
	// 	NULL
	// };

	// // index of expected output in expected[]
	// int expectedmappings[] = {
	// 	0, 1, 2, 2, 3, 4, 5, 6, 7
	// };

	// struct csv_table * table;
	// for(int i=0; strings[i] != NULL; i++ ){
	// 	printf("String %d: \"", i);
	// 	special_print(strings[i]);
	// 	printf("\"\n");
	// 	printf("Expected: %s\n", expected[ expectedmappings[i] ]);
	// 	table = parse_string_to_csv_table_exp(strings[i], strlen(strings[i]), ',', FALSE, TRUE);
	// 	// table = parse_string_to_csv_table(strings[i], strlen(strings[i]), ',', TRUE);
	// 	print_csv_table(table);
	// 	printf("\n");
	// }


	return 0;
}