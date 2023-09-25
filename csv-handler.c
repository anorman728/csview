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
    (*outputLine)[0] = '|'; // Opening brace.
    (*outputLine)[1] = '\0';

    for (int i = 0; parsedLine[i] != NULL; i++) {
        appendBoxedValue(outputLine, parsedLine[i]);
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
    (*outputLine)[0] = '+';

    for (int i = 1; i < lineLen - 1; i++) {
        (*outputLine)[i] = '-';
    }
    (*outputLine)[lineLen - 1] = '+';
    (*outputLine)[lineLen] = '\0';

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
        entireInput[arrLen - 1] = (char **) malloc(sizeof(char **) * (countFields + 1));

        int i = 0;
        for (; parsedLine[i] != NULL; i++) {
            entireInput[arrLen - 1][i] = (char *) malloc(sizeof(char) * strlen(parsedLine[i]) + 1);
            strcpy(entireInput[arrLen - 1][i], parsedLine[i]);
        }

        entireInput[arrLen - 1][i] = NULL;

        free_csv_line(parsedLine);
    }

    entireInput = (char ***) realloc(entireInput, sizeof(char **) * (arrLen + 1));
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

    *outputLine = (char *) malloc(sizeof(char));
    (*outputLine)[0] = '\0'; // Start as empty string.

    for (int i = 0; entireInput[i] != NULL; i++) {
        appendBoxedValue(outputLine, entireInput[i][ind]);

        if (i == 0) {
            //*outputLine = realloc(*outputLine, sizeof(char) * (strlen(*outputLine) + 2));
            //strcat(*outputLine, "|");
            (*outputLine)[strlen(*outputLine) - 1] = '{';
        }
    }

    (*outputLine)[strlen(*outputLine) - 1] = '}';

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

    len = (len * (width + 1) - 1);
    // len is number of elements in first row, so multiply it by field width
    // (plus one for |).  There is no opening | for transposed, and no closing
    // |, so need to subtract off one.  No null terminator to match strlen.

    *outputLine = (char *) malloc(sizeof(char) * (len + 1));

    for (int i = 0; i < len; i++) {
        (*outputLine)[i] = '-';
    }

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
