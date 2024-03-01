#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "csv.h"

#include "csvh-line-helper.h"

// This is a helper module for csv-handler.c.

// This module does not read the input file from disk.  It accepts one line
// at a time, if that, as an argument.

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
// Value range.
#define COND_TYPE__RANGE        3
// Value equals
#define COND_TYPE__EQUALS       4

// END output condition types.

// Forward declarations for static functions.

static char condLine();

static char condRange(char **parsedLine);

static char condEquals(char **parsedLine);

static char strIsInt(char *inputStr);

static void condLineBounds(int *bounds, int condInd);

static char condRangeCompare(char *val, char *range);

static int stringHasChar(char *testStr, char inChar);

// END forward declarations.

/**
 * Array of strings defining output conditions.
 */
static char **conds;

/**
 * Critical index, i.e., the index determining the column that we use for
 * incoming records to determine if they match our restrictions.  (In other
 * words, if our restriction is "Column 5 must be equal to 'zebra', then the
 * critical index is 5.)  Not applicable for all condition types.
 */
static int critInd = -1;

/**
 * The type of line conditions we're using (i.e., intervals, ranges, etc.).
 *
 * This can change while traversing the file, i.e., a line interval can
 * have a single line or it can have an actual interval., but in the
 * current design it will never switch "type families", i.e., switching
 * from a line interval type to a value range type.
 *
 * (Note: I don't think the above is still 100% relevant?  There's only one
 * "line" type after a refactor.)
 *
 * One exception is that anything other than "none" type can switch to
 * "done" type.
 *
 * Also, I added the dark type to counterbalance the psychic type.
 */
static int condType = COND_TYPE__NONE;

/**
 * The current line number.  Starts at zero, but the first non-header row
 * is the first line.  (Meaning that, header or not, the first row of actual
 * data is row 1.)
 */
static int lineNum = 0;

/**
 * Yes if file has a header, no if either it doesn't have one or it's
 * already been passed.
 */
static char hasHeader = 1;

/**
 * Initialize with line ranges restrictions.
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
 * Initialize with value ranges restrictions.
 *
 * "ranges" is string like "3-4,6,9-13".  Can also be rational numbers.
 *
 * @param   critIndInput
 * @param   ranges
 */
char csvh_line_helper_init_ranges(int critIndInput, char *ranges)
{
    condType = COND_TYPE__RANGE;

    critInd = critIndInput;

    conds = parse_csv(ranges, ',');
    if (conds == NULL) {
        return CSVH_LINE_HELPER__INVALID_INPUT;
    }

    return CSVH_LINE_HELPER__OK;
}

/**
 * Initialize with value equals restrictions.
 *
 * "equals" is string like "bob,sue,abdul alhazred".  Must equal exactly to
 * match.
 *
 * @param   critIndInput
 * @param   ranges
 */
char csvh_line_helper_init_equals(int critIndInput, char *equals)
{
    condType = COND_TYPE__EQUALS;

    critInd = critIndInput;

    conds = parse_csv(equals, ',');
    if (conds == NULL) {
        return CSVH_LINE_HELPER__INVALID_INPUT;
    }

    return CSVH_LINE_HELPER__OK;
}

/**
 * Get the current line number.
 */
int csvh_line_helper_get_line_num()
{
    return lineNum;
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
    if (hasHeader) {
        // Always want to get the header.
        // Note that if there's no actual header, csv-handler.c creates one and
        // provides it.  The variable name is a little misleading, because it's
        // basically always true for the first line.
        hasHeader = 0;
        return CSVH_LINE_HELPER__OK;
    }

    lineNum++;

    // Don't waste time parsing lines for these.
    switch (condType) {
        case COND_TYPE__NONE:
            return CSVH_LINE_HELPER__OK;
        case COND_TYPE__DONE:
            return CSVH_LINE_HELPER__DONE;
        case COND_TYPE__LINE:
            return condLine();
    }

    // Now parse the line, because it'll be used in the other condition checks.
    char **parsedLine = parse_csv(unparsedLine, ',');
    char res = CSVH_LINE_HELPER__INTERNAL_ERROR;
    // If return this, it means that there's some kind of foreign condition
    // type that's defined but never used.

    switch (condType) {
        case COND_TYPE__RANGE:
            res = condRange(parsedLine);
            break;
        case COND_TYPE__EQUALS:
            res = condEquals(parsedLine);
            break;
    }

    free_csv_line(parsedLine);
    parsedLine = NULL;
    return res;
}

/**
 * Close out all open variables, etc.
 */
