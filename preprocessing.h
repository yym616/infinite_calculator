#include <stdio.h>
#include <stdlib.h>   // malloc, realloc, free
#include <string.h>   // strlen
#include <ctype.h>    // isdigit

// read_line, postfix, calculate, +, -, *, /

char* read_line(FILE* fp);
char* postfix(char* line, int len);

struct NODE {
    char operator;
    struct NODE* next;
};

void push(struct NODE* target, char operator);
char pop(struct NODE* target);
int precedence(char op);

/* ----------------- 입력 한 줄 읽기 ----------------- */

char* read_line(FILE* fp)
{
    char* buf = NULL;
    size_t len = 0;
    int ch;

    while ((ch = fgetc(fp)) != EOF) {
        char* new_buf = realloc(buf, len + 2);
        if (new_buf == NULL) {
            free(buf);
            return NULL;
        }
        buf = new_buf;

        buf[len++] = (char)ch;

        if (ch == '\n') {
            break;
        }
    }
    if (len == 0) {
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    return buf;   // 👉 나중에 main에서 free 해줘야 함
}

/* -------------- 연산자 우선순위 함수 -------------- */

int precedence(char op)
{
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

/* ------------- infix → postfix 변환 -------------- */

char* postfix(char* line, int len)
{
    struct NODE top = { 0, NULL };             // 스택 헤드 (더미노드)
    char* post = malloc(len * 3 + 10);         // 공백 때문에 여유 있게
    int j = 0;                                 // post 인덱스

    if (post == NULL) {
        return NULL;
    }

    for (int i = 0; i < len && line[i] != '\0'; i++)
    {
        char c = line[i];

        // 🔹 숫자 또는 '.' → 하나의 실수 토큰으로 처리
        if (isdigit((unsigned char)c) || c == '.') {
            // 숫자/점이 끝날 때까지 계속 복사
            while (i < len && line[i] != '\0' &&
                (isdigit((unsigned char)line[i]) || line[i] == '.')) {
                post[j++] = line[i++];
            }
            post[j++] = ' ';   // 토큰 구분용 공백
            i--;               // for문의 i++ 보정
        }
        // 🔹 연산자 처리
        else if (c == '+' || c == '-' || c == '*' || c == '/') {

            // 스택 top의 우선순위가 현재 연산자보다 크거나 같으면 pop해서 출력
            while (top.next != NULL &&
                precedence(top.next->operator) >= precedence(c)) {

                // 연산자 앞에 공백 하나 (토큰 구분)
                if (j > 0 && post[j - 1] != ' ')
                    post[j++] = ' ';

                post[j++] = pop(&top);
                post[j++] = ' ';   // 연산자 뒤에도 공백
            }

            push(&top, c);
        }
        // 🔹 공백/탭은 무시
        else if (c == ' ' || c == '\t') {
            // 그냥 무시
        }
        // 그 외 문자는 현재는 무시
    }

    // 🔹 스택에 남은 연산자들 모두 출력
    while (top.next != NULL) {
        if (j > 0 && post[j - 1] != ' ')
            post[j++] = ' ';

        post[j++] = pop(&top);
        post[j++] = ' ';
    }

    // 마지막 공백 하나 정리
    if (j > 0 && post[j - 1] == ' ')
        j--;

    post[j] = '\0';
    return post;
}

/* ---------------- 스택 push / pop ---------------- */

void push(struct NODE* target, char operator)
{
    struct NODE* pushNode = malloc(sizeof(struct NODE));
    if (pushNode == NULL) return;

    pushNode->operator = operator;
    pushNode->next = target->next;
    target->next = pushNode;
}

char pop(struct NODE* target)
{
    char popData;

    if (target->next == NULL) {
        printf("Stack is empty!\n");
        return '\0';
    }

    struct NODE* delNode = target->next;
    popData = delNode->operator;

    target->next = delNode->next;
    free(delNode);

    return popData;
}
