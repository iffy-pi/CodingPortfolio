#include "csvparser.h"

/*
Allocate new memory for the node passed in
*/

int mallocstrcpy(char ** dest, char * src, int len){
	// use malloc to allocate the required number of bytes
	(*dest) = (char*) malloc( (len+1)*sizeof(char) );

	if ( (*dest) == NULL ) {
		printf("mallocstrcpy failed!\n");
		exit(1);
	}

	// use for loop to copy
	int i;
	for(i=0; i < len; i++) (*dest)[i] = src[i];
	(*dest)[i] = '\0';

	return 0;
}

char * malloc_strip_quotes_and_spaces(char  * string, int len, char quot_char, int strip_quotes, int strip_spaces, int free_string){
	// strips string of leading and trailing spaces
	// returns a pointer to the stripped string, allocated using malloc
	// option to free old string as parameter

	// quot_char is either '"' or '''

	if ( string == NULL ) return NULL;

	int new_word_start_pos = 0;
	int new_word_end_pos = len;

	if ( strip_quotes && string[new_word_start_pos] == quot_char && string[new_word_end_pos-1] == quot_char) {
		new_word_start_pos++;
		new_word_end_pos--;
	}


	// remove leading / trailing spaces
	while(strip_spaces && new_word_end_pos != new_word_start_pos && string[new_word_end_pos-1] == ' ' ) new_word_end_pos--;
	while(strip_spaces && new_word_start_pos != new_word_end_pos && string[new_word_start_pos] == ' ') new_word_start_pos++;


	int new_wordlen = new_word_end_pos - new_word_start_pos;
	char *new_string = (char *)malloc( (new_wordlen+1) * sizeof(char));

	int place_character;
	int new_string_indx = 0;
	int quot_count = 0;

	for(int old_string_indx=new_word_start_pos; old_string_indx < new_word_end_pos; old_string_indx++){
		
		// remove quotes unless escaped i.e. for '""' result is '"' and for '""""' result is '""'=
		place_character = TRUE;
		if ( string[old_string_indx] == quot_char ){
			// quots must not be placed if
			// quot appears at the beginning of the word (there is no quot behind it)
			// previous character is not a quot
			// previous character is a quot but not an even quot count, indicates it was used as escape for another quot
			quot_count = (quot_count + 1) % 2;
			if ( old_string_indx == new_word_start_pos || string[old_string_indx-1] != quot_char || string[old_string_indx-1] == quot_char && quot_count == 1 ) place_character = FALSE;
		}

		if ( place_character ){
			new_string[new_string_indx] = string[old_string_indx];
			new_string_indx++;
		}
	}

	// populate the remaining spaces with null terminator?, this may not be safe
	for( new_string_indx=new_string_indx; new_string_indx < new_wordlen; new_string_indx++) new_string[new_string_indx] = '\0';
	new_string[new_wordlen] = '\0';

	if (free_string) free(string);

	return new_string;

}

struct csv_cell * new_csv_cell(){
	struct csv_cell * cellptr = (struct csv_cell *) malloc(sizeof(struct csv_cell));
	cellptr->str = NULL;
	cellptr->parent = NULL;
	cellptr->prev = NULL;
	cellptr->next = NULL;
	return cellptr;
}

struct csv_cell * new_csv_cell_from_str(char * string){
	struct csv_cell * new_cell = new_csv_cell();
	populate_csv_cell_str(new_cell, string);
	return new_cell;
}

struct csv_row * new_csv_row(){
	struct csv_row * rowptr = (struct csv_row *) malloc( sizeof(struct csv_row) );
	rowptr->cell_count = 0;
	rowptr->cell_list_head = NULL;
	rowptr->cell_list_tail = NULL;
	rowptr->parent_table = NULL;
	rowptr->prev = NULL;
	rowptr->next = NULL;
}

struct csv_table * new_csv_table(){
	struct csv_table * tableptr = (struct csv_table *) malloc(sizeof(struct csv_table));
	tableptr->row_count = 0;
	tableptr->row_list_head  = NULL;
	tableptr->row_list_tail = NULL;

	return tableptr;
}

void free_csv_cell(struct csv_cell * cellptr){
	if ( cellptr->str != NULL ){
		free(cellptr->str);
	}

	// careful calling this function, does not free next and prev pointers!
	free(cellptr);
	cellptr=NULL;
}

void free_csv_row(struct csv_row * rowptr){
	if (rowptr == NULL ) return;

	// work our way backwards in the list, starting from the tail
	// use the element count

	struct csv_cell * cur_cell = rowptr->cell_list_head;

	while ( cur_cell != NULL && cur_cell->next != NULL ){
		// while there is a next cell
		// so we will break when we reach tail cell

		// move up the list
		cur_cell = cur_cell->next;

		if (VERBOSE){
			printf("Freeing: ");
			print_csv_cell(cur_cell->prev);
			printf("\n");
		}

		// free the previous current cell
		free_csv_cell(cur_cell->prev);
		rowptr->cell_count--;
	}

	// will break when we are at tail
	// now just have to free it
	if (rowptr->cell_list_tail != NULL ) {
		if (VERBOSE){
			printf("Freeing: ");
			print_csv_cell(rowptr->cell_list_tail);
			printf("\n");
		}

		free_csv_cell(rowptr->cell_list_tail);
		rowptr->cell_count--;
	}

	if ( rowptr->cell_count != 0) {
		printf("Missing cells somewhere!");
		exit(1);
	}

	// free the actual row structure
	free(rowptr);
	rowptr=NULL;
}

