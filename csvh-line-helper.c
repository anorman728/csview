#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "csv.h"

#include "csvh-line-helper.h"

// I'm not at all happy with the terminology here because it comes off as
// very confusing.  The actual organization isn't hard once you understand
// the terms, though

// Constants defining output condition types (how we're restricting what lines
// to output).

// No restrictions-- Put out all lines.
#define COND_TYPE__NONE         0
// Finished.  Don't put out any more lines at all.
#define COND_TYPE__DONE         1
// Line interval.
#define COND_TYPE__LINE         2

// END output condition types.

// Forward declarations for static functions.

static char condLine();

static char strIsInt(char *inputStr);

static int *condLineBounds();

// END forward declarations.

/**
 * Array of strings defining output conditions.
 */
static char **conds;

/**
 * Condition index.  Start one below first value so move into that value.
 */
static int condInd = -1;

/**
 * The type of line conditions we're using (i.e., intervals, ranges, etc.).
 *
 * This can change while traversing the file, i.e., a line interval can
 * have a single line or it can have an actual interval., but in the
 * current design it will never switch "type families", i.e., switching
 * from a line interval type to a value range type.
 *
 * One exception is that anything other than "none" type can switch to
 * "done" type.
 *
 * Also, I added the dark type to counterbalance the psychic type.
 */
static int condType = COND_TYPE__NONE;

/**
 * The current line number.  Starts at zero, but the first non-header row
 * is the first line.
 */
static int lineNum = 0;

/**
 * Initialize format with line ranges.
 *
 * @param   lines
 */
char csvh_line_helper_init_lines(char *lines)
{
    condType = COND_TYPE__LINE;
    conds = parse_csv(lines, ',');
    if (conds == NULL) {
        return CSVH_LINE_HELPER__INVALID_INPUT;
    }

    return CSVH_LINE_HELPER__OK;
}

/**
 * Determine if should skip the current line.  Returns "OK" (don't skip),
 * "Skip" (skip) and "Done" (nothing left to print), according to constants
 * defined in csvh-line-helper.h.
 *
 * What's passed is the unparsed line.  It's important that it be the full
 * line from the input source, not just what's going to be in the output in
 * case the condition depends on a column that's not in the output.
 *
 * @param   unparsedLine
 */
char csvh_line_helper_should_skip(char *unparsedLine)
{
    lineNum++;
    switch (condType) {
        case COND_TYPE__NONE:
            return CSVH_LINE_HELPER__OK;
        case COND_TYPE__DONE:
            return CSVH_LINE_HELPER__DONE;
        case COND_TYPE__LINE:
            return condLine();
    }

    // TODO: Parse line, when get to conditions that will need it.

    return 0; // This is impossible, but otherwise the compiler will say
    // reached end of non-void function.
}

/**
 * Close out all open variables, etc.
 */
char csvh_line_helper_close()
{
    free_csv_line(conds);

    return CSVH_LINE_HELPER__OK;
}


// Static functions below this line.

/**
 * Handle line conditions.
 */
static char condLine()
{
    static int lower = 0;
    static int upper = 0;

    if (lower == 0) {
        // This means need to get the next segment.

        condInd++;

        // Check if we're done with reading the conditions.
        if (conds[condInd] == NULL) {
            condType = COND_TYPE__DONE;
            return CSVH_LINE_HELPER__DONE;
        }

        int *bounds = condLineBounds();
        lower = bounds[0];
        upper = bounds[1];
        free(bounds);

        if (lower == 0) {
            return CSVH_LINE_HELPER__INVALID_INPUT;
        }
    }

    if (lineNum == upper) {
        lower = 0;
        upper = 0;
        return CSVH_LINE_HELPER__OK;
    }

    return (lineNum < lower) ? CSVH_LINE_HELPER__SKIP : CSVH_LINE_HELPER__OK;
}

/**
 * Check that a string is an integer.
 *
 * @param   inputStr
 */
static char strIsInt(char *inputStr)
{
    for (int i = 0; inputStr[i] != '\0'; i++) {
        if (!isdigit(inputStr[i])) {
            return 0;
        }
    }

    return 1;
}

/**
 * Get the next upper and lower bound for line conditions, as an array of
 * two ints.
 *
 * Returns value on heap!
 */
static int *condLineBounds()
{
    // Check if there's a hyphen anywhere in the condition.
    int hyphen = -1;
    char dumChar = 0;
    for (;(dumChar = conds[condInd][++hyphen]) != '-' && dumChar != '\0';) {}
    char isRange = (dumChar == '-');

    char *lowerStr;
    char *upperStr;

    if (isRange) {
        conds[condInd][hyphen] = '\0'; // Turning these into two different strings.

        lowerStr = conds[condInd];
        upperStr = conds[condInd] + hyphen + 1;
    } else {
        lowerStr = conds[condInd];
        upperStr = lowerStr;
    }

    int *bounds = malloc(sizeof(int) * 2);

    // Check that the strings are integers.
    if (!strIsInt(lowerStr) || !strIsInt(upperStr)) {
        bounds[0] = 0;
        bounds[1] = 0;
        return bounds;
    }

    bounds[0] = atoi(lowerStr);
    bounds[1] = atoi(upperStr);

    return bounds;
}
