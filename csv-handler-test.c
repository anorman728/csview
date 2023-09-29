#include <stdlib.h>
#include <stdio.h>

#include "csv-handler.h"

void testfunc(char **line);

int main()
{
    char *outputLine = NULL;
    char *borderLine = NULL;

    csv_handler_set_width(17);
    csv_handler_restrict_by_lines("1-2,4");

    // Print header, with border.
    csv_handler_read_next_line(); // Not bothering checking RCs right now.
    csv_handler_set_headers_from_line(); // Needed for setting fields.
    //csv_handler_set_selected_fields("HeadA,HeadC");
    csv_handler_border_line(&borderLine);
    printf("%s\n", borderLine);
    csv_handler_output_line(&outputLine);
    printf("%s\n", outputLine);
    printf("%s\n", borderLine);

    while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
        csv_handler_output_line(&outputLine);
        printf("%s\n", outputLine);
    }

    printf("%s\n", borderLine);

    // Transposed test.
    //csv_handler_initialize_transpose("HeadA,HeadB");

    //csv_handler_transposed_border_line(&borderLine);
    //printf("%s\n", borderLine);
    //while (csv_handler_transposed_line(&outputLine) == CSV_HANDLER__OK) {
    //    printf("%s\n", outputLine);
    //}
    //printf("%s\n", borderLine);

    // Vertical test.
    //csv_handler_vertical_border_line(&borderLine);
    //csv_handler_read_next_line();
    //csv_handler_set_headers_from_line(); // Needed for vertical output.
    //csv_handler_set_selected_fields("HeadA,HeadC");

    //while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
    //    printf("%s\n", borderLine);
    //    csv_handler_output_vertical_entry(&outputLine);
    //    printf("%s\n", outputLine);
    //}
    //printf("%s\n", borderLine);

    // Print headers test.
    //csv_handler_read_next_line();
    //csv_handler_set_headers_from_line();
    //while (csv_handler_output_headers(&outputLine) == CSV_HANDLER__OK) {
    //    printf("%s\n", outputLine);
    //}

    // Print raw lines.
    //csv_handler_read_next_line(); // Not bothering checking RCs right now.
    //csv_handler_set_headers_from_line(); // Needed for setting fields.
    //csv_handler_set_selected_fields("HeadA,HeadC");
    //csv_handler_raw_line(&outputLine);
    //printf("%s\n", outputLine);

    //while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
    //    csv_handler_raw_line(&outputLine);
    //    printf("%s\n", outputLine);
    //}


    free(outputLine);
    free(borderLine);
    csv_handler_close();
}
