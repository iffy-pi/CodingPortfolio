# C CSV Parser Documentation
## Overview
The included files provide functions to parse CSV (or other character separated) files and/or strings into an accessible structure in the C programming language. The underlying parsed structures make use of doubly linked lists and are allocated on the heap of the running program.

The reason I wrote this CSV parser for C is its ease of access. While there are several other CSV parsers available, I could not find anyone that was as convenient as calling a function on a string or filename parameter and the parsed CSV structure being returned. Most of the parsers I found were more complicated than what I needed.

This CSV parser parses the input file or string and returns the parsed structure allocated on the heap.

This documentation is intended to provide information on how to use the provided CSV parser.

The parser testing process and possible limitations are discussed in Testing and Evaluation respectively.

[Link text Here](https://link-url-here.org)

**For a PDF version of documentation: see [doc/CSVParserDoc.pdf](/C-CSVParser/doc/CSVParserDoc.pdf)**

## Basic Tutorial
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

![Structure of CSV Diagram](/C-CSVParser/doc/csv_structure_diagram.png?raw=true "CSV Structure Diagram")