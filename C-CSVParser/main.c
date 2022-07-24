#include "csvparser.h"


int main(){

	char * filename = "addresses.csv";
	FILE * csv_file = fopen(filename, "r");
	if ( csv_file == NULL ) {
		printf("Could not open CSV file (%s)!!!\n", filename);
		exit(1);
	}
	
	struct csv_table * table;
	
	table = parse_file_to_csv_table(csv_file, ',', FALSE, FALSE);
	char * a_str = "iffy1,iffy2,iffy3";
	//table = parse_string_to_csv_table_exp(a_str, ',', TRUE, TRUE);



	if ( table == NULL ){
		printf("NO value!\n");
		return 0;
	}
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