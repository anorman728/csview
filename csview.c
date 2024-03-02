#include <stdlib.h>
#include <stdio.h>

#include "csv-handler.h"

// Internal-use-only macro.
#define RETURN_ERR_IF_APP(EXPR) \
    if ((rc = EXPR)) {\
        printError(rc);\
        return rc;\
    }

// START forward declarations for helper functions.

void printError(char rc);

// END forward declarations for helper functions.

int main(int argc, char **argv)
{
	char *outputLine = NULL;
    char *borderLine = NULL;
    char *borderPadd = NULL;
    char rc = 0;

    //csv_handler_set_width(15); // TODO: Make option.
    //csv_handler_set_has_headers(0); // TODO: Make option.
    //csv_handler_set_delim(',');// TODO: Make option.

    // START Normal format.
    RETURN_ERR_IF_APP(csv_handler_read_next_line())
    RETURN_ERR_IF_APP(csv_handler_set_headers_from_line())
    //csv_handler_set_selected_fields(/* TODO */);

    // Print header.
    csv_handler_output_line_padding(&borderPadd);
    csv_handler_border_line(&borderLine);
    csv_handler_output_line(&outputLine);

    printf("%s", borderPadd);
    printf("%s\n", borderLine);

    printf("%s", borderPadd);
    printf("%s\n", outputLine);

    printf("%s", borderPadd);
    printf("%s\n", borderLine);

    // TODO: Restrictions here.

    while ((rc = csv_handler_read_next_line()) == CSV_HANDLER__OK) {
        RETURN_ERR_IF_APP(csv_handler_output_line_number(&outputLine))
        printf("%s", outputLine);
        RETURN_ERR_IF_APP(csv_handler_output_line(&outputLine))
        printf("%s\n", outputLine);
    }

    if (rc != CSV_HANDLER__DONE) {
        printError(rc);
        return rc;
    }

    printf("%s", borderPadd);
    printf("%s\n", borderLine);

    // END Normal format.


    free(outputLine);
    free(borderLine);
    free(borderPadd);
}

/**
 * Print error from CSV Handler.
 *
 * This code organization is definitely less than ideal.  The error string
 * should come from the module itself.  But I don't want to add that right now.
 *
 * @param   rc
 */
void printError(char rc)
{
    // "Internal errors" means that there's something wrong with this main
    // module itself.
    switch (rc) {
        case CSV_HANDLER__FILE_NOT_FOUND:
            printf("Error: File not found.");
            break;
        case CSV_HANDLER__DONE:
            printf("Interal error: Done.");
            // In this case, the error is probably that handling the response
            // incorrectly.
            break;
        case CSV_HANDLER__LINE_IS_NULL:
            printf("Internal error: Line is null.");
            break;
        case CSV_HANDLER__ALREADY_SET:
            printf("Internal error:  Already set.");
            break;
        case CSV_HANDLER__OUT_OF_MEMORY:
            printf("Error: Out of memory.");
            break;
        case CSV_HANDLER__HEADERS_NOT_SET:
            printf("Internal error: Headers not set.");
            break;
        case CSV_HANDLER__INVALID_INPUT:
            printf("Error: Invalid input in passed parameters.");
            break;
        case CSV_HANDLER__HEADER_NOT_FOUND:
            printf("Error: A specified header in passed parameters does not exist.");
            break;
        case CSV_HANDLER__UNKNOWN_ERROR:
            printf("Unknown error!");
            break;
    }
    printf("\n");
}
