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
 * Count of fields in file.
 */
static int countFields = -1;

/**
 * Width of cells to output.
 */
static int width = 15;


// START forward declarations for static functions.

// TO COME

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
        return CSV_HANDLER__ALREADY_SET_HEADER;
    }

    headers = parse_csv(line, delim);

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
 * Get the line en toto.
 *
 * @param   wholeLine   Pointer to string.
 */
//char csv_handler_line(char **wholeLine)
//{
//    if (line == NULL) {
//        return CSV_HANDLER__LINE_IS_NULL;
//    }
//    if (*wholeLine != NULL) {
//        free(wholeLine);
//        wholeLine = NULL;
//    }
//    *wholeLine = (char *) realloc(*wholeLine, sizeof(char) * (strlen(line) + 1));
//    strcpy(*wholeLine, line);
//
//    return CSV_HANDLER__OK;
//}
// No longer needed, so commented out to make the executable smaller.  Uncomment
// if ever needed in the future.
// This probably actually will be useful in the "raw" feature to come later.

/**
 * Get line to print out to stdout.
 *
 * @param   outputLine
 */
char csv_handler_output_line(char **outputLine)
{
    if (line == NULL) {
        return CSV_HANDLER__LINE_IS_NULL;
    }
    if (*outputLine != NULL) {
        free(*outputLine);
        *outputLine = NULL;
    }

    char **parsedLine = parse_csv(line, delim);

    *outputLine = (char *) malloc(sizeof(char) * 2);
    (*outputLine)[0] = '|'; // Opening brace.
    (*outputLine)[1] = '\0';

    for (int i = 0; parsedLine[i] != NULL; i++) {
        // Add in the next segment.

        int initialLen = strlen(*outputLine);
        *outputLine = (char *) realloc(*outputLine, sizeof(char) * (initialLen + 2 + width));
        // +2 is one for '|' and one for null terminator.

        // Only want to concat part of the string, so need to do some funky stuff.

        int contentLength = strlen(parsedLine[i]);
        if (contentLength > width) {
            contentLength = width;
        }
        int fillerLength = width - contentLength; // Will be zero if content is larger than width.

        for (int j = 0; j < contentLength; j++) {
            if (parsedLine[i][j] == '\n') {
                // Don't display newline.  It's confusing in this context.
                (*outputLine)[initialLen + j] = ' ';
            } else {
                (*outputLine)[initialLen + j] = parsedLine[i][j];
            }
            // Note that this starts with j = 0, so overwriting the null terminator.
        }
        for (int j = 0; j < fillerLength; j++) {
            (*outputLine)[initialLen + contentLength + j] = ' ';
        }

        (*outputLine)[initialLen + width] = '|'; // Next brace.
        (*outputLine)[initialLen + width + 1] = '\0'; // Putting back in the null terminator.
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

    int lineLen = (width + 1) * countFields + 1;
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

    return CSV_HANDLER__OK;
}