void free_csv_table(struct csv_table * tableptr){
	if ( tableptr == NULL ) return;

	struct csv_row * cur_row = tableptr->row_list_head;

	while ( cur_row != NULL && cur_row->next != NULL ){
		// while there is a next cell
		// so we will break when we reach tail cell

		// move up the list
		cur_row = cur_row->next;

		if (VERBOSE){
			printf("Freeing: ");
			print_csv_row(cur_row->prev);
		}

		// free the previous current cell
		free_csv_row(cur_row->prev);
		tableptr->row_count--;
	}

	// will break when we are at tail
	// now just have to free it
	if (tableptr->row_list_tail != NULL ) {
		if (VERBOSE){
			printf("Freeing: ");
			print_csv_row(tableptr->row_list_tail);
		}

		free_csv_row(tableptr->row_list_tail);
		tableptr->row_count--;
	}

	if ( tableptr->row_count != 0) {
		printf("Missing cells somewhere!");
		exit(1);
	}

	// free the actual row structure
	free(tableptr);
	tableptr=NULL;
}

void populate_csv_cell_str(struct csv_cell * cell, char * string){
	if ( cell->str != NULL ){
		free(cell->str);
	}

	// copy the string with mallocstr copy
	mallocstrcpy(&(cell->str), string, strlen(string));
}

void print_csv_cell_w_params(struct csv_cell *cellptr, int print_newline){
	if (cellptr ==  NULL ){
		printf("(null)");
	} else printf("\"%s\"",cellptr->str);
	

	if (print_newline) printf("\n");
}

void print_csv_row_w_params(struct csv_row * rowptr, int print_newline){

	if (rowptr == NULL ){
		printf("(null)");
		return;
	}

	struct csv_cell *cur_cell = rowptr->cell_list_head;

	printf("[");

	for(int i=0; i < rowptr->cell_count; i++){

		print_csv_cell_w_params(cur_cell, FALSE);

		if ( i != rowptr->cell_count-1)
			printf(", ");

		cur_cell = cur_cell->next;
	}

	printf("]");

	if ( print_newline ) printf("\n");
}

void print_csv_cell(struct csv_cell * cellptr ){
	print_csv_cell_w_params(cellptr, TRUE);
}

void print_csv_row(struct csv_row * rowptr){
	print_csv_row_w_params(rowptr, TRUE);
}

void print_csv_table(struct csv_table * tableptr){
	if ( tableptr == NULL ) {
		printf("(null)\n");
		return;
	}

	printf("[");

	struct csv_row *cur_row = tableptr->row_list_head;

	for(int i=0; i < tableptr->row_count; i++){

		print_csv_row_w_params(cur_row, FALSE);

		// if not the last one print comma
		if ( i != tableptr->row_count-1)
			printf(", ");

		cur_row = cur_row->next;
	}

	printf("]\n");
}

void pretty_print_csv_row(struct csv_row* rowptr){
	if (rowptr == NULL ){
		printf("(null)");
		return;
	}

	struct csv_cell *cur_cell = rowptr->cell_list_head;

	printf("[\n");

	for(int i=0; i < rowptr->cell_count; i++){

		printf("\t");
		print_csv_cell_w_params(cur_cell, FALSE);

		if ( i != rowptr->cell_count-1)
			printf(",");

		printf("\n");

		cur_cell = cur_cell->next;
	}

	printf("]\n");
}

void pretty_print_csv_table(struct csv_table *tableptr){
	if ( tableptr == NULL ) {
		printf("(null)\n");
		return;
	}

	printf("[\n");

	struct csv_row *cur_row = tableptr->row_list_head;

	for(int i=0; i < tableptr->row_count; i++){

		printf("\t");
		print_csv_row_w_params(cur_row, FALSE);

		// if not the last one print comma
		if ( i != tableptr->row_count-1)
			printf(",");

		printf("\n");

		cur_row = cur_row->next;
	}

	printf("]\n");

}

void super_pretty_print_csv_table(struct csv_table *tableptr){
	if ( tableptr == NULL ) {
		printf("(null)\n");
		return;
	}

	printf("[\n");

	struct csv_row *cur_row = tableptr->row_list_head;
	struct csv_cell *cur_cell;

	for(int i=0; i < tableptr->row_count; i++){

		printf("\t[\n");

		cur_cell = cur_row->cell_list_head;

		for(int j=0; j < cur_row->cell_count; j++){
			printf("\t\t");
			print_csv_cell_w_params(cur_cell, FALSE);

			if ( j != cur_row->cell_count-1)
				printf(",");

			printf("\n");
			cur_cell = cur_cell->next;
		}

		printf("\t]");

		// if not the last one print comma
		if ( i != tableptr->row_count-1)
			printf(",\n");

		printf("\n");

		cur_row = cur_row->next;
	}

	printf("]\n");
}


struct csv_cell * clone_csv_cell(struct csv_cell * cell){
	if ( cell == NULL ) return NULL;

	// allocate the new cell
	struct csv_cell * new_cell = new_csv_cell();

	// use mallocstrcpy to copy the data
	mallocstrcpy(&(new_cell->str), cell->str, strlen(cell->str));

	return new_cell;
}

struct csv_row * clone_csv_row(struct csv_row * row){
	if ( row == NULL ) return NULL;

	// allocate the new row
	struct csv_row * new_row = new_csv_row();

	// copy each cell for the row
	for( struct csv_cell * cur_cell = row->cell_list_head; has_next_cell(row, cur_cell); cur_cell=cur_cell->next){
		add_str_to_csv_row(new_row, cur_cell->str);
	}

	return new_row;
}


struct csv_table * clone_csv_table(struct csv_table * table){
	if (table == NULL ) return NULL;

	struct csv_table * new_table = new_csv_table();
	for( struct csv_row * cur_row=table->row_list_head; has_next_row(table, cur_row); cur_row=cur_row->next){
		add_row_clone_to_csv_table(table, cur_row);
	}

	return new_table;
}

