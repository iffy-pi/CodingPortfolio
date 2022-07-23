#include "csvparser.h"


char * malloc_stripped_str(char * string, int free_str){
	// strips string of leading and trailing spaces
	// returns a pointer to the stripped string, allocated using malloc
	// option to free old string as parameter

	if ( string == NULL ) return NULL;

	int word_start_pos = 0;
	int word_end_pos = strlen(string);

	// go through the word and find the beginning of non space character
	while( word_end_pos != word_start_pos && string[word_end_pos-1] == ' ' ) word_end_pos--;
	while( word_start_pos != word_end_pos && string[word_start_pos] == ' ') word_start_pos++;

	// now we have our range subtract to get new word length and allocate on the heap
	int wordlen = word_end_pos - word_start_pos;

	char * new_string = (char *)malloc( (wordlen+1)*sizeof(char));

	// copy the words over
	for(int i=word_start_pos; i < word_end_pos; i++){
		new_string[i - word_start_pos] = string[i];
	}
	// add the null terminator
	new_string[wordlen] = '\0';

	// free the old string
	if (free_str) free(string);

	// return new string
	return new_string;
}

struct csv_table * parse_file_to_csv_table_exp(FILE * csv_file, char delim, int strip_spaces, int discard_empty_cells){
	int bufflen = 10;
	char buffer[10];

	char last_char, second_last_char;


	int curline_finished = TRUE;
	int prev_line_finished = TRUE;

	struct csv_table *table = new_csv_table();
	
	struct csv_row *cur_row,  *sum_row;

	sum_row = NULL;

	struct csv_cell *sum_row_last_cell, *cur_row_first_cell;

	int merged_cells, error_occured;

