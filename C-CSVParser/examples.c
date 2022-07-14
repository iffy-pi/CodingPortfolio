#include "csvparser.h"

int main(){

	char * expected[] = {
		"<iffy1>",
		"<iffy1>,<iffy2,iffy3>",
		"<iffy1>,<>,<iffy2,iffy3>",
		"<iffy1>,<iffy2,iffy3>,<>,<>",
		NULL
	};

	char * strings[] = {
		"iffy1", // 0
		"iffy1\n", // 0
		"iffy1\r\n", // 0 
		"iffy1\niffy2,iffy3", // 1
		"iffy1\niffy2,iffy3\n", // 1
		"iffy1\niffy2,iffy3\r\n", // 1
		"iffy1\n\niffy2,iffy3\n", // 2
		"iffy1\n,,\niffy2,iffy3", // 2 *
		"iffy1\niffy2,iffy3,,,\n\n\n", // 3
		NULL
	};

	// index of expected output in expected[]
	int expectedmappings[] = {
		0, 0, 0, 1, 1, 1, 2, 2, 3
	};

	char * expected[] = {
		"<iffy1,iffy2>",
		"<iffy1,,iffy2>",
		"<,iffy1,>",
		"<,,iffy1,,>",
		"<iffy1,,>,<iffy2>",
		"<iffy1,,>,<,iffy2>",
		"<>",
		"<,,,>",
		NULL
	};

	char * strings[] = {
		"iffy1 ,iffy2", //0
		"iffy1, ,iffy2", // 1
		",iffy1 , ", // 2
		",iffy1 , \n", // 2
		",,iffy1 , ,", //3
		"iffy1,  \niffy2", //4
		"iffy1,,\n,iffy2", //5
		"", //6
		" , , ", // 7
		NULL
	};

	// index of expected output in expected[]
	int expectedmappings[] = {
		0, 1, 2, 2, 3, 4, 5, 6, 7
	};

	struct csv_table * table;
	for(int i=0; strings[i] != NULL; i++ ){
		printf("String %d: \"", i);
		special_print(strings[i]);
		printf("\"\n");
		printf("Expected: %s\n", expected[ expectedmappings[i] ]);
		table = parse_string_to_csv_table_exp(strings[i], strlen(strings[i]), ',', FALSE, TRUE);
		// table = parse_string_to_csv_table(strings[i], strlen(strings[i]), ',', TRUE);
		print_csv_table(table);
		printf("\n");
	}


	return 0;
}