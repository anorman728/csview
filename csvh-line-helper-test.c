#include <stdlib.h>
#include <stdio.h>

#include "csvh-line-helper.h"

int main()
{
    // Line intervals.
    //int res = csvh_line_helper_init_lines("4,7-11,13,15-17");
    //printf("res: %d\n", res);
    //int line = 0; // zero is header in this case.

    //for (int i = 1; i < 21; i++) {
    //    printf("line: %d, res: %d\n", line++, csvh_line_helper_should_skip(""));
    //}

    // Ranges.
    csvh_line_helper_init_ranges(2, "5-7,11.1-12.8, 15");

    printf("header, always print: should be 0: %d\n", csvh_line_helper_should_skip("blah"));
    printf("integer range 1: should be 1: %d\n", csvh_line_helper_should_skip("a,b,1"));
    printf("double range 2: should be 1: %d\n", csvh_line_helper_should_skip("a,b,4.9"));
    printf("integer range 3: should be 0: %d\n", csvh_line_helper_should_skip("a,b,5"));
    printf("double range 4: should be 0: %d\n", csvh_line_helper_should_skip("a,b,5.1"));
    printf("double range 5: should be 0: %d\n", csvh_line_helper_should_skip("a,b,6.999"));
    printf("integer range 6: should be 0: %d\n", csvh_line_helper_should_skip("a,b,7"));
    printf("double range 7: should be 1: %d\n", csvh_line_helper_should_skip("a,b,7.0001"));
    printf("integer range 8: should be 1: %d\n", csvh_line_helper_should_skip("a,b,8"));
    printf("double range 9: should be 1: %d\n", csvh_line_helper_should_skip("a,b,11"));
    printf("double range 10: should be 0: %d\n", csvh_line_helper_should_skip("a,b,11.1"));
    printf("double range 11: should be 0: %d\n", csvh_line_helper_should_skip("a,b,12"));
    printf("double range 12: should be 1: %d\n", csvh_line_helper_should_skip("a,b,13"));
    printf("integer range 13: should be 1: %d\n", csvh_line_helper_should_skip("a,b,14"));
    printf("integer range 14: should be 1: %d\n", csvh_line_helper_should_skip("a,b,16"));
    printf("integer range 15: should be 0: %d\n", csvh_line_helper_should_skip("a,b,15"));
    printf("integer range 15: should be 0, but might not be: %d\n", csvh_line_helper_should_skip("a,b,15.0"));

    // Don't bother testing a double that's not a range.
}
