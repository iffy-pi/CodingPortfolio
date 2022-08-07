#include "csvparser.h"

// Create CSV cell with no string value
struct csv_cell *c1 = new_csv_cell();

// Create a cell with a string value
struct csv_cell *c2 = new_csv_cell_from_str("ACell2");

// Populate c1 with string
populate_csv_cell_str(c1, "Cell1");

// Overwrite value in c2
populate_csv_cell_str(c2, "Cell2");


// Create a new row
struct csv_row *r1 = new_csv_row();

// Create a new table
struct csv_table *t1 = new_csv_table();

// Getting "Cell1"
struct csv_cell *cell0 = get_cell_ptr_in_csv_row(r1, 0);

// Getting "Cell2"
struct csv_cell *cell1 = get_cell_ptr_in_csv_row(r1, 1);


// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4"]]

// Getting "Cell1"
struct csv_cell *cell0 = get_cell_ptr_in_csv_table(t1, 0, 0);

// Getting "Cell3"
struct csv_cell *cell2 = get_cell_ptr_in_csv_table(t1, 1, 0);


struct csv_cell *search_cell = new_csv_cell_from_str("Cell1");
int index = get_cell_coord_in_csv_row(r1, search_cell);



struct csv_cell *c1 = new_csv_cell_from_str("Cell1");
struct csv_cell *c2 = new_csv_cell_from_str("Cell2");
struct csv_row *r1 = new_csv_row();

map_cell_into_csv_row(r1, c1); // r1 = ["Cell1"]
map_cell_into_csv_row(r1, c2); // r1 = ["Cell1", "Cell2"]

populate_csv_cell_str(c2, "Apple"); // c2 = "Apple"
									// r1 = ["Cell1", "Apple"]


// r1 = ["Cell1", "Cell3", "Apple"]
struct csv_cell *c4 = new_csv_cell_from_str("Cell4");

insmap_cell_into_csv_row(r1, c3, r1->length); // r1 = ["Cell1", "Cell3", "Apple", "Cell4"]

// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4"]]
struct csv_cell *c5 = new_csv_cell_from_str("Cell5");

insmap_cell_into_csv_table(t1, c5, t1->length, 0);
// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4"], ["Cell5"]]


// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4"], ["Cell5"]]
struct csv_cell *c6 = new_csv_cell_from_str("Cell6");

insmap_cell_into_csv_table(t1, c6, 1, 2);
// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4", "Cell6"], ["Cell5"]]


// r1 = ["Cell1", "Cell3", "Apple", "Cell4"]
struct csv_cell *apple_cell = pop_cell_from_csv_row(r1, 2);

// r1 = ["Cell1", "Cell3", "Cell4"]
populate_csv_cell_str(apple_cell, "Orange"); // apple_cell = "Orange"
                                             // r1 = ["Cell1", "Cell3", "Cell4"]

struct csv_cell *c1 = new_csv_cell_from_str("Apple");
struct csv_cell *c2 = new_csv_cell_from_str("Banana");
struct csv_cell *c3 = new_csv_cell_from_str("Apple");
struct csv_cell *c4 = new_csv_cell_from_str("");
struct csv_cell *c5 = new_csv_cell();
struct csv_cell *c6 = NULL;
struct csv_cell *c7 = NULL;

csv_cell_equals(c1, c3); // TRUE (1)
csv_cell_equals(c1, c2); // FALSE (0)
csv_cell_equals(c1, c4); // FALSE

csv_cell_equals(c4, c5); // FALSE
csv_cell_equals(c4, c6); // FALSE

csv_cell_equals(c5, c6); // FALSE
csv_cell_equals(c6, c7); // TRUE

// r1 = ["Cell1", "Cell3", "Cell4"]
// r2 = ["Cell1", "Cell3", "Cell4"]
// r3 = ["Cell4", "Cell3", "Cell1"]
// r4 = ["Cell1"]
// r5 = []
// r6 = NULL

csv_row_equals(r1, r2); // TRUE
csv_row_equals(r1, r3); // FALSE
csv_row_equals(r1, r4); // FALSE
csv_row_equals(r1, r5); // FALSE

csv_row_equals(r5, r6); // FALSE


// r1 = ["Cell1", "Cell3", "Cell4"]
struct csv_cell *c1 = new_csv_cell_from_str("Cell1");
struct csv_cell *c2 = new_csv_cell_from_str("Cell2");

is_cell_in_csv_row(r1, c1); // TRUE
is_cell_in_csv_row(r1, c2); // FALSE


// r1 = ["Cell1", "Cell3", "Cell4"]
// Initialization: Set cur_cell to beginning of list
// Conditional: has_next_cell with parent structure and current value iterator
// Incrementation: Set cur cell to the next cell

for( struct csv_cell *cur_cell=r1->list_head; has_next_cell(r1, cur_cell); cur_cell=cur_cell->next ){
	printf("%s\n", cur_cell->str);
}


// r1 = ["Cell1", "Cell3", "Cell4"]
struct csv_cell *cur_cell = r1->list_head;

while ( cur_cell != NULL && cur_cell->next != NULL ){

	printf("%s, ", cur_cell->str);

	cur_cell = cur_cell->next;
}

if ( r1->list_tail != NULL ){
	printf("%s\n", r1->list_tail->str);
}

int total_cells = 0;
for ( struct csv_row *cur_row=table->list_head; has_next_row(table, cur_row); cur_row=cur_row->next){
	total_cells += cur_row->length;
}

printf("Table has %d cells\n", total_cells);