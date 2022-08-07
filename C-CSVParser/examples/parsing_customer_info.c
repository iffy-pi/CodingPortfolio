#include "..\release\csvparser.h"

int main(){
	/*/////////////////////////////////////////////*/
	/* Find the age of customer Julian Applebottom */
	/*/////////////////////////////////////////////*/

	char *customer_name = "Julian Applebottom";

	// open and parse the CSV file into the CSV table
	struct csv_table *customer_info = open_and_parse_file_to_csv_table("customer_info.csv", ',', '"', FALSE, FALSE);

	if ( customer_info == NULL || customer_info->length == 0){
		printf("No customer information parsed!\n");
		exit(1);
	}

	// first find the column for the customer age
	struct csv_row *customer_info_header = get_row_ptr_in_csv_table(customer_info, 0);

	int age_col_indx = get_str_coord_in_csv_row(customer_info_header, "Age");

	if ( age_col_indx == -1 ){
		printf("Could not find age header cell!\n");
		exit(1);
	}

	// we can find the cell with Julian's name and get the parent row
	struct csv_cell *customer_name_cell = get_cell_for_str_in_csv_table(customer_info, customer_name);

	if ( customer_name_cell == NULL ){
		printf("Could not find name cell for Julian!\n");
		exit(1);
	}
	struct csv_row *customer_info_row = customer_name_cell->parent;

	char *customer_age_str = get_str_from_csv_row( customer_info_row, age_col_indx);

	if ( customer_age_str == NULL ){
		printf("Could not find age string for customer!\n");
		exit(1);
	}

	int age = atoi(customer_age_str);


	printf("Age for '%s': %d\n", customer_name, age);



	return 0;
}