#ifndef csvh_line_helper_h
#define csvh_line_helper_h

// Constants

#define CSVH_LINE_HELPER__OK                0
#define CSVH_LINE_HELPER__SKIP              1
#define CSVH_LINE_HELPER__DONE              2
#define CSVH_LINE_HELPER__INVALID_INPUT     3
#define CSVH_LINE_HELPER__INTERNAL_ERROR    4
// "Internal error" means it's an error inside of the module itself.

char csvh_line_helper_init_lines(char *lines);

char csvh_line_helper_init_ranges(int critIndInput, char *ranges);

char csvh_line_helper_init_equals(int critIndInput, char *equals);

int csvh_line_helper_get_line_num();

char csvh_line_helper_should_skip(char *unparsedLine);

char csvh_line_helper_close();

#endif
