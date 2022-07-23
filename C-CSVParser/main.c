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

char * malloc_strip_quotes_and_spaces(char  * string, int len, int strip_quotes, int strip_spaces, int free_string){
	// strips string of leading and trailing spaces
	// returns a pointer to the stripped string, allocated using malloc
	// option to free old string as parameter

	if ( string == NULL ) return NULL;

	int new_word_start_pos = 0;
	int new_word_end_pos = len;

	// remove the quotes if present
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
	for(int i=new_word_start_pos; i < new_word_end_pos; i++){
		new_string[i - new_word_start_pos] = string[i];
	}
	// add the null terminator
	new_string[new_wordlen] = '\0';

	if (free_string) free(string);

	return new_string;

}

// struct csv_table * parse_file_to_csv_table_exp(FILE * csv_file, char delim, int strip_spaces, int discard_empty_cells){
// 	int bufflen = 10;
// 	char buffer[10];

// 	char last_char, second_last_char;


// 	int curline_finished = TRUE;
// 	int prev_line_finished = TRUE;

// 	struct csv_table *table = new_csv_table();
	
// 	struct csv_row *cur_row,  *sum_row;

// 	sum_row = NULL;

// 	struct csv_cell *sum_row_last_cell, *cur_row_first_cell;

// 	int merged_cells, error_occured;

// 	while ( !feof(csv_file) ){

// 		memset(buffer, 0, bufflen*sizeof(char));


// 		// reads characters into buffer
// 		// stops when it reaches newline, end of file or read bufflen-1 characters
// 		// places null terminator at position it stopped at
// 		// source: https://www.ibm.com/docs/en/i/7.4?topic=functions-fgets-read-string	
// 		fgets(buffer, bufflen, csv_file);

// 		if ( ferror(csv_file) ){
// 			// returns TRUE if an error occured while reading the CSV file
// 			// printf("An error occured reading the csv file!\n");
// 			error_occured = TRUE;
// 			break;
// 		}

// 		if ( buffer[0] != '\0' ){

// 			//printf("---------------------------------->\n");

// 			// read an entire line
// 			// last char is null and second last char is null (entire line read and buffer not full)
// 			// or
// 			// last char is null and second last char is newline (entire line is read and buffer full)
// 			// or
// 			// we have reached eof (no more characters to read from the file, so this line is an entire line)
// 			// for lines that are bigger than the buffer, none of these will be true

// 			second_last_char = buffer[bufflen-2];

// 			// last char will always be null terminator either because of memset or fgets
// 			// curline_finished used to store if the current line was read entirely

// 			curline_finished = ( second_last_char == '\0' || second_last_char == '\n' || feof(csv_file));


// 			if ( prev_line_finished && curline_finished ){
// 				// previous line finished and this line finished
// 				// can just parse the row with parameters directly since entire line is in buffer
// 				sum_row = parse_line_to_csv_row(buffer, bufflen, delim, strip_spaces, discard_empty_cells);
// 				cur_row = NULL;
// 			} else {
// 				// we have to treat it as a current row
// 				// the buffer does not contain entire line
// 				// parsing it raw and applying parameter requirements afterwards
// 				cur_row = parse_line_to_csv_row(buffer, bufflen, delim, FALSE, FALSE);

// 				// printf("Current Row:\n");
// 				// print_csv_row(cur_row);
// 				// printf("Current Final Row:\n");
// 				// print_csv_row(sum_row);
// 				// if (sum_row == NULL ) printf("\n");
// 				// printf("\n");

// 				merged_cells = FALSE;

// 				if ( sum_row == NULL ){
// 					sum_row = new_csv_row();

				// } else if ( sum_row->cell_count > 0 && cur_row->cell_count > 0 ){
				// 	// merge the last entry of sum_row with first entry of the cur_row
				// 	merged_cells = TRUE;
				// 	sum_row_last_cell = get_cell_ptr_in_csv_row(sum_row, sum_row->cell_count-1);
				// 	cur_row_first_cell = get_cell_ptr_in_csv_row(cur_row, 0);

				// 	// printf("Merging: \"%s\" + \"%s\"\n\n", sum_row_last_cell->str, cur_row_first_cell->str);
					
				// 	struct csv_cell * sum_row_new_last_cell = new_csv_cell();

				// 	// get the new string size
				// 	int sum_row_last_str_len = strlen(sum_row_last_cell->str);
				// 	int cur_row_first_str_len =  strlen(cur_row_first_cell->str);
				// 	int new_str_len = sum_row_last_str_len + cur_row_first_str_len;

				// 	// allocate the string amount in the heap
				// 	sum_row_new_last_cell->str = (char *)malloc(new_str_len + 1);

				// 	// copy over the characters into the new string
				// 	for(int i=0; i < new_str_len; i++){
				// 		if ( i < sum_row_last_str_len ){
				// 			sum_row_new_last_cell->str[i] = sum_row_last_cell->str[i];
				// 		} else {
				// 			sum_row_new_last_cell->str[i] = cur_row_first_cell->str[ i - sum_row_last_str_len ];
				// 		}
				// 	}
				// 	sum_row_new_last_cell->str[new_str_len] = '\0';

				// 	// delete the old final cell from the table and append the new one
				// 	unmap_cell_in_csv_row(sum_row, sum_row_last_cell);
				// 	free_csv_cell(sum_row_last_cell);
				// 	map_cell_into_csv_row(sum_row, sum_row_new_last_cell);

				// 	// apply ignore empty cells or strip spaces rule
				// 	if ( strip_spaces ){
				// 		// ignoring spaces so strip the string
				// 		sum_row_new_last_cell->str = malloc_stripped_str(sum_row_new_last_cell->str, TRUE);
				// 	}

				// 	if ( curline_finished && discard_empty_cells && strcmp(sum_row_new_last_cell->str, "") == 0 ){
				// 		delete_cell_from_csv_row(sum_row, sum_row->cell_count-1);
				// 	}

