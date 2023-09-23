#include <stdlib.h>
#include <stdio.h>

#include "csv-handler.h"

void testfunc(char **line);

int main()
{
    char *outputLine = NULL;

    //while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
    //    csv_handler_read_next_line();
    //    csv_handler_output_line(&outputLine);
    //    printf("%s\n", outputLine);
    //}

    csv_handler_read_next_line();
    csv_handler_output_line(&outputLine);
    printf("%s\n", outputLine);
    csv_handler_read_next_line();
    csv_handler_output_line(&outputLine);
    printf("%s\n", outputLine);

    free(outputLine);
}
