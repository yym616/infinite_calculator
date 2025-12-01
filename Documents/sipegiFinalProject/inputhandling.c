#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void)
{
    FILE *input_file;
    char *input_buffer = NULL;     /* Pointer for dynamically allocated buffer */
    size_t buffer_size = 0;        /* Buffer size (managed by getline) */
    ssize_t input_length;          /* Number of characters read per line */
    int current_line = 0;          /* Line counter */
    int is_valid, char_index, current_char;

    /* Attempt to open the input file for reading */
    input_file = fopen("input", "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error!!! unable to open file 'input': %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Read each line dynamically using getline */
    while ((input_length = getline(&input_buffer, &buffer_size, input_file)) != -1) {
        current_line++;

        /* Remove newline character at end, if it exists */
        if (input_length > 0 && input_buffer[input_length - 1] == '\n') {
            input_buffer[input_length - 1] = '\0';
            input_length--;
        }

        /* Skip empty lines and print a message */
        if (input_length == 0) {
            printf("Line %d is vacant. Skipping.\n", current_line);
            continue;
        }

        /*  확인 each character in the line */
        is_valid = 1;
        for (char_index = 0; char_index < input_length; char_index++) {
            current_char = input_buffer[char_index];
            if ((current_char < '0' || current_char > '9') &&
                current_char != '+' && current_char != '-' &&
                current_char != '*' && current_char != '/' &&
                current_char != '(' && current_char != ')' &&
                current_char != '.' && current_char != ' ') {
                printf("Error!!! Line %d contains invalid character '%c'.\n", current_line, current_char);
                is_valid = 0;
                break;
            }
        }
        if (!is_valid) continue;

        /* Print the valid line number and expression */
        printf("Line %d: %s\n", current_line, input_buffer);
    }

    /* 읽은 후 , check for any file reading errors */
    if (ferror(input_file)) {
        fprintf(stderr, "Error reading from file.\n");
    }

    free(input_buffer);     /* Release dynamically-allocated memory */
    fclose(input_file);     /* Close the input file */
    return 0;
}
