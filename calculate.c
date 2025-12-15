#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "calculate.h"
#include "operation.h"

//free 함수
void free_bignumber(BigNumber *n) {
    if (!n) return;
    DigitNode *cur = n->head;
    while (cur) {
        DigitNode *t = cur;
        cur = cur->next;
        free(t);
    }
    free(n);
}

//기본 유틸
DigitNode* new_digit(int d) {
    DigitNode *n = malloc(sizeof(DigitNode));
    n->digit = d;
    n->next = n->prev = NULL;
    return n;
}

BigNumber* new_bignumber(void) {
    BigNumber *n = malloc(sizeof(BigNumber));
    n->sign = 1;
    n->scale = 0;
    n->head = n->tail = NULL;
    return n;
}

void push_lsb(BigNumber *n, int d) {
    DigitNode *x = new_digit(d);
    x->next = n->head;
    if (n->head) n->head->prev = x;
    n->head = x;
    if (!n->tail) n->tail = x;
}

void push_msb(BigNumber *n, int d) {
    DigitNode *x = new_digit(d);
    x->prev = n->tail;
    if (n->tail) n->tail->next = x;
    n->tail = x;
    if (!n->head) n->head = x;
}

int length_digits(const BigNumber *n) {
    int len = 0;
    for (DigitNode *c = n->head; c; c = c->next) len++;
    return len;
}

BigNumber* clone_bignumber(const BigNumber *src) {
    BigNumber *n = new_bignumber();
    n->sign = src->sign;
    n->scale = src->scale;
    for (DigitNode *c = src->head; c; c = c->next)
        push_msb(n, c->digit);
    return n;
}

//0인지 확인, 정규화
int is_zero(const BigNumber *n) {
    for (DigitNode *c = n->head; c; c = c->next)
        if (c->digit != 0) return 0;
    return 1;
}

void normalize(BigNumber *n) {
    if (!n->head) {
        push_msb(n, 0);
        n->scale = 0;
        n->sign = 1;
        return;
    }

    int len = length_digits(n);
    while (n->tail && n->tail->digit == 0 && len > n->scale + 1) {
        DigitNode *t = n->tail;
        n->tail = t->prev;
        if (n->tail) n->tail->next = NULL;
        else n->head = NULL;
        free(t);
        len--;
    }

    if (is_zero(n)) {
        n->sign = 1;
        n->scale = 0;
    }
}

//문자열->BihNumber
BigNumber* parse_number(const char *s) {
    BigNumber *n = new_bignumber();
    int i = 0;

    if (s[i] == '+') i++;
    else if (s[i] == '-') { n->sign = -1; i++; }

    int cap = 16, len = 0;
    int *digits = malloc(cap * sizeof(int));
    int dot_pos = -1;

    for (; s[i]; i++) {
        if (s[i] == '.') {
            dot_pos = len;
        } else if (isdigit((unsigned char)s[i])) {
            if (len >= cap) {
                cap *= 2;
                digits = realloc(digits, cap * sizeof(int));
            }
            digits[len++] = s[i] - '0';
        }
    }

    n->scale = (dot_pos == -1) ? 0 : (len - dot_pos);

    if (len == 0) {
        push_msb(n, 0);
    } else {
        for (int k = len - 1; k >= 0; k--)
            push_msb(n, digits[k]);
    }

    free(digits);
    normalize(n);
    return n;
}

//BihNumber 출력
void print_bignumber(const BigNumber *n) {
    if (n->sign < 0 && !is_zero(n)) printf("-");
    int len = length_digits(n);
    DigitNode *c = n->tail;
    for (int i = len - 1; i >= 0; i--) {
        printf("%d", c->digit);
        if (n->scale > 0 && i == n->scale) printf(".");
        c = c->prev;
    }
}

//소수점 맞춤
void align_scales(BigNumber *a, BigNumber *b) {
    while (a->scale < b->scale) { push_lsb(a, 0); a->scale++; }
    while (b->scale < a->scale) { push_lsb(b, 0); b->scale++; }
}

//절댓값 비교
int compare_abs(const BigNumber *A, const BigNumber *B) {
    BigNumber *a = clone_bignumber(A);
    BigNumber *b = clone_bignumber(B);
    align_scales(a, b);

    int la = length_digits(a), lb = length_digits(b);
    if (la != lb) {
        int r = (la > lb) ? 1 : -1;
        free_bignumber(a); free_bignumber(b);
        return r;
    }

    for (DigitNode *ca = a->tail, *cb = b->tail; ca; ca = ca->prev, cb = cb->prev) {
        if (ca->digit != cb->digit) {
            int r = (ca->digit > cb->digit) ? 1 : -1;
            free_bignumber(a); free_bignumber(b);
            return r;
        }
    }

    free_bignumber(a); free_bignumber(b);
    return 0;
}

//후위식 메인 계산 함수
typedef struct NumNode {
    BigNumber *num;
    struct NumNode *next;
} NumNode;

static void push_num(NumNode **s, BigNumber *n) {
    NumNode *x = malloc(sizeof(NumNode));
    x->num = n;
    x->next = *s;
    *s = x;
}

static BigNumber* pop_num(NumNode **s) {
    NumNode *t = *s;
    if (!t) return NULL;
    *s = t->next;
    BigNumber *n = t->num;
    free(t);
    return n;
}

static int is_number_start(const char *p) {
    if (isdigit((unsigned char)p[0]) || p[0]=='.') return 1;
    if ((p[0]=='+'||p[0]=='-') && (isdigit((unsigned char)p[1])||p[1]=='.')) return 1;
    return 0;
}

//메인 postfix 계산 함수
BigNumber* evaluate_postfix(const char *post) {
    NumNode *st = NULL;
    int i = 0;

    while (post[i]) {
        while (post[i] == ' ') i++;
        if (!post[i]) break;

        if (is_number_start(&post[i])) {
            int cap = 16, len = 0;
            char *tok = malloc(cap);

            if (post[i]=='+' || post[i]=='-') tok[len++] = post[i++];

            while (isdigit((unsigned char)post[i]) || post[i]=='.') {
                if (len+1 >= cap) {
                    cap *= 2;
                    tok = realloc(tok, cap);
                }
                tok[len++] = post[i++];
            }
            tok[len] = '\0';

            BigNumber *n = parse_number(tok);
            free(tok);
            push_num(&st, n);
        }
        else if (post[i]=='+'||post[i]=='-'||post[i]=='*'||post[i]=='/') {
            char op = post[i++];
            BigNumber *b = pop_num(&st);
            BigNumber *a = pop_num(&st);
            if (!a || !b) return NULL;

            BigNumber *r =
                (op=='+') ? add(a,b) :
                (op=='-') ? subtract(a,b) :
                (op=='*') ? multiply(a,b) :
                            divide(a,b);

            free_bignumber(a);
            free_bignumber(b);

            if (!r) {
                while (st) free_bignumber(pop_num(&st));
                return NULL;
            }

            push_num(&st, r);
        }
        else i++;
    }

    BigNumber *res = pop_num(&st);
    while (st) free_bignumber(pop_num(&st));
    return res;
}
