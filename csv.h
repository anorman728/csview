#ifndef CSV_DOT_H_INCLUDE_GUARD
#define CSV_DOT_H_INCLUDE_GUARD

char **parse_csv( const char *line, char del );
void free_csv_line( char **parsed );
int count_fields(const char *line, char del);

#endif
