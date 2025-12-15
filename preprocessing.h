#include <ctype.h>
#include <string.h>
#include <stdlib.h>

//한 줄(한 문제)씩 동적 입력
static char *read_line_dynamic(FILE *in) {
    size_t cap = 128;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) {
        printf("malloc failed\n");
        return NULL;
    }

    int ch;
    char * tmp;
    while ((ch = fgetc(in)) != EOF) {
        if (ch == '\n') break;        //한 줄 끝

        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = (char *)realloc(buf, cap);
            if (!tmp) {
                printf("realloc failed\n");
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
        buf[len++] = (char)ch;
    }

    if (ch == EOF && len == 0) {
        free(buf);
        return NULL; // 더 이상 읽을 줄 없음
    }

    buf[len] = '\0';
    return buf;
}

// 빈 줄은 건너뜀
static int is_blank_line(const char *s) {
    while (*s) {
        if (*s != ' ' && *s != '\t') return 0;
        s++;
    }
    return 1;
}

//다음 의미있는 문자(공백,개행 제외)를 찾아 반환, 없으면 '\0
//preprocess 함수에서 다음 문자 확인에 사용
static char peek_next_nonspace(const char *s, int start_idx) {
    int k = start_idx;
    while (s[k] && (s[k] == ' ' || s[k] == '\t' || s[k] == '\n' || s[k] == '\r')) k++;
    return s[k] ? s[k] : '\0';
}

//공백 제거 + 괄호 곱셈 처리 (괄호와 괄호 사이 공백도 처리)
static char* preprocess(const char *s) {
    int len = (int)strlen(s);
    char *out = (char*)malloc(len * 3 + 20); //넉넉히 할당
    int j = 0;

    for (int i = 0; i < len; i++) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue; //의미 없는 문자 무시

        out[j++] = c;

        char next = peek_next_nonspace(s, i + 1); //다음 문자 확인
        if (!next) continue;

        /*괄호 곱셈 규칙
           1) 숫자 or '. ' or ')' 다음에 '(' 오면 곱셈 처리
           2) ')' 다음에 숫자 or '.' 오면 곱셈 처리
        */
        if ((isdigit((unsigned char)c) || c == '.' || c == ')') && next == '(') {
            out[j++] = '*';
        }
        if (c == ')' && (isdigit((unsigned char)next) || next == '.')) {
            out[j++] = '*';
        }
    }

    out[j] = '\0';
    return out;
}

// 연산자 우선 순위 판별, 스택 쌓는 알고리즘에 사용
static int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

//연산자인지 판별
static int is_op(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

//중위식 -> 후위식 변환 
//단항 +/- 문제 해결: 식 시작 또는 '(' 또는 연산자 뒤에 오는 +/- 가 숫자에 붙으면 숫자 토큰으로 처리 ex)(2*-3)  */
static char* infix_to_postfix(const char *s) {
    int len = (int)strlen(s);
    char *out = (char*)malloc(len * 4 + 20); //넉넉히 할당, main에서 free
    int j = 0;

    char opstack[4096];
    int top = -1;

    /* prevType: 이전 문자 형식 판별, +/-가 단항인지 이항인지 구분하기 위해!
                0=시작,연산자,'('      -> 부호
                1=숫자,               -> 연산
                2=')'     */
    int prevType = 0;

    for (int i = 0; i < len; ) {
        char c = s[i];

        // 숫자(또는 소수점 시작) 토큰
        if (isdigit((unsigned char)c) || c == '.') {
            prevType = 1;
            while (isdigit((unsigned char)s[i]) || s[i] == '.') {
                out[j++] = s[i++];
            }
            out[j++] = ' ';
            continue;
        }

        // 단항 +/- : 시작, 연산자, '(' 뒤에 오는 +/- 이고 다음이 숫자/소수점이면 숫자 토큰으로 흡수
        if ((c == '+' || c == '-') && prevType == 0) {
            char next = s[i+1];
            if (isdigit((unsigned char)next) || next == '.') {
                prevType = 1;
                out[j++] = c; // 부호 포함, 숫자 토큰에 흡수
                i++;
                while (isdigit((unsigned char)s[i]) || s[i] == '.') {
                    out[j++] = s[i++];
                }
                out[j++] = ' ';
                continue;
            }
        }
        // '('는 무조건 푸시
        if (c == '(') {
            opstack[++top] = c;
            prevType = 0;
            i++;
            continue;
        }
        // ')'는 '('나올때까지 pop
        if (c == ')') {
            while (top >= 0 && opstack[top] != '(') {
                out[j++] = opstack[top--];
                out[j++] = ' ';
            }
            if (top >= 0 && opstack[top] == '(') top--;
            prevType = 2;
            i++;
            continue;
        }

        // 이항 연산자
        if (is_op(c)) {
            while (top >= 0 && opstack[top] != '(' &&
                   precedence(opstack[top]) >= precedence(c)) {
                out[j++] = opstack[top--];
                out[j++] = ' '; //스택안의 연산자가 우선순위가 더 높으면 pop
            }
            opstack[++top] = c; 
            prevType = 0;
            i++;
            continue;
        }

        // 기타 문자는 무시
        i++;
    }
    // 남은 연산자 전부 pop
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