// 				}

// 				// now we add the new values
// 				// we apply empty cells check here
// 				struct csv_cell * cur_cell = cur_row->cell_list_head;
// 				if (merged_cells){
// 					// if we merged the cells, then starting cell will be the one after
// 					cur_cell = cur_row->cell_list_head->next;
// 				}

// 				while ( cur_cell != NULL ){

// 					// strip the current cell if required
// 					if ( strip_spaces ){
// 						cur_cell->str = malloc_stripped_str(cur_cell->str, TRUE);
// 					}

// 					// add cell if we are not ignoring empty cells
// 					// or the current line is not finished and the  current cell is the tail cell (it might be merged in the next iteration)
// 					// or we are ignoring them and this cell is not empty
// 					if ( !discard_empty_cells || (!curline_finished && cur_cell==cur_row->cell_list_tail) || (discard_empty_cells && strcmp(cur_cell->str, "")!=0 )){
// 						// printf("Adding \"%s\"\n", cur_cell->str);
// 						add_cell_clone_to_csv_row( sum_row, cur_cell);
// 					}

// 					cur_cell = cur_cell->next;
// 				}

// 				// printf("\nNew Final Row:\n");
// 				// print_csv_row(sum_row);
// 			}

// 			if ( curline_finished ) {
// 				// sum_row has all the parsed entries for the current line, add it to the table and then continue
// 				map_row_into_csv_table(table, sum_row);
// 				sum_row = NULL;
// 			}

// 			if (cur_row != NULL) free_csv_row(cur_row);

// 			prev_line_finished = curline_finished;

// 			// printf("---------------------------------->\n");
// 		}

// 	}

// 	if ( error_occured ){
// 		free_csv_table(table);
// 		table = NULL;
// 	}

// 	return table;
// }

struct csv_table * parse_file_to_csv_table_exp2(FILE * csv_file, char delim, int strip_spaces, int discard_empty_cells){

	// buffer for fgets
	int bufflen = 10;
	char buffer[10];

	char second_last_char;

	// used csv structures
	struct csv_table *table = new_csv_table();
	struct csv_row *cur_row = NULL;

	// values used for merging cells
	struct csv_cell *cur_cell, *last_cell, *replacement_cell;
	int last_word_len, combined_str_len, merging_cells;

	int error_occured = FALSE;

	// character position items
	int cur_pos, cur_word_start_pos, cur_word_end_pos;
	char * cur_word;
	int cur_word_len;
	int cur_cell_str_len;
	int cur_delim_pos;

	int has_reached_delim, has_reached_space, has_reached_newl, has_reached_crnewl, has_reached_only_cr;
	int is_at_a_delim, has_reached_eos, has_reached_eof, eos_char_offset;

	int quot_count = 0;

	int first_word_for_buffer_parsed;

	int linecnt = 0;

	int entire_cur_line_in_buffer, entire_prev_line_in_buffer;
	int append_this_cell;