int csv_cell_equals(struct csv_cell * cell1, struct csv_cell * cell2){

	if ( cell1 == NULL && cell2 == NULL ) return TRUE; // they are both null
	else if ( cell1 == NULL || cell2 == NULL ) return FALSE; // one of them is null and the other isnt

	if ( cell1->str == NULL && cell2->str == NULL ) return TRUE;
	else if ( cell1->str == NULL || cell2->str == NULL) return FALSE;

	// compares the values of the cells
	return ( strcmp(cell1->str, cell2->str) == 0 );
}


int csv_row_equals(struct csv_row * row1, struct csv_row * row2){
	if ( row1 == NULL && row2 == NULL ) return TRUE;
	else if ( row1 == NULL || row2 == NULL) return FALSE;

	//check the row count
	if ( row1->cell_count != row2->cell_count ) return FALSE;

	int different = FALSE;

	struct csv_cell *cur_cell1, *cur_cell2;

	// compare each individual cell, break if there is a difference found
	for ( int i=0; i < row1->cell_count && !different; i++ ){
		cur_cell1 = get_cell_ptr_in_csv_row( row1, i);
		cur_cell2 = get_cell_ptr_in_csv_row( row2, i);

		if ( !csv_cell_equals(cur_cell1, cur_cell2) ) different = TRUE;
	}

	return !different;
}


int csv_table_equals(struct csv_table * table1, struct csv_table * table2){
	if ( table1 == NULL && table2 == NULL ) return TRUE;
	else if ( table1 == NULL || table2 == NULL) return FALSE;

	//check the row count
	if ( table1->row_count != table2->row_count ) return FALSE;

	int different = FALSE;

	struct csv_row *cur_row1, *cur_row2;

	// compare each individual cell, break if there is a difference found
	for ( int i=0; i < table1->row_count && !different; i++ ){
		cur_row1 = get_row_ptr_in_csv_table( table1, i);
		cur_row2 = get_row_ptr_in_csv_table( table2, i);

		if ( !csv_row_equals(cur_row1, cur_row2) ) different = TRUE;
	}

	return !different;
}

struct csv_cell * get_cell_ptr_in_csv_row(struct csv_row * row, int index){
	if( row == NULL || row->cell_count == 0 || index >= row->cell_count || index < 0 ) return NULL;

	struct csv_cell * cur_cell = row->cell_list_head;

	for(int i=0; i < index; i++) cur_cell = cur_cell->next;

	return cur_cell;

}

struct csv_row * get_row_ptr_in_csv_table(struct csv_table * table, int index){

	if( table == NULL || table->row_count == 0 || index >= table->row_count || index < 0 ) return NULL;

	struct csv_row * cur_row = table->row_list_head;

	for(int i=0; i < index; i++) cur_row = cur_row->next;

	return cur_row;

}

struct csv_cell * get_cell_ptr_in_csv_table(struct csv_table * table, int rowindx, int colindx){
	if( table == NULL ) return NULL;

	// get the relevant row
	struct csv_row * rel_row = get_row_ptr_in_csv_table(table, rowindx);

	if ( rel_row == NULL ) return NULL;

	return get_cell_ptr_in_csv_row(rel_row, colindx);

}

struct csv_cell * get_cell_clone_in_csv_row(struct csv_row * row, int index){
	struct csv_cell * ref = get_cell_ptr_in_csv_row(row, index);

	if ( ref != NULL ) return clone_csv_cell(ref);
	return NULL;
}

struct csv_row * get_row_clone_in_csv_table(struct csv_table * table, int index){
	struct csv_row * ref = get_row_ptr_in_csv_table(table, index);

	if ( ref != NULL ) return clone_csv_row(ref);
	return NULL;
}

struct csv_cell * get_cell_clone_in_csv_table(struct csv_table * table, int rowindx, int colindx){
	struct csv_cell * ref = get_cell_ptr_in_csv_table(table, rowindx, colindx);

	if ( ref != NULL ) return clone_csv_cell(ref);
	return NULL;
}

struct csv_cell * get_cell_for_str_in_csv_row(struct csv_row * row, char * string){
	if ( row == NULL || row->cell_count == 0 ) return NULL;

	int found_match = FALSE;

	struct csv_cell * cur_cell;
	for( cur_cell=row->cell_list_head; has_next_cell(row, cur_cell); cur_cell = cur_cell->next ){

		// check if the curecell is a match
		if ( strcmp(cur_cell->str, string) == 0) {
			found_match = TRUE;
			break;
		}
	}

	return (found_match) ? cur_cell : NULL;
}


struct csv_cell * get_cell_for_str_in_csv_table(struct csv_table * table, char * string){
	if ( table == NULL || table->row_count == 0)  return NULL;

	int found_match = FALSE;
	struct csv_cell * rel_cell;

	for( struct csv_row * cur_row = table->row_list_head; has_next_row(table, cur_row); cur_row = cur_row->next ){
		rel_cell = get_cell_for_str_in_csv_row( cur_row, string );

		if ( rel_cell != NULL ){
			found_match = TRUE;
			break;
		}
	}

	return (found_match) ? rel_cell : NULL;
}

int get_cell_coord_in_csv_row(struct csv_row * row, struct csv_cell * cell){
	if ( cell == NULL || row == NULL || row->cell_count == 0 ) return -1;

	int indx;
	struct csv_cell * cur_cell = row->cell_list_head;

	int found_match = FALSE;

	for( indx=0; indx < row->cell_count; indx++ ){

		// if they are the same then the index is the match
		found_match = csv_cell_equals(cur_cell, cell);

		if (found_match) break;

		cur_cell = cur_cell->next;
	}

	// went through the list and found no match
	if ( indx == row->cell_count ) return -1;

	return indx;
}

int get_row_coord_in_csv_table(struct csv_table * table, struct csv_row * row){
	if ( row == NULL || table == NULL || table->row_count == 0 ) return -1;

	int indx;
	struct csv_row * cur_row = table->row_list_head;

	int found_match = FALSE;
	for( indx=0; indx < table->row_count; indx++){
		found_match = csv_row_equals(cur_row, row);
		if (found_match) break;
		cur_row = cur_row->next;
	}

	if ( indx == table->row_count ) return -1;

	return indx;
}

