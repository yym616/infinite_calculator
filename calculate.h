#ifndef CALCULATE_H
#define CALCULATE_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* ==================== BigNumber 정의 ==================== */

typedef struct DigitNode {
    int digit;                  
    struct DigitNode* prev;
    struct DigitNode* next;
} DigitNode;

typedef struct BigNumber {
    DigitNode* head;
    DigitNode* tail;
    int scale;          // 소수부 자리수
} BigNumber;

/* ==================== BigNumber 생성/유틸 ==================== */

static BigNumber* create_bignumber() {
    BigNumber* bn = malloc(sizeof(BigNumber));
    bn->head = bn->tail = NULL;
    bn->scale = 0;
    return bn;
}

static void append_digit(BigNumber* bn, int digit, int in_fraction) {
    DigitNode* node = malloc(sizeof(DigitNode));
    node->digit = digit;
    node->next = NULL;
    node->prev = bn->tail;

    if (!bn->tail)
        bn->head = node;
    else
        bn->tail->next = node;

    bn->tail = node;

    if (in_fraction)
        bn->scale++;
}

static void free_bignumber(BigNumber* bn) {
    DigitNode* p = bn->head;
    while (p) {
        DigitNode* nx = p->next;
        free(p);
        p = nx;
    }
    free(bn);
}

/* 문자열 → BigNumber (ex: "123.45") */
static BigNumber* make_bignumber_from_token(const char* token, size_t len) {
    BigNumber* bn = create_bignumber();
    int in_fraction = 0;

    for (size_t i = 0; i < len; i++) {
        char c = token[i];
        if (c == '.') {
            in_fraction = 1;
            continue;
        }
        if (!isdigit((unsigned char)c)) continue;

        append_digit(bn, c - '0', in_fraction);
    }
    return bn;
}

/* ==================== BigNumber 스택 ==================== */

typedef struct BigNode {
    BigNumber* value;
    struct BigNode* next;
} BigNode;

static void push_big(BigNode** top, BigNumber* v) {
    BigNode* n = malloc(sizeof(BigNode));
    n->value = v;
    n->next = *top;
    *top = n;
}

static BigNumber* pop_big(BigNode** top) {
    if (!*top) return NULL;
    BigNode* del = *top;
    BigNumber* v = del->value;
    *top = del->next;
    free(del);
    return v;
}

/* ==================== 덧셈 add(a, b) ==================== */

BigNumber* add(const BigNumber* A, const BigNumber* B) {
    if (!A || !B) return NULL;

    /* A, B 전체 길이 */
    int lenA = 0, lenB = 0;
    for (DigitNode* p = A->head; p; p = p->next) lenA++;
    for (DigitNode* p = B->head; p; p = p->next) lenB++;

    int fracA = A->scale;
    int fracB = B->scale;

    int intA  = lenA - fracA;
    int intB  = lenB - fracB;

    int frac_res = (fracA > fracB ? fracA : fracB);
    int int_res  = (intA  > intB  ? intA  : intB);

    int L = int_res + frac_res;      // 정수부+소수부 총 길이

    /* A, B를 자릿수 배열로 변환해서 같은 자리 정렬 */
    int* a = calloc(L, sizeof(int));
    int* b = calloc(L, sizeof(int));

    /* A 채우기 */
    {
        DigitNode* p = A->head;
        int off = int_res - intA;    // 정수부 왼쪽 패딩 길이

        for (int i = 0; i < intA; i++) {
            a[off + i] = p->digit;
            p = p->next;
        }
        for (int i = 0; i < fracA; i++) {
            a[int_res + i] = p->digit;
            p = p->next;
        }
    }

    /* B 채우기 */
    {
        DigitNode* p = B->head;
        int off = int_res - intB;

        for (int i = 0; i < intB; i++) {
            b[off + i] = p->digit;
            p = p->next;
        }
        for (int i = 0; i < fracB; i++) {
            b[int_res + i] = p->digit;
            p = p->next;
        }
    }

    /* 정수 덧셈 (캐리 포함) */
    int* R = calloc(L + 1, sizeof(int));
    int carry = 0;

    for (int i = L - 1; i >= 0; i--) {
        int sum = a[i] + b[i] + carry;
        R[i + 1] = sum % 10;
        carry = sum / 10;
    }
    R[0] = carry;

    free(a);
    free(b);

    /* BigNumber 결과 생성
       🔥 scale 은 append_digit 에서만 증가시키고
       여기서는 직접 건드리지 않는다!!
    */
    BigNumber* res = create_bignumber();

    int start = (R[0] == 0 ? 1 : 0);     // 맨 앞 0이면 스킵

    for (int i = start; i <= L; i++) {
        int d = R[i];
        int pos_from_right = L - i;      // 0 = 제일 오른쪽 자리
        int in_fraction = (pos_from_right < frac_res);
        append_digit(res, d, in_fraction);
    }

    free(R);
    return res;
}

/* ==================== calculate(postfix) ==================== */

static int is_op(const char* t, size_t len) {
    return (len == 1 && (t[0] == '+' || t[0] == '-' || t[0] == '*' || t[0] == '/'));
}

static BigNumber* calculate(const char* post) {
    BigNode* st = NULL;
    const char* p = post;

    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        const char* s = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        size_t len = p - s;

        if (is_op(s, len)) {
            BigNumber* b = pop_big(&st);
            BigNumber* a = pop_big(&st);
            BigNumber* r = add(a, b);
            push_big(&st, r);
        } else {
            BigNumber* x = make_bignumber_from_token(s, len);
            push_big(&st, x);
        }
    }

    return pop_big(&st);
}

/* ==================== print_bignumber ==================== */

static void print_bignumber(const BigNumber* bn) {
    if (!bn) {
        printf("(null)");
        return;
    }

    int total = 0;
    for (DigitNode* p = bn->head; p; p = p->next)
        total++;

    int int_len = total - bn->scale;   // 정수 자릿수

    if (int_len <= 0)
        printf("0");                   // 0.xxx 형태

    int idx = 0;
    for (DigitNode* p = bn->head; p; p = p->next, idx++) {

        if (idx == int_len && bn->scale > 0)
            printf(".");

        printf("%d", p->digit);
    }
}

#endif