	while ( !feof(csv_file) ){
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

		if ( buffer[0] != '\0' ){

			printf("---------------------------------->\n");

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
				has_reached_eof = ( has_reached_eos && feof(csv_file) );

				if ( buffer[cur_pos] == '"') quot_count = (quot_count+1) % 2;

				// if quot_count is odd, then we are in the middle of a quot, so spaces and delim should be ignored
				if ( quot_count != 0 ){
					if (has_reached_delim) {
						//printf("Cur pos: %d, resetting delim!\n",cur_pos);
						has_reached_delim = FALSE;
					}
				}

				is_at_a_delim = ( has_reached_delim || has_reached_crnewl || has_reached_newl || has_reached_only_cr || has_reached_eos );

				if ( is_at_a_delim ){
					// we have reached the end of a word,
					//empty the previous word if there was one
					// if ( cur_word != NULL ) {
					// 	free(cur_word);
					// 	cur_word = NULL;
					// }

					cur_delim_pos = cur_pos;
					cur_word_end_pos = cur_delim_pos;

					cur_word_len = cur_word_end_pos - cur_word_start_pos;

					// the cur word just points to where it starts in the buffer
					// we have the current word len so we know where it ends
					cur_word = buffer + cur_word_start_pos;

					printf("$cur_word = \"");
					int q = 0;
					while ( q < cur_word_len-1 ){
						printf("%c", cur_word[q]);
						q++;
					}
					if (cur_word_len == 0) printf("\"\n");
					else printf("%c\"\n", cur_word[q]);

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

						printf("Merging: \"%s\" + $cur_word\n", last_cell->str);


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
						printf("Removed cell \"%s\"\n", last_cell->str);
						unmap_cell_in_csv_row(cur_row, last_cell);
						free_csv_cell(last_cell);


						// //if the entire line is in the buffer then the replacement cell can be stripped as required
						// if ( entire_cur_line_in_buffer ){
						// 	printf("Stripping quotes and spaces\n");
						// 	replacement_cell->str = malloc_strip_quotes_and_spaces(replacement_cell->str, combined_str_len, TRUE, strip_spaces, TRUE);

						// 	if ( !discard_empty_cells || (discard_empty_cells && strlen(replacement_cell->str) > 0) ){
						// 		map_cell_into_csv_row(cur_row, replacement_cell);
						// 	} else {
						// 		free_csv_cell(replacement_cell);
						// 		replacement_cell = NULL;
						// 	}

						// }


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
					
					// if this is the last line and the line is not complete
					// then we do not strip or discard since the line can be merged in the next iteration
					if ( !entire_cur_line_in_buffer && buffer[cur_word_end_pos] == '\0'){
						if ( !merging_cells ) mallocstrcpy( &cur_cell->str, cur_word, cur_word_len);
					} else {
						// not the last word in buffer without entire line
						// or is the last word but the buffer has entire line
						// strip spaces and quotes

						printf("Stripping quotes and spaces\n");

						if ( merging_cells){
							cur_cell->str = malloc_strip_quotes_and_spaces(cur_cell->str, combined_str_len, TRUE, strip_spaces, TRUE);
						} else{
							cur_cell->str = malloc_strip_quotes_and_spaces(cur_word, cur_word_len, TRUE, strip_spaces, FALSE);
						}

						// do not append this cell if we are discarding empties and it has empty string
						append_this_cell = ( !discard_empty_cells || (discard_empty_cells && strlen(cur_cell->str) > 0) );
					}

					if ( append_this_cell ){
						if ( merging_cells) printf("Appending merged cell: \"%s\"\n", cur_cell->str);
						else printf("Appending cell: \"%s\"\n", cur_cell->str);
						map_cell_into_csv_row(cur_row, cur_cell);
					} else {
						printf("Cell \"%s\" discarded!\n", cur_cell->str);
					}

					first_word_for_buffer_parsed = TRUE;

					// next word starts after this word ends
					cur_word_start_pos = cur_delim_pos+1;
					if (has_reached_crnewl) cur_word_start_pos++;

				}

				// it has reached the end of the line
				// fgets only gets to newline so we know we have reached end of the buffer
				if ( has_reached_newl || has_reached_only_cr || has_reached_crnewl || has_reached_eof ){
					// we have gotten to the end of a line
					// append the currernt row
					printf("===============================\n");
					printf("Final Row:\n");
					print_csv_row(cur_row);
					printf("===============================\n");
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


			entire_prev_line_in_buffer = entire_cur_line_in_buffer;

			printf("----------------------------------->\n");
		}
	}

	if ( error_occured ){
		printf("An error occured!\n");
		free_csv_table(table);
		return NULL;
	}

	printf("\n");
	for(struct csv_row * cr = table->row_list_head; has_next_row(table, cr); cr=cr->next){
		struct csv_cell * cc = cr->cell_list_head;
		while ( cc != cr->cell_list_tail){
			printf("%s||",cc->str);
			cc = cc->next;
		}
		if ( cr->cell_list_tail != NULL ) printf("%s", cr->cell_list_tail->str);
		printf("\n");
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

	struct csv_table * table = parse_file_to_csv_table_exp2(csv_file, ',', TRUE, TRUE);
	// printf("\n");
	// for(struct csv_row * cur_row = table->row_list_head; has_next_row(table, cur_row); cur_row=cur_row->next){
	// 	struct csv_cell * cur_cell = cur_row->cell_list_head;
	// 	while ( cur_cell != cur_row->cell_list_tail){
	// 		printf("%s||",cur_cell->str);
	// 		cur_cell = cur_cell->next;
	// 	}
	// 	if ( cur_row->cell_list_tail != NULL ) printf("%s", cur_row->cell_list_tail->str);
	// 	printf("\n");
	// }

	return 0;
}