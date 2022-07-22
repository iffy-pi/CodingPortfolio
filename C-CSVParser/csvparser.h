#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define VERBOSE 0

#define BUFFSIZE 1024

struct csv_cell {
	char * str;
	struct csv_row * parent_row;
	struct csv_cell * next;
	struct csv_cell * prev;
};

struct csv_row {
	int cell_count;
	struct csv_cell * cell_list_head;
	struct csv_cell * cell_list_tail;

	struct csv_table * parent_table;
	struct csv_row * next;
	struct csv_row * prev;

};

struct csv_table {
	int row_count;
	struct csv_row * row_list_head;
	struct csv_row * row_list_tail;
};

int mallocstrcpy(char ** dest, char * src, int len);

void print_csv_cell(struct csv_cell * cellptr );
void print_csv_row(struct csv_row * rowptr);
void print_csv_table(struct csv_table * tableptr);

/* Allocate csv structures on the heap, anything allocated should be deallocated with free functions */
struct csv_cell * new_csv_cell();
struct csv_cell * new_csv_cell_from_str(char * string);
struct csv_row * new_csv_row();
struct csv_table * new_csv_table();
void free_csv_cell(struct csv_cell * cellptr);
void free_csv_row(struct csv_row * rowptr);
void free_csv_table(struct csv_table * tableptr);

void populate_csv_cell_str(struct csv_cell * cell, char * string);

/* Performs a deep copy of the specified parameters and returns a pointer to the new object */
struct csv_cell * clone_csv_cell(struct csv_cell * cell);
struct csv_row * clone_csv_row(struct csv_row * row);
struct csv_table * clone_csv_table(struct csv_table * table);

/* Compares the values of the cells/rows and returns TRUE if they are the same */ 
int csv_cell_equals(struct csv_cell * cell1, struct csv_cell * cell2);
int csv_row_equals(struct csv_row * row1, struct csv_row * row2);
int csv_table_equals(struct csv_table * table1, struct csv_table * table2);

/* Get pointer to cell or row at the specified index */
struct csv_cell * get_cell_ptr_in_csv_row(struct csv_row * row, int index);
struct csv_row * get_row_ptr_in_csv_table(struct csv_table * table, int index);
struct csv_cell * get_cell_ptr_in_csv_table(struct csv_table * table, int rowindx, int colindx);

/* Clone cell/row contents at specified index and return pointer to allocated memory */
struct csv_cell * get_cell_clone_in_csv_row(struct csv_row * row, int index);
struct csv_row * get_row_clone_in_csv_table(struct csv_table * table, int index);
struct csv_cell * get_cell_clone_in_csv_table(struct csv_table * table, int rowindx, int colindx);

/* Gets the pointer to the cell in the specified row/table that contains the string, returns NULL if no match is found */
struct csv_cell * get_cell_for_str_in_csv_row(struct csv_row * row, char * string);
struct csv_cell * get_cell_for_str_in_csv_table(struct csv_table * table, char * string);

/* Gets the index of the specified row/cell in the specified row/table */
/* Done by comparing the values of the cell/row and returning index where values match */
/* Returns -1 if no match */
int get_cell_coord_in_csv_row(struct csv_row * row, struct csv_cell * cell);
int get_row_coord_in_csv_table(struct csv_table * table, struct csv_row * row);
/* takes pointers to row and column index coordinates, return value is -1 if no match */
int get_cell_coord_in_csv_table(struct csv_table * table, struct csv_cell * cell, int * rowindx, int * colindx);

/* Gets the coordinates of the cell that contains the specified string */
int get_str_coord_in_csv_row(struct csv_row * row, char * string);
int get_str_coord_in_csv_table(struct csv_table * table, char * string, int * rowindx, int * colindx);