int get_cell_coord_in_csv_table(struct csv_table * table, struct csv_cell * cell, int * rowindx, int * colindx){
	if ( cell == NULL || table == NULL || table->row_count == 0 ) return -1;

	int cur_row_indx, cur_column_indx;
	struct csv_row * cur_row = table->row_list_head;
	int found_match = FALSE;

	for( cur_row_indx=0; cur_row_indx < table->row_count; cur_row_indx++){
		cur_column_indx = get_cell_coord_in_csv_row( cur_row, cell);

		found_match = ( cur_column_indx != -1);

		if (found_match) break;

		cur_row = cur_row->next;
	}

	// if we reached the end of the table or there were no matches found
	if ( cur_row_indx == table->row_count  ) return -1;

	// populate the pointers
	*rowindx = cur_row_indx;
	*colindx = cur_column_indx;

	return 0;
}

int get_str_coord_in_csv_row(struct csv_row * row, char * string){
	struct csv_cell * c = get_cell_for_str_in_csv_row(row, string);

	return get_cell_coord_in_csv_row(row, c);
}

int get_str_coord_in_csv_table(struct csv_table * table, char * string, int * rowindx, int * colindx){
	struct csv_cell * c = get_cell_for_str_in_csv_table(table, string);

	return get_cell_coord_in_csv_table(table, c, rowindx, colindx);
}

int is_cell_mapped_to_csv_row(struct csv_row * row, struct csv_cell * cellptr){
	// checks if the specified cell is in the csv row

	if ( row == NULL || cellptr == NULL || row->cell_count == 0 ) return FALSE;

	int found_match = FALSE;

	// iterate through the cells to check if the curcell matches
	for( struct csv_cell * cur_cell=row->cell_list_head; has_next_cell(row, cur_cell) && !found_match; cur_cell=cur_cell->next){
		found_match = ( cur_cell == cellptr );
	}

	return found_match;
}


int is_row_mapped_to_csv_table(struct csv_table * table, struct csv_row * rowptr){
	if ( table == NULL || rowptr == NULL || table->row_count == 0 ) return FALSE;

	int found_match = FALSE;

	for(struct csv_row * cur_row=table->row_list_head; has_next_row(table, cur_row) && !found_match; cur_row=cur_row->next){
		found_match = ( cur_row == rowptr);
	}

	return found_match;
}

int is_cell_mapped_to_csv_table(struct csv_table * table, struct csv_cell * cellptr){
	if ( table == NULL || cellptr == NULL || table->row_count == 0 ) return FALSE;

	int found_match = FALSE;

	struct csv_row * cur_row;

	for(cur_row=table->row_list_head; has_next_row(table, cur_row) && !found_match; cur_row=cur_row->next){
		found_match = is_cell_mapped_to_csv_row( cur_row, cellptr );
	}

	return found_match;
}

int is_cell_in_csv_row(struct csv_row * row, struct csv_cell * cell){
	// checks cell value and returns true if cell is in csv row
	return ( get_cell_coord_in_csv_row(row, cell) != -1 );
}

int is_row_in_csv_table(struct csv_table * table, struct csv_row * row){
	return ( get_row_coord_in_csv_table(table, row) != -1 );
}

int is_cell_in_csv_table(struct csv_table * table, struct csv_cell * cell){
	int r, c;
	return ( get_cell_coord_in_csv_table(table, cell, &r, &c) != -1 );
}

int is_string_in_csv_row(struct csv_row * row, char * string){
	return ( get_cell_for_str_in_csv_row(row, string) != NULL );
}

int is_string_in_csv_table(struct csv_table * table, char * string){
	return ( get_cell_for_str_in_csv_table(table, string) != NULL );
}

void map_cell_into_csv_row(struct csv_row * rowptr, struct csv_cell * cellptr){
	// populate parent in the cell
	cellptr->parent = rowptr;

	// add the element to the list
	if ( rowptr->cell_list_head == NULL ){
		// this is the first item in the list
		rowptr->cell_list_head = cellptr;

		// also assign the tail
		rowptr->cell_list_tail = cellptr;

	} else {
		// not the first item, append to the end of the list
		// and then make it the new tail

		rowptr->cell_list_tail->next = cellptr;
		cellptr->prev = rowptr->cell_list_tail;
		rowptr->cell_list_tail = cellptr;

	}

	// incremenet elementcount
	rowptr->cell_count++;
}

void map_row_into_csv_table(struct csv_table * tableptr, struct csv_row * rowptr){

	// populate parent info
	rowptr->parent_table = tableptr;

	// add the row to the list
	if ( tableptr->row_list_head == NULL ){
		// this is the first item in the list
		tableptr->row_list_head = rowptr;

		// also assign the tail
		tableptr->row_list_tail = rowptr;

	} else {
		// not the first item, append to the end of the list
		// and then make it the new tail

		tableptr->row_list_tail->next = rowptr;
		rowptr->prev = tableptr->row_list_tail;
		tableptr->row_list_tail = rowptr;

	}

	// incremenet elementcount
	tableptr->row_count++;
}

