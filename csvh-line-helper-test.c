#include <stdlib.h>
#include <stdio.h>

#include "csvh-line-helper.h"

int main()
{
    csvh_line_helper_init_lines("4,7-11,13,15-17");
    int line = 0; // zero is header in this case.

    for (int i = 1; i < 21; i++) {
        printf("line: %d, res: %d\n", line++, csvh_line_helper_should_skip(""));
    }
}
