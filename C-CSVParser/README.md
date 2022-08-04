![Alt text](/C-CSVParser/doc/csv_structure_diagram.png?raw=true "CSV Structure Diagram")

Table test

| **Use Case**         | **Big-O** | **Reasoning**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| -------------------- | --------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Parsing CSV          | O(n)      | There are two cases, in both of which the parser results in O(n). (Calculations are based on the behaviour of the base parser function parse\_fileptr\_or\_char\_array\_to\_csv\_table).

Case 1: No cell merges need to happen. Case for parsing string or fgets has entire CSV row in its buffer.

For a given word of length n, we traverse the buffer n times to find the word delimiter ⸫ O(n).

The word is copied from the buffer into the allocated cell:

O(1) for cell allocation

O(n) for word copy (copied character by character)

For one word: O(n) + O(n) + O(1) = O(2n) + O(1).

This scales to the other words in the buffer: O(2n) + O(1) ![](file:///C:/Users/omnic/AppData/Local/Temp/msohtmlclip1/01/clip_image002.png) O(n)

Case 2: Cell merges happen (fgets does not have entire CSV row in the buffer).

In the worst case where a word is split across two buffers from fgets, it will have to be merged. To perform this:

O(n) to traverse buffer and find delimiter of first part of the word

O(n) + O(1) to copy the first part of the word into an allocated cell

O(n) to traverse the new buffer and find delimiter and therefore second part of the word

O(n) + O(n) + O(1) to copy the first part of the word and the second part of the word to a new cell that has the combined string

O(1) to delete old cell from the table.

O(n) + O(n) to strip combined string of quotes and spaces

O(n +  n + 1 + n +  n + n + 1 +  1 +  n + n) = O(7n + 1) = O(7n) ![](file:///C:/Users/omnic/AppData/Local/Temp/msohtmlclip1/01/clip_image002.png) O(n) |
| Sequential Access    | O(n)      | Accessing the beginning and end of the list is O(1) since there is the list head and tail pointers.

Get CSV Structure functions are designed to start from the end closest to the specified index, therefore worst-case scenario will mean going through n/2 elements ⸫ O(n/2) ![](file:///C:/Users/omnic/AppData/Local/Temp/msohtmlclip1/01/clip_image002.png) O(n).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| Inserting Structures | O(n)      | Appending to the end of the list is O(1) since adjusting list\_tail and incrementing length is constant time.

The worst case for an insertion:

In a list of n CSV structures, we are inserting the structure at location n-1. This would mean traversing past the first n-1 elements ⸫ O(n-1).

Adjusting the list pointers is O(1).

Worst case: O(n-1) + O(1) ![](file:///C:/Users/omnic/AppData/Local/Temp/msohtmlclip1/01/clip_image002.png) O(n).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Checking for Content | O(n)      | No starting optimization can be made as content to search for can be placed anywhere in the list. Worst case is going through all list elements to find them therefore O(n).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |