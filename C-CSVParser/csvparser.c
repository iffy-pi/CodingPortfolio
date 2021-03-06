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

char * malloc_strip_quotes_and_spaces(char  * string, int len, int strip_quotes, int strip_spaces, int free_string){
	// strips string of leading and trailing spaces
	// returns a pointer to the stripped string, allocated using malloc
	// option to free old string as parameter

	if ( string == NULL ) return NULL;

	int new_word_start_pos = 0;
	int new_word_end_pos = len;

	// remove the original quotes if present
	if ( strip_quotes && string[new_word_start_pos] == '"' && string[new_word_end_pos-1] == '"') {

		new_word_start_pos++;
		new_word_end_pos--;
	}


	// go through the word and find the beginning of non space character
	while(strip_spaces && new_word_end_pos != new_word_start_pos && string[new_word_end_pos-1] == ' ' ) new_word_end_pos--;
	while(strip_spaces && new_word_start_pos != new_word_end_pos && string[new_word_start_pos] == ' ') new_word_start_pos++;

	// now we have our range subtract to get new word length and allocate on the heap
	int new_wordlen = new_word_end_pos - new_word_start_pos;

	char *new_string = (char *)malloc( (new_wordlen+1) * sizeof(char));

	// copy the words over
	int place_character;
	int new_string_indx = 0;
	int quot_count = 0;

	for(int old_string_indx=new_word_start_pos; old_string_indx < new_word_end_pos; old_string_indx++){
		// if the current character is quote and previous character is not quote, then dont add the quote
		place_character = TRUE;
		if ( string[old_string_indx] == '"' ){

			// quot can only be placed if it is escaped, i.e. for '""' result is '"' and for '""""' result is '""'
			// quots must be removed if
			// quot appears at the beginning of the word (there is no quot behind it)
			// previous character is not a quot
			// previous character is a quot but not an even quot count, indicates it was used as escape for another quot
			quot_count = (quot_count + 1) % 2;
			if ( old_string_indx == new_word_start_pos || string[old_string_indx-1] != '"' || string[old_string_indx-1] == '"' && quot_count == 1 ) place_character = FALSE;
		}

		if ( place_character ){
			new_string[new_string_indx] = string[old_string_indx];
			new_string_indx++;
		}
	}

	for( new_string_indx=new_string_indx; new_string_indx < new_wordlen; new_string_indx++){
		// populate the remaining spaces with null terminator?, this may not be safe
		new_string[new_string_indx] = '\0';
	}
	new_string[new_wordlen] = '\0';

	if (free_string) free(string);

	return new_string;

}

