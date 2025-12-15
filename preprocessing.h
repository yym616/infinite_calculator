#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <stdio.h>

char *read_line_dynamic(FILE *in);
int is_blank_line(const char *s);
char* preprocess(const char *s);
char* infix_to_postfix(const char *s);

#endif