int map_cell_to_coord_in_csv_row(struct csv_row *row, struct csv_cell *new_cell, int index){
	if ( index < 0 || row == NULL || new_cell == NULL ) return -2;

	// first we get the cell at that specific index
	struct csv_cell *ptr_cell = get_cell_ptr_in_csv_row(row, index);

	if ( ptr_cell == NULL ){
		if ( row->cell_count == index){
			// just appending to the end no problem
			map_cell_into_csv_row( row, new_cell);
			return 0;

		} else return -1;
	}

	// ptr_cell is where our new cell will be
	struct csv_cell *ptr_prev = ptr_cell->prev;
	struct csv_cell *ptr_next = ptr_cell->next;

	if ( row->cell_count == 1){
		// there is only one item, ptr cell is both head and tail
		// current cell pushes ptr cell one up therefore

		ptr_cell->prev = new_cell;
		new_cell->next = ptr_cell;
		new_cell->prev = NULL;

		// new cell becomes head and pointer cell becomes tail
		row->cell_list_head = new_cell;
		row->cell_list_tail = ptr_cell;
	
	} else if ( ptr_cell == row->cell_list_head ){
		// new cell becomes new head
		ptr_cell->prev = new_cell;
		new_cell->next = ptr_cell;
		row->cell_list_head = new_cell;
	
	} else {
		ptr_prev->next = new_cell;
		ptr_cell->prev = new_cell;
		new_cell->prev = ptr_prev;
		new_cell->next = ptr_cell;
	}

	new_cell->parent = row;
	row->cell_count++;

	return 0;

}

int map_row_to_coord_in_csv_table(struct csv_table *table, struct csv_row *new_row, int index){
	if ( index < 0 || table == NULL || new_row == NULL ) return -2;

	// first we get the cell at that specific index
	struct csv_row *ptr_row = get_row_ptr_in_csv_table(table, index);

	if ( ptr_row == NULL ){
		if ( table->row_count == index){
			// just appending to the end no problem
			map_row_into_csv_table( table, new_row);
			return 0;

		} else return -1;
	}

	// ptr_row is where our new row will be
	struct csv_row *ptr_prev = ptr_row->prev;
	struct csv_row *ptr_next = ptr_row->next;

	if ( table->row_count == 1){
		// there is only one item, ptr row is both head and tail
		// current row pushes ptr row one up therefore

		ptr_row->prev = new_row;
		new_row->next = ptr_row;
		new_row->prev = NULL;

		// new row becomes head and pointer row becomes tail
		table->row_list_head = new_row;
		table->row_list_tail = ptr_row;
	
	} else if ( ptr_row == table->row_list_head ){
		// new row becomes new head
		ptr_row->prev = new_row;
		new_row->next = ptr_row;
		table->row_list_head = new_row;
	
	} else {
		ptr_prev->next = new_row;
		ptr_row->prev = new_row;
		new_row->prev = ptr_prev;
		new_row->next = ptr_row;
	}

	new_row->parent_table = table;
	table->row_count++;

	return 0;

}

int map_cell_to_coord_in_csv_table(struct csv_table *table, struct csv_cell *new_cell, int rowindx, int colindx){
	// in this case, the specified row must exist
	if ( rowindx < 0 || colindx < 0 || table == NULL || new_cell == NULL ) return -2;

	// get the row at the specified index
	struct csv_row *ref_row = get_row_ptr_in_csv_table(table, rowindx);

	if ( ref_row == NULL ){
		if ( table->row_count == rowindx && colindx == 0 ){
			// there is no row for it so we create a new row, in this case colindx must be 0
			ref_row = new_csv_row();
			map_row_into_csv_table(table, ref_row);

		} else return -1;
	}

	// add the cell to the relevant location in the ref row
	return map_cell_to_coord_in_csv_row(ref_row, new_cell, colindx);

}

void add_cell_clone_to_csv_row(struct csv_row * rowptr, struct csv_cell * cellptr){
	struct csv_cell * new_cell = clone_csv_cell(cellptr);
	map_cell_into_csv_row(rowptr, new_cell);
}

void add_row_clone_to_csv_table(struct csv_table * tableptr, struct csv_row * rowptr){
	struct csv_row * new_row = clone_csv_row(rowptr);
	map_row_into_csv_table(tableptr, rowptr);
}

int add_cell_clone_at_coord_to_csv_row(struct csv_row * rowptr, struct csv_cell * cellptr, int index){
	struct csv_cell *new_cell = clone_csv_cell(cellptr);
	return map_cell_to_coord_in_csv_row(rowptr, new_cell, index);
}

int add_row_clone_at_coord_to_csv_table(struct csv_table * tableptr, struct csv_row * rowptr, int index){
	struct csv_row *new_row = clone_csv_row(rowptr);
	return map_row_to_coord_in_csv_table(tableptr, new_row, index);
}

int add_cell_clone_at_coord_to_csv_table(struct csv_table *tableptr, struct csv_cell * cellptr, int rowindx, int colindx){
	struct csv_cell *new_cell = clone_csv_cell(cellptr);
	return map_cell_to_coord_in_csv_table(tableptr, new_cell, rowindx, colindx);
}

int add_char_array_to_csv_row(struct csv_row * rowptr, char arr[], int arrlen){
	if ( arr[arrlen-1] != '\0' ){
		printf("Char array does not end with NULL character!\n");
		return -1;	
	}

	// create a new element structure
	struct csv_cell *cellptr = new_csv_cell();

	// use mallocstrcpy to copy the string
	// -1 because malloc is for string words, automatically adds null terminator at the end
	mallocstrcpy( &(cellptr->str), arr, arrlen-1);

	map_cell_into_csv_row(rowptr, cellptr);
	
	return 0;
}

int add_str_to_csv_row(struct csv_row *rowptr, char * string){
	return add_char_array_to_csv_row(rowptr, string, strlen(string)+1);
}