char csvh_line_helper_close()
{
    if (conds != NULL) {
        free_csv_line(conds);
        conds = NULL;
    }

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
    static int condInd = -1; // The condition of the conds array that we're
    // currently using.  With lines condition we can just go straight through.
    // TODO: Figure out what that last sentence mean and better explain it.

    if (lower == 0) {
        // This means need to get the next segment.

        condInd++;

        // Check if we're done with reading the conditions.
        if (conds[condInd] == NULL) {
            condType = COND_TYPE__DONE;
            return CSVH_LINE_HELPER__DONE;
        }

        int bounds[2];
        condLineBounds(bounds, condInd);
        lower = bounds[0];
        upper = bounds[1];

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
 * Get the next upper and lower bound for line conditions, as an array of
 * two ints.
 *
 * Sets values as zero if input is invalid.
 *
 * @param   bounds
 * @param   condInd
 */
static void condLineBounds(int *bounds, int condInd)
{
    int isRange = stringHasChar(conds[condInd], '-');

    char *lowerStr;
    char *upperStr;

    if (isRange) {
        int breakInd = isRange - 1; // To make coding a little easier.
        conds[condInd][breakInd] = '\0';
        // Turning these into two different strings.

        lowerStr = conds[condInd];
        upperStr = conds[condInd] + breakInd + 1;
    } else {
        lowerStr = conds[condInd];
        upperStr = lowerStr;
    }

    // Check that the strings are integers.
    if (!strIsInt(lowerStr) || !strIsInt(upperStr)) {
        bounds[0] = 0;
        bounds[1] = 0;
        return;
    }

    bounds[0] = atoi(lowerStr);
    bounds[1] = atoi(upperStr);
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
 * Handle range conditions.
 *
 * @param   parsedLine
 */
static char condRange(char **parsedLine)
{
    // Loop through each condition and see if it applies.  Return OK on the
    // *first* one where it's true.

    char *val = parsedLine[critInd];
    char *condDum;
    // Need to make a dummy string because going to mutate it later, and don't
    // want to change the original.

    char rc;
    for (int i = 0; conds[i] != NULL; i++) {
        condDum = malloc(sizeof(char) * strlen(conds[i]) + 1);
        strcpy(condDum, conds[i]);
        if ((rc = condRangeCompare(val, condDum)) != CSVH_LINE_HELPER__SKIP) {
            free(condDum);
            return rc;
        }
        free(condDum);
        // If this returns "skip", don't actually skip yet.  It just means
        // that *this* range in the conds array does not apply.
    }


    return CSVH_LINE_HELPER__SKIP;
}

/**
 * Check value against the conditions passed.
 *
 * @param   val
 * @param   range
 */
static char condRangeCompare(char *val, char *range)
{
    int isRange = stringHasChar(range, '-');
    char isDouble = stringHasChar(range, '.') || stringHasChar(val, '.');
    char *eptr; // Seems to be needed to convert string to double.  Won't
    // be used.

    // Don't check both parts for double in case of range, b/c going to
    // upconvert them anyway.

    if (isRange) {
        // See if within given range.

        int breakInd = isRange - 1; // To make coding a little easier.

        range[breakInd] = '\0';
        // Turning these into two different strings.
        char *lowerStr = range;
        char *upperStr = range + breakInd + 1;

        if (isDouble) {
            double valDbl = strtod(val, &eptr);
            double lowerDbl = strtod(lowerStr, &eptr);

            if (valDbl < lowerDbl) {
                return CSVH_LINE_HELPER__SKIP;
            }

            double upperDbl = strtod(upperStr, &eptr);

            if (valDbl > upperDbl) {
                return CSVH_LINE_HELPER__SKIP;
            }
        } else {
            int valInt = atoi(val);
            int lowerInt = atoi(lowerStr);

            if (valInt < lowerInt) {
                return CSVH_LINE_HELPER__SKIP;
            }

            int upperInt = atoi(upperStr);

            if (valInt > upperInt) {
                return CSVH_LINE_HELPER__SKIP;
            }
        }

        // I feel like a function-like macro could simplify this, but I
        // don't want to do it right now.  Definitely do it if I end up
        // doing this pattern another time.
    } else {
        // Compare single value, but still needs to be as number.

        if (isDouble) {
            // If it's a double and not a range, it will almost certainly
            // not match, but still need to put this here.
            if (strtod(val, &eptr) != strtod(range, &eptr)) {
                return CSVH_LINE_HELPER__SKIP;
            }
        } else {
            if (atoi(val) != atoi(range)) {
                return CSVH_LINE_HELPER__SKIP;
            }
        }
    }

    return CSVH_LINE_HELPER__OK;
}

/**
 * Handle equals condition.
 *
 *
 * @param   parsedLine
 */
static char condEquals(char **parsedLine)
{
    // Loop through each condition and see if it applies.  Return OK on the
    // *first* one where it's true.

    char *val = parsedLine[critInd];

    for (int i = 0; conds[i] != NULL; i++) {
        if (strcmp(conds[i],val) == 0) {
            return CSVH_LINE_HELPER__OK;
        }
    }

    return CSVH_LINE_HELPER__SKIP;
}

/**
 * Check if there's a hyphen in a string representing a condition.
 *
 * Returns one greater than the index (so that it's false instead of -1 if
 * it's just missing).
 *
 * @param   testStr
 * @param   inChar
 */
static int stringHasChar(char *testStr, char inChar)
{
    int i = -1;
    for (;testStr[++i] != inChar && testStr[i] != '\0';) {} // Because I can.
    if (testStr[i] == '\0') { i = -1; }
    return i + 1;
}