/* Checks if the cell/row ptr is in the specified table/row list, returns TRUE if is */
// just a pointer comparison
int is_cell_mapped_to_csv_row(struct csv_row * row, struct csv_cell * cellptr);
int is_row_mapped_to_csv_table(struct csv_table * table, struct csv_row * rowptr);
int is_cell_mapped_to_csv_table(struct csv_table * table, struct csv_cell * cellptr);

/* Checks if the value of the specified cell/row is in the specified table/row list, returns TRUE if is */
int is_cell_in_csv_row(struct csv_row * row, struct csv_cell * cell);
int is_row_in_csv_table(struct csv_table * table, struct csv_row * row);
int is_cell_in_csv_table(struct csv_table * table, struct csv_cell * cell);

/* Checks if specified string is in the csv table, returns TRUE if match is found */
int is_string_in_csv_row(struct csv_row * row, char * string);
int is_string_in_csv_table(struct csv_table * table, char * string);

/* Add csv cell/row to csv row/table list by adding pointer to list, uses shallow copy*/
void map_cell_into_csv_row(struct csv_row * rowptr, struct csv_cell * cellptr);
void map_row_into_csv_table(struct csv_table * tableptr, struct csv_row * rowptr);

/* Add csv cell/row to csv row/table list, uses deep copy (allocates new memory for new rows/cells) */
void add_cell_clone_to_csv_row(struct csv_row * rowptr, struct csv_cell * cellptr);
void add_row_clone_to_csv_table(struct csv_table * tableptr, struct csv_row * rowptr);

/* Create new cell with word and add it to row, uses deep copy for copying word */
void add_str_to_csv_row(struct csv_row * rowptr, char * word, int wordlen);

/* Unmap the specified cell/row ptr from its parent row/table */
void unmap_cell_in_csv_row(struct csv_row * row, struct csv_cell *  cellptr);
void unmap_row_in_csv_table(struct csv_table * table, struct csv_row * rowptr);
void unmap_cell_in_csv_table(struct csv_table * table, struct csv_cell * cellptr);

/* removes the specified cell/row at the specified coordinates in the specified row/table list and returns a pointer to it */
/* returns NULL if it could not find a match */
struct csv_cell * pop_cell_from_csv_row(struct csv_row * row, int index);
struct csv_row * pop_row_from_csv_table(struct csv_table * table, int index);
struct csv_cell * pop_cell_from_csv_table(struct csv_table * table, int rowindx, int colindx);

/* removes and frees the specified cell/row at the specified coordinates in the specified row/table */
/* returns 0 if things were deleted */
int delete_cell_from_csv_row(struct csv_row * row, int index);
int delete_row_from_csv_table(struct csv_table * table, int index);
int delete_cell_from_csv_table(struct csv_table * table, int rowindx, int colindx);

/* used as for loop conditional when iterating through list of rows */
/* for( cur_row=table->row_list_head; has_next_row(table, cur_row); cur_row =cur_row->next ); */ 
int has_next_cell(struct csv_row * row, struct csv_cell * cur_cell);
int has_next_row(struct csv_table * table, struct csv_row * cur_row);

/* parse csv file and strings to csv table structure (linked list of csv rows) */
/*
	delim -> what should be used as the delimiter
	strip_spaces -> if TRUE, trims cell strings of trailing spaces
	discard_empty_cells -> if TRUE, any cells with empty string (after trimming) will not be put into returned structure

	if delim is a space (' '), then values for ignore parameters are overridden:
	strip_spaces = FALSE
	discard_empty_cells = TRUE
*/
struct csv_table * parse_string_to_csv_table(char str[], int charcount, char delim, int strip_spaces, int discard_empty_cells);
struct csv_row * parse_line_to_csv_row(char curline[], int charcount, char delim, int strip_spaces, int discard_empty_cells);
struct csv_table * parse_file_to_csv_table(FILE * fileptr, char delim, int strip_spaces, int discard_empty_cells);
struct csv_table * open_and_parse_file_to_csv_table(char * filename, char delim, int strip_spaces, int discard_empty_cells);
