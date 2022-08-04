# C CSV Parser Documentation
# Overview
The included files provide functions to parse CSV (or other character separated) files and/or strings into an accessible structure in the C programming language. The underlying parsed structures make use of doubly linked lists and are allocated on the heap of the running program.

The reason I wrote this CSV parser for C is its ease of access. While there are several other CSV parsers available, I could not find anyone that was as convenient as calling a function on a string or filename parameter and the parsed CSV structure being returned. Most of the parsers I found were more complicated than what I needed.

This CSV parser parses the input file or string and returns the parsed structure allocated on the heap.

This documentation is intended to provide information on how to use the provided CSV parser.

The parser testing process and possible limitations are discussed in Testing and Evaluation respectively.

[Link text Here](https://link-url-here.org)

**For a PDF version of documentation: see [doc/CSVParserDoc.pdf](/C-CSVParser/doc/CSVParserDoc.pdf)**

# Basic Tutorial
Once the header file and the implementation file are included in your project, CSV files and strings can be parsed using the following functions:
```c
struct csv_table * parse_string_to_csv_table(char * string, char delim, char quot_char, int strip_spaces, int discard_empty_cells);

struct csv_table * open_and_parse_file_to_csv_table(char * filename, char delim, char quot_char, int strip_spaces, int discard_empty_cells);

```

The functions take the string to parse (string) or the address of the file to read and parse (filename). The other function parameters control the parser behaviour:

| Parameter             | Description |
| :---                  |    :---     |
| `delim`               | This is the character used to separate values. For CSVs it is `,` but it can be set to any symbol. |
| `quot_char`           | Character used for making quotes, e.g. `"` or `'` |
| `strip_spaces`        | Boolean flag. If true (1), parsed CSV values will be stripped of leading and trailing spaces. |
| `discard_empty_cells` | Boolean flag. If true (1), any value that is an empty string (`""`) will not be added to the parsed structure. |

*Note: If the delimiter is a space character* (`' '`), *strip_spaces and discard_empty_cells will be overridden to 0 (false) and 1 (true) respectively.*

The return value is a pointer to the structure used to store the parsed CSV values, a `struct csv_table`.

`struct csv_table` is the highest level of the parsed CSV structure. It is made up of a doubly linked list of `struct csv_row`, which are in turn made up of a doubly linked list of `struct csv_cell`. The `struct csv_cell` contains the appropriate parsed string value in its `str` field.

![Structure of CSV Diagram](/C-CSVParser/doc/csv_structure_diagram.png?raw=true "CSV Structure Diagram")

The structure is designed to mirror the table layout in the unparsed CSV. The length field is the number of CSV rows or CSV Cells in the list of a CSV table or CSV row respectively.

Functions are provided to get a row/cell at a specific row and column index (see Get CSV Structures and Coordinates).

The parsed CSV structure is allocated on the heap and therefore must be freed manually. The header files include free functions for each of the CSV structures to make sure all allocated memory is freed (see Free CSV Structures).

# In-depth Tutorial
## Create CSV Structures
CSV structures can be created in code using the *new_csv* functions.

### Create CSV Cells
A new CSV cell can be created using the following:

```c
// Create CSV cell with no string value
struct csv_cell *c1 = new_csv_cell();

// Create a cell with a string value
struct csv_cell *c2 = new_csv_cell_from_str("ACell2");
```

CSV cells can have their `str` field populated/overwritten with a string value using `populate_csv_cell_str`. If the `str` field already has a value, the function will free the current value and write in the string passed in.

```c
// Populate c1 with string
populate_csv_cell_str(c1, "Cell1");

// Overwrite value in c2
populate_csv_cell_str(c2, "Cell2");
```

### Create CSV Rows
New CSV rows can be created with the `new_csv_row`.
```c
struct csv_row *r1 = new_csv_row();
```

### Create CSV Tables
New CSV tables can be created with `new_csv_table`.
```c
struct csv_table *t1 = new_csv_table();
```

## Clone CSV Structures
The clone functions are provided to clone the contents of the passed in pointer to the CSV structure.
```c
struct csv_cell * clone_csv_cell(struct csv_cell * cell);
struct csv_row * clone_csv_row(struct csv_row * row);
struct csv_table * clone_csv_table(struct csv_table * table);
```

The clone function allocates a new CSV structure on the heap and then copies the contents of the source CSV structure. Cloned rows and tables will have their child cells cloned as well.

## Free CSV Structures
CSV Structures are allocated on the heap and therefore must be freed appropriately.

### Free CSV Cells
An allocated CSV cell can be de-allocated using `free_csv_cell`.
```c
free_csv_cell(c1);
```

### Free CSV Rows
An allocated CSV row can be de-allocated using `free_csv_row`.
```c
free_csv_row(r1);
```

When a CSV row is freed, all cells in its cell list are also freed (using `free_csv_cell`). Therefore, if you are working with a cell mapped to a row, be sure to clone the cell contents before freeing the row.

### Free CSV Tables
An allocated CSV table can be de-allocated using `free_csv_table`.
```c
free_csv_table(t1);
```

When a CSV table is freed, all rows in its row list are also freed (using `free_csv_row`).

## Print CSV Cell Structures
The contents of csv structures can be printed using the print functions.

### Print CSV Cells
CSV cells can be printed using `print_csv_cell`.
```c
print_csv_cell(c1);
```

The function prints the string contained in the cell's str field surrounded by quotes.
```
"Cell1"
```

### Print CSV Rows
CSV rows can be printed using `print_csv_row`.
```c
print_csv_row(r1);
```

The function prints all the cells in the row's cell list in the order that they appear. The cell list is surrounded by square brackets and a new line character is also printed.
```
["Cell1", "Cell2"]
```

There is also `pretty_print_csv_row`, which prints CSV cell contents with tabulation and newlines.
```
[
        "Cell1",
        "Cell2"
]
```

### Print CSV Tables
A basic print can be done using the `print_csv_table` function.
```c
print_csv_table(t1);
```

This prints the set of CSV rows with their cell contents on one line. A newline character is also printed.
```
[["Cell1", "Cell2"], ["Cell3", "Cell4"]]
```

The function `pretty_print_csv_table` organizes the rows as they are printed. Each row is printed on one line with tabulation.
```
[
        ["Cell1", "Cell2"],
        ["Cell3", "Cell4"]
]
```

There is also `super_pretty_print_csv_table`, which performs a pretty print to the CSV cells in each CSV row in addition to the row organization from `pretty_print_csv_table`.
```
[
        [
                "Cell1",
                "Cell2"
        ],

        [
                "Cell3",
                "Cell4"
        ]
]
```


## Get CSV Structures and Coordinates
### Get CSV Structure at Specified Coordinate
#### Get Structure Pointers
The following functions are provided to get a pointer to the CSV structure at a specified coordinate:
```c
struct csv_cell * get_cell_ptr_in_csv_row(struct csv_row *row, int index);
struct csv_row * get_row_ptr_in_csv_table(struct csv_table *table, int index);
struct csv_cell * get_cell_ptr_in_csv_table(struct csv_table *table, int rowindx, int colindx);
```
The functions take the parent row/table and the index for the cell/row.

Use `get_cell_ptr_in_csv_row` to get the pointer to CSV cells at a specific index.
```c
// r1 = ["Cell1", "Cell2"]

// Getting "Cell1"
struct csv_cell *cell0 = get_cell_ptr_in_csv_row(r1, 0);

// Getting "Cell2"
struct csv_cell *cell1 = get_cell_ptr_in_csv_row(r1, 1);
```

Use `get_row_ptr_in_csv_table` to get the pointer to the CSV row at the specific index.
```c
// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4"]]

// Getting ["Cell1", "Cell2"]
struct csv_row *row0 = get_row_ptr_in_csv_table(t1, 0);
```

To get a specific cell in a CSV table, `get_cell_ptr_in_csv_table` can be used.
```c
// t1 = [["Cell1", "Cell2"], ["Cell3", "Cell4"]]

// Getting "Cell1"
struct csv_cell *cell0 = get_cell_ptr_in_csv_table(t1, 0, 0);

// Getting "Cell3"
struct csv_cell *cell2 = get_cell_ptr_in_csv_table(t1, 1, 0);
```
If the specified index is out of range of the parent CSV structure or the pointer to the parent CSV structure, a NULL pointer is returned.

Note: **The pointer to the CSV structure mapped in the parent structure is returned, meaning any changes made to the pointer's value will affect the parent structure. To get a deep copy, refer to Get Structure Clones.**

#### Get Structure Clones
The following functions are provided to get the clone of the CSV structure at the specified index.
```c
struct csv_cell * get_cell_clone_in_csv_row(struct csv_row *row, int index);
struct csv_row * get_row_clone_in_csv_table(struct csv_table *table, int index);
struct csv_cell * get_cell_clone_in_csv_table(struct csv_table *table, int rowindx, int colindx);
```

In this case, the pointer returned points to a clone of the CSV structure at the specified coordinates. This means that it is separate from the parent structure it was taken from.

### Get CSV Cell for a String
The functions below are provided to get the pointer to the CSV cell in the row/table whose str field is the same string as the string parameter.
```c
struct csv_cell * get_cell_for_str_in_csv_row(struct csv_row *row, char * string);
struct csv_cell * get_cell_for_str_in_csv_table(struct csv_table *table, char * string);
```
The pointer to the CSV cell returned will be the first match found in the parent structure.

### Get Coordinates for CSV Structure
For a passed in CSV structure, the provided functions return the coordinates to the CSV structure in the passed in parent CSV structure which contains the same contents as the reference CSV structure.
```c
int get_cell_coord_in_csv_row(struct csv_row *row, struct csv_cell *cell);
int get_row_coord_in_csv_table(struct csv_table *table, struct csv_row *row);
```
For example, consider the CSV row r1 which is` ["Cell1", "Cell2"]`. We can get the index of the cell that contains the string `"Cell1"` using the following.
```c
struct csv_cell *search_cell = new_csv_cell_from_str("Cell1");
int index = get_cell_coord_in_csv_row(r1, search_cell); // index is 0
```
Note that the reference CSV structure does not have to be a part of the parent CSV structure (row/table) since the search is performed using structure content comparison.

If there is no match found, -1 is returned.

`get_cell_coord_in_csv_table` is provided to get the coordinates of a CSV cell structure in the parent table with the same contents as the reference CSV structure.
```c
int get_cell_coord_in_csv_table(struct csv_table *table, struct csv_cell *cell, int *rowindx, int *colindx);
```

Since a cell coordinate in a table has both a row and column, the function also takes pointers to store the resulting row and column index.

If a match is found, the function will populate the index pointers and return 0. Otherwise, it will return -1.

### Get Coordinates for a String
The below functions have the same behaviour as the functions discussed in Get Coordinates for CSV Structure but instead take a string parameter instead of a reference CSV structure.
```c
int get_str_coord_in_csv_row(struct csv_row *row, char *string);
int get_str_coord_in_csv_table(struct csv_table *table, char *string, int *rowindx, int *colindx);
```

In this case, the returned coordinates are to the first CSV cell that contains the same string as the reference string.
