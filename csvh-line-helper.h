#ifndef csvh_line_helper_h
#define csvh_line_helper_h

// Constants

#define CSVH_LINE_HELPER__OK            0
#define CSVH_LINE_HELPER__SKIP          1
#define CSVH_LINE_HELPER__DONE          2
#define CSVH_LINE_HELPER__INVALID_INPUT 3

char csvh_line_helper_init_lines(char *lines);

char csvh_line_helper_should_skip(char *unparsedLine);

char csvh_line_helper_close();

#endif
