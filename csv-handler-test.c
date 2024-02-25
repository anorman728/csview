#include <stdlib.h>
#include <stdio.h>

#include "csv-handler.h"

// REMINDER: Need to pass stdin for this to work!

void testfunc(char **line);

int main()
{
    char *outputLine = NULL;
    char *borderLine = NULL;

    csv_handler_set_width(17);

    // Normal test.

    // Print header, with border.
    //csv_handler_set_has_headers(0); // Need to do this, if do it, before read_next_line.
    //csv_handler_set_delim('|');
    //csv_handler_read_next_line(); // Not bothering checking RCs right now.
    //csv_handler_set_headers_from_line(); // Needed for setting fields.
    //// Always do above two things *first*.
    //csv_handler_set_selected_fields("B,D");
    //csv_handler_border_line(&borderLine);
    //printf("%s\n", borderLine);
    //csv_handler_output_line(&outputLine);
    //printf("%s\n", outputLine);
    //printf("%s\n", borderLine);

    //csv_handler_restrict_by_lines("2-3,5");
    //// This specific restriction can technically be done before getting the
    //// headers, but since other restrictions can't, putting this here for
    //// consistency.

    ////csv_handler_restrict_by_ranges("3", "10-17,23");
    ////csv_handler_restrict_by_equals("3", "7,15");

    //while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
    //    csv_handler_output_line(&outputLine);
    //    printf("%s\n", outputLine);
    //}

    //printf("%s\n", borderLine);

    // Transposed test.
    //csv_handler_set_has_headers(0);
    //csv_handler_read_next_line();
    //csv_handler_set_headers_from_line();
    ////csv_handler_set_selected_fields("HeadA,HeadC,Range");
    //csv_handler_restrict_by_lines("2-3,5");
    ////csv_handler_restrict_by_ranges("Range", "5,7-9");
    //csv_handler_initialize_transpose();

    //csv_handler_transposed_border_line(&borderLine);
    //printf("%s\n", borderLine);
    //while (csv_handler_transposed_line(&outputLine) == CSV_HANDLER__OK) {
    //    printf("%s\n", outputLine);
    //}
    //printf("%s\n", borderLine);

    // Vertical test.
    //csv_handler_set_has_headers(0);
    //csv_handler_vertical_border_line(&borderLine);
    //csv_handler_read_next_line();
    //csv_handler_set_headers_from_line(); // Needed for vertical output.
    //csv_handler_set_selected_fields("B,C");
    //csv_handler_restrict_by_lines("1-2,4");

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
    csv_handler_set_delim('|');
    csv_handler_read_next_line(); // Not bothering checking RCs right now.
    csv_handler_set_headers_from_line(); // Needed for setting fields.
    csv_handler_set_selected_fields("B,D");
    csv_handler_restrict_by_lines("2-4");
    csv_handler_raw_line(&outputLine);
    printf("%s\n", outputLine);

    while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
        csv_handler_raw_line(&outputLine);
        printf("%s\n", outputLine);
    }


    free(outputLine);
    free(borderLine);
    csv_handler_close();
}