void unmap_cell_in_csv_row(struct csv_row * row, struct csv_cell *  cellptr){
	if (cellptr == NULL ) return;

	// assuming cellptr is part of csv row

	struct csv_cell * next_cell = cellptr->next;
	struct csv_cell * prev_cell = cellptr->prev;

	if ( next_cell == NULL && prev_cell == NULL ){
		// this is the one and only item in the list, so both head and tail
		if ( cellptr != row->cell_list_head && cellptr != row->cell_list_tail ) {
			printf("Invalid cell pointer!\n");
			exit(1);
		}

		// to unmap just set the head and tail to NULL
		row->cell_list_head = row->cell_list_tail = NULL;
	
	} else if ( next_cell == NULL ){
		// there is prev but no next, must be tail
		if ( cellptr != row->cell_list_tail ) {
			printf("Invalid cell pointer!\n");
			exit(1);
		}

		// in this case just cut it off from the list by setting tail to prev
		row->cell_list_tail = prev_cell;
		row->cell_list_tail->next = NULL;
	
	} else if ( prev_cell == NULL ) {
		// there is next cell but no prev, must be head
		if ( cellptr != row->cell_list_head ) {
			printf("Invalid cell pointer!\n");
			exit(1);
		}

		// in this case we move the head up one
		row->cell_list_head = next_cell;
		row->cell_list_head->prev = NULL;
	
	} else {
		// both prev and next are not null, so we just skip it over
		prev_cell->next = next_cell;
		next_cell->prev = prev_cell;
	}

	// remove any pointers
	cellptr->parent = NULL;
	cellptr->next = NULL;
	cellptr->prev = NULL;

	//decrement element count
	row->cell_count--;
}

void unmap_row_in_csv_table(struct csv_table * table, struct csv_row *  rowptr){
	if (rowptr == NULL ) return;

	// assuming rowptr is part of csv table

	struct csv_row * next_row = rowptr->next;
	struct csv_row * prev_row = rowptr->prev;

	if ( next_row == NULL && prev_row == NULL ){
		// this is the one and only item in the list, so both head and tail
		if ( rowptr != table->row_list_head && rowptr != table->row_list_tail ) {
			printf("Invalid row pointer!\n");
			exit(1);
		}

		// to unmap just set the head and tail to NULL
		table->row_list_head = table->row_list_tail = NULL;
	
	} else if ( next_row == NULL ){
		// there is prev but no next, must be tail
		if ( rowptr != table->row_list_tail ) {
			printf("Invalid row pointer!\n");
			exit(1);
		}

		// in this case just cut it off from the list by setting tail to prev
		table->row_list_tail = prev_row;
		table->row_list_tail->next = NULL;
	
	} else if ( prev_row == NULL ) {
		// there is next cell but no prev, must be head
		if ( rowptr != table->row_list_head ) {
			printf("Invalid row pointer!\n");
			exit(1);
		}

		// in this case we move the head up one
		table->row_list_head = next_row;
		table->row_list_head->prev = NULL;
	
	} else {
		// both prev and next are not null, so we just skip it over
		prev_row->next = next_row;
		next_row->prev = prev_row;
	}

	// remove any pointers
	rowptr->parent_table = NULL;
	rowptr->next = NULL;
	rowptr->prev = NULL;

	//decrement element count
	table->row_count--;
}

void unmap_cell_in_csv_table(struct csv_table * table, struct csv_cell * cellptr){
	struct csv_row * rel_row = cellptr->parent;

	// makes sure the cell is actually in the table
	if ( rel_row == NULL || rel_row->parent_table != table ) return;

	unmap_cell_in_csv_row( rel_row, cellptr);
}

struct csv_cell * pop_cell_from_csv_row(struct csv_row * row, int index){
	// get the relevant cell
	struct csv_cell * rel_cell = get_cell_ptr_in_csv_row(row, index);

	if ( rel_cell == NULL ) return NULL;

	//unmap the cell from the row
	unmap_cell_in_csv_row(row, rel_cell);

	//return the pointer to it
	return rel_cell;
}

struct csv_row * pop_row_from_csv_table(struct csv_table * table, int index){
	// get the relevant cell
	struct csv_row * rel_row = get_row_ptr_in_csv_table(table, index);

	if ( rel_row == NULL ) return NULL;

	//unmap the cell from the row
	unmap_row_in_csv_table(table, rel_row);

	//return the pointer to it
	return rel_row;
}

struct csv_cell * pop_cell_from_csv_table(struct csv_table * table, int rowindx, int colindx){
	struct csv_cell * rel_cell = get_cell_ptr_in_csv_table(table, rowindx, colindx);

	if ( rel_cell == NULL ) return NULL;

	//unmap the cell from the table
	unmap_cell_in_csv_table(table, rel_cell);

	//return the pointer to it
	return rel_cell;
}

int delete_cell_from_csv_row(struct csv_row * row, int index){
	struct csv_cell * rel_cell = pop_cell_from_csv_row(row, index);

	if ( rel_cell == NULL ) return 1;

	free_csv_cell(rel_cell);
	return 0;
}

int delete_row_from_csv_table(struct csv_table * table, int index){
	struct csv_row * rel_row = pop_row_from_csv_table(table, index);

	if ( rel_row == NULL ) return 1;

	free_csv_row(rel_row);
	return 0;
}

int delete_cell_from_csv_table(struct csv_table * table, int rowindx, int colindx){
	struct csv_cell * rel_cell = pop_cell_from_csv_table(table, rowindx, colindx);

	if ( rel_cell == NULL ) return 1;

	free_csv_cell(rel_cell);
	return 0;
}

int has_next_cell(struct csv_row * row, struct csv_cell * cur_cell){
	if ( cur_cell != NULL && cur_cell->parent != row ) {
		printf("Cell is not a child of the specified row!!\n");
		exit(1);
	}
	return  (cur_cell != NULL) && ((cur_cell->next != NULL) || (cur_cell->next == NULL && cur_cell==row->cell_list_tail));
}

int has_next_row(struct csv_table * table, struct csv_row * cur_row){
	// there is a next row if
	// the current row is not NULL (would be for reloop on tail)
	// and 
	// the next pointer is not NULL or the next pointer is null and we are at the tail)
	if ( cur_row != NULL && cur_row->parent_table != table ){
		printf("Row is not a child of the specified table!!\n");
		exit(1);
	}
	return  (cur_row != NULL) && ((cur_row->next != NULL) || (cur_row->next == NULL && cur_row==table->row_list_tail));
}


