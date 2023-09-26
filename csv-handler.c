#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv.h"
#include "csv-handler.h"

/**
 * Delimiter.
 */
static char delim = ',';

/**
 * Current complete line.
 */
static char *line = NULL;

/**
 * Headers as array of strings.
 */
static char **headers = NULL;

/**
 * Count of fields in source file.
 */
static int countFields = -1;

/**
 * Count of fields displayed in output.
 */
static int printedFieldCount = -1;


/**
 * Width of cells to output.
 */
static int width = 15;

/**
 * Array of array of strings, terminated by a NULL at the end.  Good grief;
 * this will be annoying.  Holds the entirety of the input file except what is
 * filtered out.  Currently only used for displaying transposed output.
 */
static char ***entireInput = NULL;


// START forward declarations for static functions.

static char getParsedLine(char ***parsedLine);

static char appendBoxedValue(char **outputLine, char *newValue);

// END forward declarations.

/**
 * Read next line into memory.
 */
char csv_handler_read_next_line()
{
    if (line != NULL) {
        free(line);
        line = NULL;
    }

    line = (char *) malloc(sizeof(char));

    if (line == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    line[0] = '\0'; // Empty string for now because we don't know how long it
    // will be.

    int buffsize = 255;
    char buff[buffsize];
    while (1) {
        if (fgets(buff, buffsize, stdin) == NULL) {
            // Note that this should happen *after* the final line has already
            // been read into memory.
            return CSV_HANDLER__EOF;
        }

        line = (char *) realloc(
            line,
            sizeof(char) * (strlen(line) + strlen(buff) + 1) // +1 for null terminator
        );

        if (line == NULL) {
            return CSV_HANDLER__OUT_OF_MEMORY;
        }

        strcat(line, buff);

        size_t lst = strlen(line) - 1;
        if (line[lst] == '\n' && ((countFields = count_fields(line, delim)) != -1)) {
            // If count_fields is -1, then that means the line is not parseable
            // as a CSV line, which probably means that the file has a field
            // with a line break in it, meaning we have to include both of the
            // *file*'s lines as part of the same logical CSV line.  Example:

            // field one,field two,"field with
            // line break", field four

            // From the CSV perspective, this is one line, but if we don't check
            // that the line we just found is parseable when we reach the first
            // newline, we'll get an unparseable string and csv.c will return
            // null.

            line[lst] = '\0'; // Removing newline, but not reallocing.
            printedFieldCount = countFields; // By default, print everything until told otherwise.
            break;
        }
    }

    return CSV_HANDLER__OK;
}

/**
 * Set the headers from the line in memory.
 */
char csv_handler_set_headers_from_line()
{
    if (line == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }
    if (headers != NULL) {
        return CSV_HANDLER__ALREADY_SET;
    }

    headers = parse_csv(line, delim);
    // Not using getParsedLine because don't want to filter anything out for
    // headers.

    return CSV_HANDLER__OK;
}

/**
 * Get the line in CSV format.
 *
 * @param   wholeLine   Pointer to string.
 */
char csv_handler_line(char **wholeLine)
{
    if (line == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }
    if (*wholeLine != NULL) {
        free(wholeLine);
        wholeLine = NULL;
    }
    *wholeLine = (char *) realloc(*wholeLine, sizeof(char) * (strlen(line) + 1));

    if (*wholeLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    strcpy(*wholeLine, line);

    return CSV_HANDLER__OK;
}

/**
 * Get line to print out to stdout.
 *
 * @param   outputLine
 */
char csv_handler_output_line(char **outputLine)
{
    if (*outputLine != NULL) {
        free(*outputLine);
        *outputLine = NULL;
    }

    if (line == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }
    char **parsedLine = NULL;
    getParsedLine(&parsedLine);

    *outputLine = (char *) malloc(sizeof(char) * 2);

    if (*outputLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    (*outputLine)[0] = '|'; // Opening brace.
    (*outputLine)[1] = '\0';

    char rc = 0;
    for (int i = 0; parsedLine[i] != NULL; i++) {
        if ((rc = appendBoxedValue(outputLine, parsedLine[i])) != CSV_HANDLER__OK) {
            return rc;
        }
    }

    free_csv_line(parsedLine);

    return CSV_HANDLER__OK;
}

/**
 * Get border to print out.
 *
 * @param   outputLine
 */
char csv_handler_border_line(char **outputLine)
{
    if (countFields == -1) {
        return CSV_HANDLER__LINE_IS_NULL; // Not sure what else to call this.
    }

    if (*outputLine != NULL) {
        free(*outputLine);
        *outputLine = NULL;
    }

    int lineLen = (width + 1) * printedFieldCount + 1;
    // I almost wanted to name this "linLen" because then it would be pronounced
    // "len-len" and that would be funny.
    // (width + 1) is the width of every field plus its left brace.
    // + 2 is one for rightmost brace.
    // Null terminator is *not* included here, because want to match what the
    // result of strlen would be.

    *outputLine = (char *) malloc(sizeof(char) * (lineLen + 1));

    if (*outputLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    (*outputLine)[0] = '+';

    for (int i = 1; i < lineLen - 1; i++) {
        (*outputLine)[i] = '-';
    }
    (*outputLine)[lineLen - 1] = '+';
    (*outputLine)[lineLen] = '\0';

    return CSV_HANDLER__OK;
}

/**
 * Get *vertical* entry to print to stdout.
 *
 * This is not a single line, so behavior is inconsistent.  It's the entirety of
 * an entry, line breaks and all.
 *
 * @param   outputEntry
 */
char csv_handler_output_vertical_entry(char **outputEntry)
{
    if (*outputEntry != NULL) {
        free(*outputEntry);
        *outputEntry = NULL;
    }

    if (line == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }

    if (headers == NULL) {
        return CSV_HANDLER__HEADERS_NOT_SET;
    }

    char **parsedLine = NULL;
    getParsedLine(&parsedLine);

    *outputEntry = malloc(sizeof(char));
    (*outputEntry)[0] = '\0';

    for (int i = 0; parsedLine[i] != NULL; i++) {
        *outputEntry = realloc(
            *outputEntry,
            strlen(*outputEntry)
            + strlen(parsedLine[i])
            + strlen(headers[i])
            + (i == 0 ? 3 : 4)
        );
        // +1 for null term, +1 for line break, +2 for ": "
        if (i != 0) {
            strcat(*outputEntry, "\n");
        }
        strcat(*outputEntry, headers[i]);
        strcat(*outputEntry, ": ");
        strcat(*outputEntry, parsedLine[i]);
    }

    free_csv_line(parsedLine);

    return CSV_HANDLER__OK;
}

/**
 * Get border line for vertical entry.
 *
 * @param   outputLine
 */
char csv_handler_vertical_border_line(char **outputLine)
{
    if (*outputLine != NULL) {
        free(*outputLine);
        *outputLine = NULL;
    }

    *outputLine = malloc(sizeof(char) * width + 1); // Re-use width, so can
    // change it if want to.

    if (*outputLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    for (int i = 0; i < width; i++) {
        (*outputLine)[i] = '*';
    }

    (*outputLine)[width] = '\0';

    return CSV_HANDLER__OK;
}

/**
 * Read the entirety of the file (that's desired) into memory so that can output
 * it as transposed.
 */
char csv_handler_initialize_transpose()
{
    if (entireInput != NULL) {
        return CSV_HANDLER__ALREADY_SET;
    }

    char **parsedLine = NULL;
    int arrLen = 0;

    while (csv_handler_read_next_line() == CSV_HANDLER__OK) {
        getParsedLine(&parsedLine);
        arrLen++;
        entireInput = (char ***) realloc(entireInput, sizeof(char ***) * arrLen);

        if (entireInput == NULL) {
            return CSV_HANDLER__OUT_OF_MEMORY;
        }

        entireInput[arrLen - 1] = (char **) malloc(sizeof(char **) * (countFields + 1));

        if (entireInput[arrLen - 1] == NULL) {
            return CSV_HANDLER__OUT_OF_MEMORY;
        }

        int i = 0;
        for (; parsedLine[i] != NULL; i++) {
            entireInput[arrLen - 1][i] = (char *) malloc(sizeof(char) * strlen(parsedLine[i]) + 1);

            if (entireInput[arrLen - 1] == NULL) {
                return CSV_HANDLER__OUT_OF_MEMORY;
            }

            strcpy(entireInput[arrLen - 1][i], parsedLine[i]);
        }

        entireInput[arrLen - 1][i] = NULL;

        free_csv_line(parsedLine);
    }

    entireInput = (char ***) realloc(entireInput, sizeof(char **) * (arrLen + 1));

    if (entireInput == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    entireInput[arrLen] = NULL;

    return CSV_HANDLER__OK;
}

/**
 * Get transposed line to print to stdout.
 *
 * @param   outputLine
 */
char csv_handler_transposed_line(char **outputLine)
{
    // This one's very different.  Everything is already stored in memory in
    // entireInput, so go down the line from there.

    static int ind = 0;

    if (*outputLine != NULL) {
        free(*outputLine);
        *outputLine = NULL;
    }

    if (entireInput == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }

    if (entireInput[0][ind] == NULL) {
        return CSV_HANDLER__EOF;
    }

    *outputLine = (char *) malloc(sizeof(char) * 2);

    if (*outputLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    strcpy(*outputLine, "[");

    char rc = 0;
    for (int i = 0; entireInput[i] != NULL; i++) {
        if ((rc = appendBoxedValue(outputLine, entireInput[i][ind])) != CSV_HANDLER__OK) {
            return rc;
        }

        if (i == 0) {
            int j = strlen(*outputLine) - 3;
            for (; (*outputLine)[j] == ' ' && j > 0;j--) {}
            (*outputLine)[j + 1] = ']';
        }
    }

    ind++;

    return CSV_HANDLER__OK;
}

/**
 * Get transposed border line to print to stdout.
 *
 * @param   outputLine
 */
char csv_handler_transposed_border_line(char **outputLine)
{
    if (*outputLine != NULL) {
        free(*outputLine);
        *outputLine = NULL;
    }

    if (entireInput == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }

    int len = 0;

    // Count up the number of fields in the array.
    for (;entireInput[++len] != NULL;){};

    len = (len * (width + 1)) + 1;
    // len is number of elements in first row, so multiply it by field width
    // (plus one for |), plus one at the end for final +

    *outputLine = (char *) malloc(sizeof(char) * (len + 1));

    if (*outputLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    for (int i = 0; i < len - 1; i++) {
        (*outputLine)[i] = '-';
    }

    (*outputLine)[len - 1] = '+';
    (*outputLine)[len] = '\0';

    return CSV_HANDLER__OK;
}

/**
 * Change the width.
 *
 * @param   newWidth
 */
void csv_handler_set_width(int newWidth)
{
    width = newWidth;
}

/**
 * Close out everything.
 */
char csv_handler_close()
{
    if (headers != NULL) {
        free_csv_line(headers);
    }
    if (line != NULL) {
        free(line);
    }
    if (entireInput != NULL) {
        for (int i = 0; entireInput[i] != NULL; i++) {
            free_csv_line(entireInput[i]);
            entireInput[i] = NULL;
        }
        free(entireInput);
    }

    return CSV_HANDLER__OK;
}


// Static functions below this line.

/**
 * Set passed pointer to array of strings to parsed CSV from line.
 *
 * @param   parsedLine
 */
static char getParsedLine(char ***parsedLine)
{
    // TODO: Later, we'll add a filter so we only use specified columns.
    // I'm not sure why I said "we".  It's all me.  You're not doing anything,
    // you good-for-nothing bum.

    *parsedLine = parse_csv(line, delim);

    return CSV_HANDLER__OK;
}

/**
 * Append string with new boxed value for printing to output.
 *
 * @param   outputLine
 */
static char appendBoxedValue(char **outputLine, char *newValue)
{
    int initialLen = strlen(*outputLine);
    *outputLine = (char *) realloc(*outputLine, sizeof(char) * (initialLen + 2 + width));
    // +2 is one for '|' and one for null terminator.

    if (*outputLine == NULL) {
        return CSV_HANDLER__OUT_OF_MEMORY;
    }

    // Only want to concat part of the string, so need to do some funky stuff.

    int contentLength = strlen(newValue);
    if (contentLength > width) {
        contentLength = width;
    }
    int fillerLength = width - contentLength; // Will be zero if content is larger than width.

    for (int j = 0; j < contentLength; j++) {
        if (newValue[j] == '\n') {
            // Don't display newline.  It's confusing in this context.
            (*outputLine)[initialLen + j] = ' ';
        } else {
            (*outputLine)[initialLen + j] = newValue[j];
        }
        // Note that this starts with j = 0, so overwriting the null terminator.
    }
    for (int j = 0; j < fillerLength; j++) {
        (*outputLine)[initialLen + contentLength + j] = ' ';
    }

    (*outputLine)[initialLen + width] = '|'; // Next brace.
    (*outputLine)[initialLen + width + 1] = '\0'; // Putting back in the null terminator.

    return CSV_HANDLER__OK;
}
