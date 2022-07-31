#include "csvparser.h"


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