#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* 공백 제거 + 암시적 곱셈 처리
 *  - "2(3+2)"  -> "2*(3+2)"
 *  - "(1+2)3"  -> "(1+2)*3"
 */
static char* preprocess(const char *s) {
    int len = (int)strlen(s);
    char *out = (char*)malloc(len * 2 + 10);
    int j = 0;

    for (int i = 0; i < len; i++) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\n') continue;

        out[j++] = c;

        char next = s[i+1];
        if (!next) continue;

        // 숫자 또는 ')' 뒤에 바로 '(' 오는 경우: 곱셈
        if ((isdigit((unsigned char)c) || c == ')') && next == '(') {
            out[j++] = '*';
        }
        // ')' 뒤에 숫자가 오는 경우도 곱셈
        if (c == ')' && isdigit((unsigned char)next)) {
            out[j++] = '*';
        }
    }

    out[j] = '\0';
    return out;
}

static int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*') return 2;
    return 0;
}

/* 중위식 -> 후위식 (Shunting-yard) */
static char* infix_to_postfix(const char *s) {
    int len = (int)strlen(s);
    char *out = (char*)malloc(len * 3 + 10);
    int j = 0;

    char opstack[512];
    int top = -1;

    for (int i = 0; i < len; i++) {
        char c = s[i];

        if (isdigit((unsigned char)c) || c == '.') {
            // 숫자/소수 토큰
            out[j++] = c;
            if (!(isdigit((unsigned char)s[i+1]) || s[i+1] == '.')) {
                out[j++] = ' ';
            }
        }
        else if (c == '(') {
            opstack[++top] = c;
        }
        else if (c == ')') {
            while (top >= 0 && opstack[top] != '(') {
                out[j++] = opstack[top--];
                out[j++] = ' ';
            }
            if (top >= 0 && opstack[top] == '(') top--; // '(' pop
        }
        else if (c == '+' || c == '-' || c == '*') {
            while (top >= 0 && opstack[top] != '(' &&
                   precedence(opstack[top]) >= precedence(c)) {
                out[j++] = opstack[top--];
                out[j++] = ' ';
            }
            opstack[++top] = c;
        }
        else {
            // 기타 문자는 무시
        }
    }

    while (top >= 0) {
        if (opstack[top] != '(') {
            out[j++] = opstack[top];
            out[j++] = ' ';
        }
        top--;
    }

    out[j] = '\0';
    return out;
}

#endif
