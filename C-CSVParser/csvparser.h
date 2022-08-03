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
	// points to its parent row
	struct csv_row * parent;
	struct csv_cell * next;
	struct csv_cell * prev;
};

struct csv_row {
	int length;
	//pointers for head and tail of cell list
	struct csv_cell * list_head;
	struct csv_cell * list_tail;

	// points to its parent table
	struct csv_table * parent;
	struct csv_row * next;
	struct csv_row * prev;

};

struct csv_table {
	int length;
	// pointers for head and tail of row list
	struct csv_row * list_head;
	struct csv_row * list_tail;
};

int mallocstrcpy(char **dest, char * src, int len);
char * malloc_strip_quotes_and_spaces(char  *string, int len, char quot_char, int strip_quotes, int strip_spaces, int free_string);

/* Allocate csv structures on the heap, anything allocated should be deallocated with free functions */
struct csv_cell * new_csv_cell();
struct csv_cell * new_csv_cell_from_str(char *string);
struct csv_row * new_csv_row();
struct csv_table * new_csv_table();
void free_csv_cell(struct csv_cell *cellptr);
void free_csv_row(struct csv_row *rowptr);
void free_csv_table(struct csv_table *tableptr);

void populate_csv_cell_str(struct csv_cell *cell, char *string);

/* Print the contents of a CSV structure */
void print_csv_cell(struct csv_cell *cellptr );
void print_csv_row(struct csv_row *rowptr);
void print_csv_table(struct csv_table *tableptr);

/* Prints cells/rows of row/table in more organized format*/
void pretty_print_csv_row(struct csv_row *rowptr);
void pretty_print_csv_table(struct csv_table *tableptr);
void super_pretty_print_csv_table(struct csv_table *tableptr);


/* Performs a deep copy of the specified parameters and returns a pointer to the new object */
struct csv_cell * clone_csv_cell(struct csv_cell *cell);
struct csv_row * clone_csv_row(struct csv_row *row);
struct csv_table * clone_csv_table(struct csv_table *table);

/* Compares the values of the cells/rows and returns TRUE if they are the same */ 
int csv_cell_equals(struct csv_cell *cell1, struct csv_cell *cell2);
int csv_row_equals(struct csv_row *row1, struct csv_row *row2);
int csv_table_equals(struct csv_table *table1, struct csv_table *table2);

/* Get pointer to cell or row at the specified index */
struct csv_cell * get_cell_ptr_in_csv_row(struct csv_row *row, int index);
struct csv_row * get_row_ptr_in_csv_table(struct csv_table *table, int index);
struct csv_cell * get_cell_ptr_in_csv_table(struct csv_table *table, int rowindx, int colindx);

/* Clone cell/row contents at specified index and return pointer to allocated memory */
struct csv_cell * get_cell_clone_in_csv_row(struct csv_row *row, int index);
struct csv_row * get_row_clone_in_csv_table(struct csv_table *table, int index);
struct csv_cell * get_cell_clone_in_csv_table(struct csv_table *table, int rowindx, int colindx);

/* Gets the pointer to the cell in the specified row/table that contains the string, returns NULL if no match is found */
struct csv_cell * get_cell_for_str_in_csv_row(struct csv_row *row, char *string);
struct csv_cell * get_cell_for_str_in_csv_table(struct csv_table *table, char *string);

/* Gets the index of the specified row/cell in the specified row/table */
/* Done by comparing the values of the cell/row and returning index where values match */
/* Returns -1 if no match */
int get_cell_coord_in_csv_row(struct csv_row *row, struct csv_cell *cell);
int get_row_coord_in_csv_table(struct csv_table *table, struct csv_row *row);
/* takes pointers to row and column index coordinates, return value is -1 if no match */
int get_cell_coord_in_csv_table(struct csv_table *table, struct csv_cell *cell, int *rowindx, int *colindx);

/* Gets the coordinates of the cell that contains the specified string */
int get_str_coord_in_csv_row(struct csv_row *row, char *string);
int get_str_coord_in_csv_table(struct csv_table *table, char *string, int *rowindx, int *colindx);

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
void map_cell_into_csv_row(struct csv_row *rowptr, struct csv_cell *cellptr);
void map_row_into_csv_table(struct csv_table *tableptr, struct csv_row *rowptr);