struct csv_cell * new_csv_cell(){
	struct csv_cell * cellptr = (struct csv_cell *) malloc(sizeof(struct csv_cell));
	cellptr->str = NULL;
	cellptr->parent_row = NULL;
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

void print_csv_cell(struct csv_cell * cellptr ){
	if (cellptr ==  NULL ){
		printf("(null)");
		return;
	}
	printf("\"%s\"",cellptr->str);
}

void print_csv_row(struct csv_row * rowptr){

	if (rowptr == NULL ){
		printf("(null)");
		return;
	}

	int printed = 0;

	struct csv_cell * cur_cell = rowptr->cell_list_head;
	printf("[");
	while ( cur_cell != NULL && cur_cell->next != NULL ){
		// while there is a next cell
		// so we will break when we reach tail cell

		print_csv_cell(cur_cell);
		printf(", ");
		printed++;

		// move up the list
		cur_cell = cur_cell->next;
	}

	if (rowptr->cell_list_tail != NULL ) {
		print_csv_cell(rowptr->cell_list_tail);
		printed++;
	}

	printf("]\n");

	if (printed != rowptr->cell_count) {
		printf("Unbalanced!\n");
		exit(1);
	}
}

void print_csv_table(struct csv_table * tableptr){
	if ( tableptr == NULL ) {
		printf("(null)\n");
		return;
	}

	int printed = 0;

	struct csv_row * cur_row = tableptr->row_list_head;

	printf("[\n");

	while ( cur_row != NULL && cur_row->next != NULL ){
		// while there is a next cell
		// so we will break when we reach tail cell

		print_csv_row(cur_row);
		printed++;


		// move up the list
		cur_row = cur_row->next;
	}

	// will break when we are at tail
	if (tableptr->row_list_tail != NULL ) {
		print_csv_row(tableptr->row_list_tail);
		printed++;
	}

	printf("]\n");

	if ( tableptr->row_count != printed) {
		printf("Missing rows somewhere!");
		exit(1);
	}
}

void populate_csv_cell_str(struct csv_cell * cell, char * string){
	// copy the string with mallocstr copy
	mallocstrcpy(&(cell->str), string, strlen(string));
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
		add_str_to_csv_row( new_row, cur_cell->str, strlen(cur_cell->str));
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
	cellptr->parent_row = rowptr;

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

void add_cell_clone_to_csv_row(struct csv_row * rowptr, struct csv_cell * cellptr){
	struct csv_cell * new_cell = clone_csv_cell(cellptr);
	map_cell_into_csv_row(rowptr, new_cell);
}

void add_row_clone_to_csv_table(struct csv_table * tableptr, struct csv_row * rowptr){
	struct csv_row * new_row = clone_csv_row(rowptr);
	map_row_into_csv_table(tableptr, rowptr);
}

void add_str_to_csv_row(struct csv_row * rowptr, char * word, int wordlen){
	// create a new element structure
	struct csv_cell * cellptr = new_csv_cell();

	// use mallocstrcpy to copy the word
	mallocstrcpy( &(cellptr->str), word, wordlen);

	map_cell_into_csv_row(rowptr, cellptr);
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
	cellptr->parent_row = NULL;
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
	struct csv_row * rel_row = cellptr->parent_row;

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
	if ( cur_cell != NULL && cur_cell->parent_row != row ) {
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


struct csv_table * parse_fileptr_or_char_array_to_csv_table( FILE * csv_file, char string[], int string_len, char delim, int strip_spaces, int discard_empty_cells, int verbose){
	int parsing_string = ( string != NULL ) && ( string_len > 0);
	int parsing_file = ( csv_file != NULL);

	if ( parsing_file == parsing_string ){
		// they are both 0 or they are both one
		// in both cases we cant decide what to parse, return null;
		return NULL;
	}

	// buffer for fgets
	int bufflen;
	char * buffer;
	if ( parsing_string) {
		// then we dont need that buffer
		buffer = string;
		bufflen = string_len;
	} else {
		// parsing file
		bufflen = 100;
		buffer = (char *) malloc(bufflen * sizeof(char));
	}


	char second_last_char;

	// how many times we got the buffer using fgets, will be 0 for parsing string
	int buffer_iterations = 0;

	// reporting fgets errors
	int error_occured = FALSE;

	// used csv structures
	struct csv_table *table = new_csv_table();
	struct csv_row *cur_row = NULL;

	// values used for merging and appending cells
	struct csv_cell *cur_cell, *last_cell, *replacement_cell;
	int last_word_len, combined_str_len, cur_cell_str_len, merging_cells;

	// character position items for getting the current word
	int cur_pos, cur_word_start_pos, cur_word_end_pos, cur_delim_pos;
	char * cur_word;
	int cur_word_len;

	// parsing flags
	int has_reached_delim, has_reached_space, has_reached_newl, has_reached_crnewl, has_reached_only_cr;
	int is_at_a_delim, has_reached_eos, has_reached_eof, eos_char_offset;

	// counting how many " appear, used for proper parsing
	int quot_count = 0;

	// used to indicate if the first word in the current buffer has been parsed
	// used in parsing files
	int first_word_for_buffer_parsed;

	// indicates if the entire line was read into buffer
	int entire_cur_line_in_buffer;

	// used as a checker
	int append_this_cell;

	// if parsing file, we call fgets till we have read entire file
	// if parsing string, we know string is entirely in memory so no need for other work

	while ( (parsing_file && !feof(csv_file)) || ( parsing_string && buffer_iterations < 1) ){

		// if we are parsing file, multiple iterations of fgets, so continue as going
		if ( parsing_file ){
			memset(buffer, 0, bufflen*sizeof(char));


			// reads characters into buffer
			// stops when it reaches newline, end of file or read bufflen-1 characters
			// places null terminator at position it stopped at
			// source: https://www.ibm.com/docs/en/i/7.4?topic=functions-fgets-read-string
			// printf("File finished: %d\n", feof(csv_file));
			// printf("Doing fgets\n");
			fgets(buffer, bufflen, csv_file);

			if ( ferror(csv_file) ){
				// returns TRUE if an error occured while reading the CSV file
				error_occured = TRUE;
				break;
			}
		}

		if ( buffer[0] != '\0' ){

			if (verbose) printf("---------------------------------->\n");

			if ( parsing_string ){
				// parsing string, buffer has all lines in the file
				entire_cur_line_in_buffer = TRUE;
			} else {
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
			}

			// parse the buffer character by character as usual

			// when new buffer has been retrieved reset our cur_pos and cur_word_start_pos
			cur_pos = 0;
			cur_word_start_pos = 0;
			first_word_for_buffer_parsed = FALSE;

			// character by character iterator
			while ( TRUE ){
				has_reached_delim = (buffer[cur_pos] == delim);
				has_reached_eos = ( buffer[cur_pos] == '\0' || cur_pos == bufflen-1);
				has_reached_crnewl =  ( buffer[cur_pos] == '\r' && buffer[cur_pos+1] == '\n');
				has_reached_newl = ( buffer[cur_pos] == '\n' );
				has_reached_only_cr = ( buffer[cur_pos] == '\r' && !has_reached_crnewl );
				has_reached_eof = ( parsing_file && has_reached_eos && feof(csv_file) );

				if ( buffer[cur_pos] == '"') quot_count = (quot_count+1) % 2;

				// if quot_count is odd, then we are in the middle of a quot, so delim and newline should be ignored
				if ( quot_count != 0 ){
					if (has_reached_delim) {
						//printf("Cur pos: %d, resetting delim!\n",cur_pos);
						has_reached_delim = FALSE;
					}
				}

				// if quot_count != 0, then has_reached_delim = FALSE
				// quot_count == 0 && has_reached_delim
				// if quot_count is odd, tn

				is_at_a_delim = ( (quot_count == 0 && has_reached_delim) || (quot_count == 0 && has_reached_crnewl) || (quot_count == 0 && has_reached_newl) || (quot_count == 0 && has_reached_only_cr) || has_reached_eos );

				if ( is_at_a_delim ){
					cur_delim_pos = cur_pos;
					cur_word_end_pos = cur_delim_pos;

					cur_word_len = cur_word_end_pos - cur_word_start_pos;

					// the cur word just points to where it starts in the buffer
					// we have the current word len so we know where it ends
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

					// do stuff with the new word here
					merging_cells = FALSE;
					if ( cur_row == NULL ){
						// we made a complete line sometime before, reset it here
						cur_row = new_csv_row();

					} else if ( cur_row->cell_count > 0 && !first_word_for_buffer_parsed ){
						// if this is false that means this is the first word,
						// we need to combine it with the last word in the cur row

						last_cell = get_cell_ptr_in_csv_row(cur_row, cur_row->cell_count-1);
						last_word_len = strlen(last_cell->str);

						combined_str_len = last_word_len + cur_word_len;

						if (verbose) printf("Merging: \"%s\" + $cur_word\n", last_cell->str);


						replacement_cell = new_csv_cell();
						// allocate the string for the replacement cell 
						replacement_cell->str = (char *) malloc( ( combined_str_len + 1)*sizeof(char));

						// copy over the characters into the new string
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
					
					// do not strip quotes or spaces if this is the last word in the buffer and the line is not complete
					// last word in the buffer: buffer[cur_word_end_pos] == '\0'
					// line is not complete: !entire_cur_line_in_buffer
					// if quot_count is odd, then the newline is part of the cell so the entire line is not in buffer
					// unless this buffer is the final buffer for the file (feof is true)
					// this does not apply for regular string since buffer contains entire line
					if (  parsing_file && (!( (quot_count == 0 && entire_cur_line_in_buffer) || feof(csv_file) ) && buffer[cur_word_end_pos] == '\0')){
						if ( !merging_cells ) mallocstrcpy( &cur_cell->str, cur_word, cur_word_len);
					} else {
						// not the last word in buffer without entire line
						// or is the last word but the buffer has entire line
						// strip spaces and quotes

						if (verbose) printf("Stripping quotes and spaces\n");

						if ( merging_cells){
							cur_cell->str = malloc_strip_quotes_and_spaces(cur_cell->str, combined_str_len, TRUE, strip_spaces, TRUE);
						} else{
							cur_cell->str = malloc_strip_quotes_and_spaces(cur_word, cur_word_len, TRUE, strip_spaces, FALSE);
						}

						// do not append this cell if we are discarding empties and it has empty string
						append_this_cell = ( !discard_empty_cells || (discard_empty_cells && strlen(cur_cell->str) > 0) );
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
				// fgets only gets to newline so we know we have reached end of the buffer
				if ( ( quot_count == 0 && has_reached_newl ) || ( quot_count == 0 && has_reached_only_cr ) || ( quot_count == 0 && has_reached_crnewl ) || has_reached_eof || (parsing_string && has_reached_eos) ){
					// we have gotten to the end of a line
					// append the currernt row
					if ( verbose ){
						printf("===============================\n");
						printf("Final Row:\n");
						print_csv_row(cur_row);
						printf("===============================\n");
					}
					if( cur_row != NULL ) map_row_into_csv_table(table, cur_row);
					// reset the row to none
					cur_row = NULL;

					// reset the quot count for next line
					quot_count = 0;

					// if we have reached end of string or end of file then break
					if ( has_reached_eos || has_reached_eof ) break;

					//If newl or crnewl, there could be more lines
					eos_char_offset = 0;
					if ( has_reached_crnewl ) eos_char_offset+=2; // possible terminator is +2 from current index
					else if ( has_reached_newl || has_reached_only_cr ) eos_char_offset+=1; // possible null terminator is +1 from current idnex

					// if there are no more lines, eof char offset will point to the terminator
					if ( buffer[cur_pos+eos_char_offset] == '\0' ){
						// then there are no other lines so we are done with the string
						break;
					}
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

struct csv_table * parse_char_array_to_csv_table(char arr[], int arrlen, char delim, int strip_spaces, int discard_empty_cells){
	return parse_fileptr_or_char_array_to_csv_table(NULL, arr, arrlen, delim, strip_spaces, discard_empty_cells, FALSE);
}

struct csv_table * parse_string_to_csv_table(char * string, char delim, int strip_spaces, int discard_empty_cells){
	return parse_char_array_to_csv_table(string, strlen(string)+1, delim, strip_spaces, discard_empty_cells);
}

struct csv_row * parse_char_array_to_csv_row(char arr[], int arrlen, char delim, int strip_spaces, int discard_empty_cells){
	// use the parse char array function for table and just gets the first table

	struct csv_table * table = parse_char_array_to_csv_table(arr, arrlen, delim, strip_spaces, discard_empty_cells);

	if ( table == NULL || table->row_count == 0 ){
		return NULL;
	}

	// obtain the first row and unmap it from the table
	struct csv_row *parsed_row = pop_row_from_csv_table(table, 0);

	// free the parsed table, we wont free the row since we unmapped it
	free_csv_table(table);

	// return the parsed row
	return parsed_row;
}

struct csv_row * parse_string_to_csv_row(char * string, char delim, int strip_spaces, int discard_empty_cells){
	return parse_char_array_to_csv_row(string, strlen(string)+1, delim, strip_spaces, discard_empty_cells);
}

struct csv_table * parse_file_to_csv_table(FILE * csv_file, char delim, int strip_spaces, int discard_empty_cells){
	return parse_fileptr_or_char_array_to_csv_table(csv_file, NULL, 0, delim, strip_spaces, discard_empty_cells, FALSE);
}

struct csv_table * open_and_parse_file_to_csv_table(char * filename, char delim, int strip_spaces, int discard_empty_cells){
	// we open the file for them
	FILE * csv_file = fopen(filename, "r");
	if ( csv_file == NULL ) {
		printf("Could not open CSV file (%s)!!!\n", filename);
		exit(1);
	}

	struct csv_table * parsed_table = parse_file_to_csv_table(csv_file, delim, strip_spaces, discard_empty_cells);

	fclose(csv_file);

	return parsed_table;
}