struct csv_table * parse_fileptr_or_char_array_to_csv_table( FILE * csv_file, char arr[], int arrlen, char delim, char quot_char, int strip_spaces, int discard_empty_cells, int verbose){
	int parsing_string = ( arr != NULL ) && ( arrlen > 0);
	int parsing_file = ( csv_file != NULL);

	if ( parsing_file == parsing_string )
		return NULL;

	if ( delim == ' '){
		strip_spaces = FALSE;
		discard_empty_cells = TRUE;
	}

	// buffer for fgets
	char * buffer;
	int bufflen;

	if ( parsing_string) {
		buffer = arr;
		bufflen = arrlen;
	} else {
		// parsing file
		bufflen = 100;
		buffer = (char *) malloc(bufflen * sizeof(char));
	}


	char second_last_char;

	int buffer_iterations = 0;
	int error_occured = FALSE;

	struct csv_table *table = new_csv_table();
	struct csv_row *cur_row = NULL;

	struct csv_cell *cur_cell, *last_cell, *replacement_cell;
	int last_word_len, combined_str_len, cur_cell_str_len, merging_cells;

	int cur_pos, cur_word_start_pos, cur_word_end_pos, cur_delim_pos;
	char * cur_word;
	int cur_word_len;

	int has_reached_delim, has_reached_space, has_reached_newl, has_reached_crnewl, has_reached_only_cr;
	int is_at_a_delim, has_reached_eos, has_reached_eof, eos_char_offset;

	int quot_count = 0;

	int within_quotes = 0;

	int first_word_for_buffer_parsed;

	int entire_cur_line_in_buffer;

	int append_this_cell;

	// if parsing file, we call fgets till we have read entire file
	// if parsing string, we know string is entirely in memory so no need for other work

	while ( (parsing_file && !feof(csv_file)) || ( parsing_string && buffer_iterations < 1) ){

		if ( parsing_file ){
			memset(buffer, 0, bufflen*sizeof(char));

			// reads characters into buffer
			// stops when it reaches newline, end of file or read bufflen-1 characters
			// places null terminator at position it stopped at
			// source: https://www.ibm.com/docs/en/i/7.4?topic=functions-fgets-read-string
			fgets(buffer, bufflen, csv_file);

			if ( ferror(csv_file) ){
				error_occured = TRUE;
				break;
			}
		}

		if ( buffer[0] != '\0' ){

			if (verbose) printf("---------------------------------->\n");

			if ( parsing_file ) {
				// read an entire line
				// last char is null and second last char is null (entire line read and buffer not full)
				// or
				// last char is null and second last char is newline (entire line is read and buffer full)
				// or
				// we have reached eof (no more characters to read from the file, so this line is an entire line)
				// for lines that are bigger than the buffer, none of these will be true

				second_last_char = buffer[bufflen-2];

				// last char will always be null terminator either because of memset or fgets
				// curline_finished used to store if the current line was read entirely

				entire_cur_line_in_buffer = ( second_last_char == '\0' || second_last_char == '\n' || feof(csv_file));

			} else entire_cur_line_in_buffer = TRUE;

			// parse the buffer character by character
			cur_pos = 0;
			cur_word_start_pos = 0;
			first_word_for_buffer_parsed = FALSE;

			while ( TRUE ){
				has_reached_delim = (buffer[cur_pos] == delim);

				has_reached_crnewl =  ( buffer[cur_pos] == '\r' && buffer[cur_pos+1] == '\n');
				has_reached_newl = ( buffer[cur_pos] == '\n' );
				has_reached_only_cr = ( buffer[cur_pos] == '\r' && !has_reached_crnewl );

				has_reached_eos = ( buffer[cur_pos] == '\0' || cur_pos == bufflen-1);
				has_reached_eof = ( parsing_file && has_reached_eos && feof(csv_file) );

				if ( buffer[cur_pos] == quot_char) within_quotes = (within_quotes+ 1) % 2;

				is_at_a_delim = (!within_quotes && (has_reached_delim || has_reached_newl || has_reached_only_cr || has_reached_crnewl)) || has_reached_eos;

				if ( is_at_a_delim ){
					cur_delim_pos = cur_pos;
					cur_word_end_pos = cur_delim_pos;

					cur_word_len = cur_word_end_pos - cur_word_start_pos;

					cur_word = buffer + cur_word_start_pos;

					if (verbose){
						printf("$cur_word = \"");
						int q = 0;
						while ( q < cur_word_len-1 ){
							printf("%c", cur_word[q]);
							q++;
						}
						if (cur_word_len == 0) printf("\"\n");
						else printf("%c\"\n", cur_word[q]);
					}

					merging_cells = FALSE;
					if ( cur_row == NULL ){
						// we made a complete line sometime before, reset it here
						cur_row = new_csv_row();

					} else if ( cur_row->cell_count > 0 && !first_word_for_buffer_parsed ){
						// if this is false that means this is the first word,
						// we need to combine it with the last word in the cur row
						last_cell = get_cell_ptr_in_csv_row(cur_row, cur_row->cell_count-1);
						last_word_len = strlen(last_cell->str);

						if (verbose) printf("Merging: \"%s\" + $cur_word\n", last_cell->str);

						combined_str_len = last_word_len + cur_word_len;
						replacement_cell = new_csv_cell();
						replacement_cell->str = (char *) malloc( ( combined_str_len + 1)*sizeof(char));

						for(int i=0; i < combined_str_len; i++){
							if ( i < last_word_len ){
								replacement_cell->str[i] = last_cell->str[i];
							} else {
								replacement_cell->str[i] = cur_word[ i - last_word_len ];
							}
						}
						replacement_cell->str[combined_str_len] = '\0';

						// remove the last cell, the replacement cell will be used
						if (verbose) printf("Removed cell \"%s\"\n", last_cell->str);
						unmap_cell_in_csv_row(cur_row, last_cell);
						free_csv_cell(last_cell);

						merging_cells = TRUE;
					}

					// create a new cell for the word and add it to the current row
					// allocate a new cell for it
					// if merging cells, replacement cell is already the current cell
					if  ( merging_cells ) {
						cur_cell = replacement_cell;
						cur_cell_str_len = combined_str_len;
					} else {
						cur_cell = new_csv_cell();
					}

					append_this_cell = TRUE;

					int line_is_complete = ( !within_quotes && entire_cur_line_in_buffer) || feof(csv_file);

					// if last word and line is not complete, then just merge do not apply stripping until we have full word
					if ( parsing_file && buffer[cur_word_end_pos] == '\0' && !line_is_complete ){
						if ( !merging_cells ) mallocstrcpy( &cur_cell->str, cur_word, cur_word_len);
					} else {
						if (verbose) printf("Stripping quotes and spaces\n");

						if ( merging_cells){
							cur_cell->str = malloc_strip_quotes_and_spaces(cur_cell->str, combined_str_len, quot_char, TRUE, strip_spaces, TRUE);
						} else{
							cur_cell->str = malloc_strip_quotes_and_spaces(cur_word, cur_word_len, quot_char, TRUE, strip_spaces, FALSE);
						}

						append_this_cell = !discard_empty_cells || (discard_empty_cells && strlen(cur_cell->str) > 0);
					}

					if ( append_this_cell ){
						if ( verbose ){
							if ( merging_cells) printf("Appending merged cell: \"%s\"\n", cur_cell->str);
							else printf("Appending cell: \"%s\"\n", cur_cell->str);
						}

						map_cell_into_csv_row(cur_row, cur_cell);
					} else {
						if (verbose) printf("Cell \"%s\" discarded!\n", cur_cell->str);
					}

					first_word_for_buffer_parsed = TRUE;

					// next word starts after this word ends
					cur_word_start_pos = cur_delim_pos+1;
					if (has_reached_crnewl) cur_word_start_pos++;

				}

				// it has reached the end of the line
				if ( (!within_quotes && (has_reached_newl || has_reached_only_cr || has_reached_crnewl)) || has_reached_eof || (parsing_string && has_reached_eos) ){
					if ( verbose ){
						printf("===============================\n");
						printf("Final Row:\n");
						print_csv_row(cur_row);
						printf("===============================\n");
					}

					
					if( cur_row != NULL ) map_row_into_csv_table(table, cur_row);

					// reset quotes and rows for next line
					cur_row = NULL;
					within_quotes = 0;

					if ( has_reached_eos || has_reached_eof ) break;

					// If newl or crnewl, there could be more lines
					// if there are no more lines, eof char offset will point to the terminator
					eos_char_offset = 0;
					if ( has_reached_crnewl ) eos_char_offset+=2;
					else if ( has_reached_newl || has_reached_only_cr ) eos_char_offset+=1;

					if ( buffer[cur_pos+eos_char_offset] == '\0' ) break;
				}


				if ( buffer[cur_pos] == '\0' || cur_pos >= bufflen ) break;

				if ( has_reached_crnewl ) cur_pos++;
				cur_pos++;
			}

			if (verbose) printf("----------------------------------->\n");
		}

		buffer_iterations++;
	}