	while ( !feof(csv_file) ){

		memset(buffer, 0, bufflen*sizeof(char));


		// reads characters into buffer
		// stops when it reaches newline, end of file or read bufflen-1 characters
		// places null terminator at position it stopped at
		// source: https://www.ibm.com/docs/en/i/7.4?topic=functions-fgets-read-string	
		fgets(buffer, bufflen, csv_file);

		if ( ferror(csv_file) ){
			// returns TRUE if an error occured while reading the CSV file
			// printf("An error occured reading the csv file!\n");
			error_occured = TRUE;
			break;
		}

		if ( buffer[0] != '\0' ){

			//printf("---------------------------------->\n");

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

			curline_finished = ( second_last_char == '\0' || second_last_char == '\n' || feof(csv_file));


			if ( prev_line_finished && curline_finished ){
				// previous line finished and this line finished
				// can just parse the row with parameters directly since entire line is in buffer
				sum_row = parse_line_to_csv_row(buffer, bufflen, delim, strip_spaces, discard_empty_cells);
				cur_row = NULL;
			} else {
				// we have to treat it as a current row
				// the buffer does not contain entire line
				// parsing it raw and applying parameter requirements afterwards
				cur_row = parse_line_to_csv_row(buffer, bufflen, delim, FALSE, FALSE);

				// printf("Current Row:\n");
				// print_csv_row(cur_row);
				// printf("Current Final Row:\n");
				// print_csv_row(sum_row);
				// if (sum_row == NULL ) printf("\n");
				// printf("\n");

				merged_cells = FALSE;

				if ( sum_row == NULL ){
					sum_row = new_csv_row();

				} else if ( sum_row->cell_count > 0 && cur_row->cell_count > 0 ){
					// merge the last entry of sum_row with first entry of the cur_row
					merged_cells = TRUE;
					sum_row_last_cell = get_cell_ptr_in_csv_row(sum_row, sum_row->cell_count-1);
					cur_row_first_cell = get_cell_ptr_in_csv_row(cur_row, 0);

					// printf("Merging: \"%s\" + \"%s\"\n\n", sum_row_last_cell->str, cur_row_first_cell->str);
					
					struct csv_cell * sum_row_new_last_cell = new_csv_cell();

					// get the new string size
					int sum_row_last_str_len = strlen(sum_row_last_cell->str);
					int cur_row_first_str_len =  strlen(cur_row_first_cell->str);
					int new_str_len = sum_row_last_str_len + cur_row_first_str_len;

					// allocate the string amount in the heap
					sum_row_new_last_cell->str = (char *)malloc(new_str_len + 1);

					// copy over the characters into the new string
					for(int i=0; i < new_str_len; i++){
						if ( i < sum_row_last_str_len ){
							sum_row_new_last_cell->str[i] = sum_row_last_cell->str[i];
						} else {
							sum_row_new_last_cell->str[i] = cur_row_first_cell->str[ i - sum_row_last_str_len ];
						}
					}
					sum_row_new_last_cell->str[new_str_len] = '\0';

					// delete the old final cell from the table and append the new one
					unmap_cell_in_csv_row(sum_row, sum_row_last_cell);
					free_csv_cell(sum_row_last_cell);
					map_cell_into_csv_row(sum_row, sum_row_new_last_cell);

					// apply ignore empty cells or strip spaces rule
					if ( strip_spaces ){
						// ignoring spaces so strip the string
						sum_row_new_last_cell->str = malloc_stripped_str(sum_row_new_last_cell->str, TRUE);
					}

					if ( curline_finished && discard_empty_cells && strcmp(sum_row_new_last_cell->str, "") == 0 ){
						delete_cell_from_csv_row(sum_row, sum_row->cell_count-1);
					}

				}

				// now we add the new values
				// we apply empty cells check here
				struct csv_cell * cur_cell = cur_row->cell_list_head;
				if (merged_cells){
					// if we merged the cells, then starting cell will be the one after
					cur_cell = cur_row->cell_list_head->next;
				}

				while ( cur_cell != NULL ){

					// strip the current cell if required
					if ( strip_spaces ){
						cur_cell->str = malloc_stripped_str(cur_cell->str, TRUE);
					}

					// add cell if we are not ignoring empty cells
					// or the current line is not finished and the  current cell is the tail cell (it might be merged in the next iteration)
					// or we are ignoring them and this cell is not empty
					if ( !discard_empty_cells || (!curline_finished && cur_cell==cur_row->cell_list_tail) || (discard_empty_cells && strcmp(cur_cell->str, "")!=0 )){
						// printf("Adding \"%s\"\n", cur_cell->str);
						add_cell_clone_to_csv_row( sum_row, cur_cell);
					}

					cur_cell = cur_cell->next;
				}

				// printf("\nNew Final Row:\n");
				// print_csv_row(sum_row);
			}

			if ( curline_finished ) {
				// sum_row has all the parsed entries for the current line, add it to the table and then continue
				map_row_into_csv_table(table, sum_row);
				sum_row = NULL;
			}

			if (cur_row != NULL) free_csv_row(cur_row);

			prev_line_finished = curline_finished;

			// printf("---------------------------------->\n");
		}

	}

	if ( error_occured ){
		free_csv_table(table);
		table = NULL;
	}

	return table;
}

int main(){

	char * filename = "addresses.csv";
	FILE * csv_file = fopen(filename, "r");
	if ( csv_file == NULL ) {
		printf("Could not open CSV file (%s)!!!\n", filename);
		exit(1);
	}

	struct csv_table * table = parse_file_to_csv_table_exp(csv_file, ',', FALSE, FALSE);
	printf("\n");
	for(struct csv_row * cur_row = table->row_list_head; has_next_row(table, cur_row); cur_row=cur_row->next){
		struct csv_cell * cur_cell = cur_row->cell_list_head;
		while ( cur_cell != cur_row->cell_list_tail){
			printf("%s||",cur_cell->str);
			cur_cell = cur_cell->next;
		}
		if ( cur_row->cell_list_tail != NULL ) printf("%s", cur_row->cell_list_tail->str);
		printf("\n");
	}

	return 0;
}