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

int is_char_control_character(char c){
	// source: https://www.gaijin.at/en/infos/ascii-ansi-character-table
	return ( (c >= 0 && c <= 31) || ( c == 127) );
}

struct csv_table * parse_string_to_csv_table(char str[], int charcount, char delim, int strip_spaces, int discard_empty_cells){
	if ( delim == ' ' ) {
		strip_spaces = FALSE;
		discard_empty_cells = TRUE;
	}

	int cur_pos, cur_word_start_pos, cur_word_end_pos;
	char * cur_word = NULL;
	int cur_word_len;
	int cur_delim_pos;

	int has_reached_delim, has_reached_space, has_reached_newl, has_reached_crnewl, has_reached_only_cr;
	int is_at_a_delim, has_reached_eos, eof_char_offset;

	struct csv_table * parsed_table;
	struct csv_row * parsed_row;

	int linecnt = 0;

	cur_pos = 0;
	cur_word_start_pos = 0;

	// make row and table csv structures
	parsed_table = new_csv_table();
	parsed_row = new_csv_row();

	while ( TRUE ){

		has_reached_delim = (str[cur_pos] == delim);
		has_reached_crnewl = ( str[cur_pos] == '\r' && str[cur_pos+1] == '\n');
		has_reached_newl = ( str[cur_pos] == '\n' );
		has_reached_only_cr = ( str[cur_pos] == '\r' && !has_reached_crnewl );
		has_reached_eos = ( str[cur_pos] == '\0' );

		is_at_a_delim = ( has_reached_delim || has_reached_crnewl || has_reached_newl || has_reached_only_cr || has_reached_eos );

		if ( is_at_a_delim ){
			// we have reached the end of a word,
			//empty the previous word if there was one
			if ( cur_word != NULL ) {
				free(cur_word);
				cur_word = NULL;
			}

			cur_delim_pos = cur_pos;
			cur_word_end_pos = cur_delim_pos;
			// there might be trailing spaces on the word, so go backwards until we find non space character
			if ( strip_spaces ){
				while ( cur_word_end_pos != cur_word_start_pos && str[cur_word_end_pos-1] == ' ') cur_word_end_pos--;
				// remove trailing spaces from the word start
				while ( cur_word_start_pos != cur_word_end_pos && str[cur_word_start_pos] == ' ') cur_word_start_pos++;
			}

			cur_word_len = cur_word_end_pos - cur_word_start_pos;

			if ( mallocstrcpy( &cur_word, (str+cur_word_start_pos), cur_word_len ) != 0){
				printf("Allocation failed!\n");
				exit(1);
			}

			if ( !discard_empty_cells || (discard_empty_cells && cur_word_len > 0)){
				// add the word to the current row
				add_str_to_csv_row(parsed_row, cur_word, cur_word_len);
			}

			// next word starts after this word ends
			cur_word_start_pos = cur_delim_pos+1;
			if (has_reached_crnewl) cur_word_start_pos++;

		}

		// it has reached the end of the line if there is a new loine, a carriage return or an end of file
		if ( has_reached_newl || has_reached_only_cr || has_reached_crnewl || has_reached_eos ){
			// we have gotten to the end of a line
			// append the currernt row
			map_row_into_csv_table(parsed_table, parsed_row);

			// if we have reached end of string then break
			if ( has_reached_eos ) break;

			//If newl or crnewl, there could be more lines
			eof_char_offset = 0;
			if ( has_reached_crnewl ) eof_char_offset+=2; // possible terminator is +2 from current index
			else if ( has_reached_newl || has_reached_only_cr ) eof_char_offset+=1; // possible null terminator is +1 from current idnex

			// if there are no more lines, eof char offset will point to the terminator
			if ( str[cur_pos+eof_char_offset] == '\0' ){
				// then there are no other lines so we are done with the string
				break;
			} else {
				// there are more lines, create a new row
				parsed_row = new_csv_row();
				linecnt++;
			}

		}


		if ( str[cur_pos] == '\0' ) break;

		if ( has_reached_crnewl ) cur_pos++;
		cur_pos++;
	}

	if ( cur_word != NULL ){
		free(cur_word);
		cur_word = NULL;
	}

	return parsed_table;

}

struct csv_row * parse_line_to_csv_row(char curline[], int charcount, char delim, int strip_spaces, int discard_empty_cells){

	// use the string table and proceed
	struct csv_table * table = parse_string_to_csv_table(curline, charcount, delim, strip_spaces, discard_empty_cells);

	if ( table == NULL || table->row_count == 0 ){
		return NULL;
	}

	// obtain and clone the first row
	struct csv_row * parsed_row = clone_csv_row( get_row_ptr_in_csv_table(table, 0));

	// free the parsed table
	free_csv_table(table);

	return parsed_row;
	
}

struct csv_table * parse_file_to_csv_table(FILE * fileptr, char delim, int strip_spaces, int discard_empty_cells){
	// open the csv file

	/*
	each line is in the format
	<vm name>,<username>,...<username>

	Multiple usernames can be specified on each line
	Function supports trailing comma on csv line
	*/

	// cur line is max MAX_CHARS_PER_LINE_IN_CSV_FILE characters
	int max_char_count = BUFFSIZE;
	char curline[max_char_count];

	struct csv_table * parsed_table = new_csv_table();
	struct csv_row * cur_row;

	while ( (fgets(curline, max_char_count, fileptr) != NULL) ){
		// fgets reads the current line into curline array, max_char_count is maximum amount of lines to read
		// stops when it hits newline or null terminator

		cur_row = parse_line_to_csv_row(curline, BUFFSIZE, delim, strip_spaces, discard_empty_cells);

		// add the current row to the table
		map_row_into_csv_table(parsed_table, cur_row);
	}

	//fclose(csv_file); only being passed a file pointer so we leave it to them to close it

	return parsed_table;
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