#include "csvparser.h"


char * malloc_strip_quotes_and_spaces_exp(char  * string, int len, int strip_quotes, int strip_spaces, int free_string){
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

int main(){

	//char *string = "\"ha \"\"ha\"\" ha\"";

	char * string =  "\"ha \"\"\"\"ha\"\"\"\" ha\"";

	char *stripped = malloc_strip_quotes_and_spaces_exp(string, strlen(string), TRUE, TRUE, FALSE);

	printf("String: %s\n", string);
	printf("Expected: ha \"ha\" ha\n");
	printf("Stripped: %s\n", stripped);

	free(stripped);

	return 0;
}