	if ( error_occured ){
		printf("An error occured!\n");
		free_csv_table(table);
		return NULL;
	}

	return table;
}

struct csv_table * parse_char_array_to_csv_table(char arr[], int arrlen, char delim, char quot_char, int strip_spaces, int discard_empty_cells){
	return parse_fileptr_or_char_array_to_csv_table(NULL, arr, arrlen, delim, quot_char, strip_spaces, discard_empty_cells, FALSE);
}

struct csv_table * parse_string_to_csv_table(char * string, char delim, char quot_char, int strip_spaces, int discard_empty_cells){
	return parse_char_array_to_csv_table(string, strlen(string)+1, delim, quot_char, strip_spaces, discard_empty_cells);
}

struct csv_row * parse_char_array_to_csv_row(char arr[], int arrlen, char delim, char quot_char, int strip_spaces, int discard_empty_cells){
	// use the parse char array function for table and just gets the first row

	struct csv_table * table = parse_char_array_to_csv_table(arr, arrlen, delim, quot_char, strip_spaces, discard_empty_cells);

	if ( table == NULL || table->row_count == 0 ){
		return NULL;
	}

	struct csv_row *first_row = pop_row_from_csv_table(table, 0);

	free_csv_table(table);

	return first_row;
}

struct csv_row * parse_string_to_csv_row(char * string, char delim, char quot_char, int strip_spaces, int discard_empty_cells){
	return parse_char_array_to_csv_row(string, strlen(string)+1, delim, quot_char, strip_spaces, discard_empty_cells);
}

struct csv_table * parse_file_to_csv_table(FILE * csv_file, char delim, char quot_char, int strip_spaces, int discard_empty_cells){
	return parse_fileptr_or_char_array_to_csv_table(csv_file, NULL, 0, delim, quot_char, strip_spaces, discard_empty_cells, FALSE);
}

struct csv_table * open_and_parse_file_to_csv_table(char * filename, char delim, char quot_char, int strip_spaces, int discard_empty_cells){
	// we open the file for them
	FILE * csv_file = fopen(filename, "r");
	if ( csv_file == NULL ) {
		printf("Could not open CSV file (%s)!!!\n", filename);
		exit(1);
	}

	struct csv_table * parsed_table = parse_file_to_csv_table(csv_file, delim, quot_char, strip_spaces, discard_empty_cells);

	fclose(csv_file);

	return parsed_table;
}