/* Maps the structure into the list at the given index, i.e. mapped structure will have that index */
/* Everything after is pushed one up */
/* Returns 0 if successful map, non zero for errors */
int insmap_cell_into_csv_row(struct csv_row *row, struct csv_cell *cell, int index);
int insmap_row_into_csv_table(struct csv_table *table, struct csv_row *row, int index);
int insmap_cell_into_csv_table(struct csv_table *table, struct csv_cell *cell, int rowindx, int colindx);

/* Add csv cell/row to csv row/table list, uses deep copy (allocates new memory for new rows/cells) */
void add_cell_clone_to_csv_row(struct csv_row *rowptr, struct csv_cell *cellptr);
void add_row_clone_to_csv_table(struct csv_table *tableptr, struct csv_row *rowptr);

/* Add but at specific coordinates , same return type as map_to_coord */
int insert_cell_clone_into_csv_row(struct csv_row *rowptr, struct csv_cell *cellptr, int index);
int insert_row_clone_into_csv_table(struct csv_table *tableptr, struct csv_row *rowptr, int index);
int insert_cell_clone_into_csv_table(struct csv_table *tableptr, struct csv_cell *cellptr, int rowindx, int colindx);

/* Create new cell with word and add it to end of row, uses deep copy for copying word */
int add_char_array_to_csv_row(struct csv_row *rowptr, char arr[], int arrlen);
int add_str_to_csv_row(struct csv_row *rowptr, char * string);

/* Insert new cell with string to row/table at specified coordinates */
int insert_str_into_csv_row(struct csv_row *rowptr, char *string, int index);
int insert_str_into_csv_table(struct csv_table *tableptr, char *string, int rowindx, int colindx);


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
/* for( cur_row=table->list_head; has_next_row(table, cur_row); cur_row =cur_row->next ); */ 
int has_next_cell(struct csv_row * row, struct csv_cell * cur_cell);
int has_next_row(struct csv_table * table, struct csv_row * cur_row);

/* Base function for all the below functions */
/* Either parses file or string */
/*
	delim				: what should be used as the delimiter
	quot_char			: What is used as the quote character, either '"' or '''
	strip_spaces 		: if TRUE, trims cell strings of trailing spaces and leading spaces
	discard_empty_cells : if TRUE, any cells with empty string (after stripping) will not be put into returned structure

	if delim is a space (' '), then values for ignore parameters are overridden:
	strip_spaces = FALSE
	discard_empty_cells = TRUE

	csv_file			: File pointer to CSV file to parse, set to NULL if you want to parse the character array
	arr 				: The character array you want to parse, set to NULL if you want to parse the CSV file
	arrlen 			: The size of the character array, set to 0 if you want to parse the CSV file

	Can only parse either arr or file, not both
	if ( csv_file != NULL && (arr == NULL || arrlen < 0)) function will return parsed csv for csv_file
	if ( csv_file == NULL && (arr != NULL && arrlen > 0)) function will return parsed csv for character array
	
	if ( csv_file == NULL && (arr == NULL || arrlen < 0)) function will return NULL
	if ( csv_file != NULL && (arr != NULL && arrlen > 0)) function will return NULL

	if an error occurs while reading the file, function will return NULL

*/
struct csv_table * parse_fileptr_or_char_array_to_csv_table( FILE * csv_file, char arr[], int arrlen, char delim, char quot_char, int strip_spaces, int discard_empty_cells, int verbose);

/* Parse character array or string into csv_table, parse_string calls parse_char_array with strlen(string)+1 as arrlen */
struct csv_table * parse_char_array_to_csv_table(char arr[], int arrlen, char delim, char quot_char, int strip_spaces, int discard_empty_cells);
struct csv_table * parse_string_to_csv_table(char * string, char delim, char quot_char, int strip_spaces, int discard_empty_cells);

/* Parse character array or string into csv_row, parse_string calls parse_char_array with strlen(string)+1 as arrlen */
struct csv_row * parse_char_array_to_csv_row(char arr[], int arrlen, char delim, char quot_char, int strip_spaces, int discard_empty_cells);
struct csv_row * parse_string_to_csv_row(char * string, char delim, char quot_char, int strip_spaces, int discard_empty_cells);

/* Parses a file pointer into csv_table */
struct csv_table * parse_file_to_csv_table(FILE * fileptr, char delim, char quot_char, int strip_spaces, int discard_empty_cells);

/* Opens the specified file and parses it into csv_table */ 
struct csv_table * open_and_parse_file_to_csv_table(char * filename, char delim, char quot_char, int strip_spaces, int discard_empty_cells);
