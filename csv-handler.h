#ifndef csvhandler_h
#define csvhandler_h

// Constants

#define CSV_HANDLER__OK                 0
#define CSV_HANDLER__FILE_NOT_FOUND     1
#define CSV_HANDLER__EOF                2
#define CSV_HANDLER__LINE_IS_NULL       3
#define CSV_HANDLER__ALREADY_SET        4
#define CSV_HANDLER__OUT_OF_MEMORY      5
#define CSV_HANDLER__HEADERS_NOT_SET    6

// Functions for typical output and vertical output.
char csv_handler_read_next_line();

char csv_handler_set_headers_from_line();

char csv_handler_line(char **wholeLine);

char csv_handler_output_line(char **outputLine);

char csv_handler_border_line(char **outputLine);

char csv_handler_output_vertical_entry(char **outputEntry);

char csv_handler_vertical_border_line(char **outputLine);

// Functions for transposed output.

char csv_handler_initialize_transpose();

char csv_handler_transposed_line(char **outputLine);

char csv_handler_transposed_border_line(char **outputLine);

// Other functions.
void csv_handler_set_width(int newWidth);

char csv_handler_close();

#endif
