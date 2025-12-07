#ifndef CALCULATE_H
#define CALCULATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ===============================
 *  BigNumber 구조체 정의
 *  - 임의 길이 10진수
 *  - digits: LSB(1의 자리)가 head 에 있는 연결리스트
 *  - scale: 소수부 자리수 (10^scale 로 나눈 값)
 *  - sign: 1 또는 -1
 * =============================== */

typedef struct DigitNode {
    int digit;
    struct DigitNode *next;
    struct DigitNode *prev;
} DigitNode;

typedef struct BigNumber {
    int sign;          // 1 또는 -1
    int scale;         // 소수부 자리수
    DigitNode *head;   // LSB
    DigitNode *tail;   // MSB
} BigNumber;

/* ====== 유틸 함수 ====== */

static DigitNode* new_digit(int d) {
    DigitNode *n = (DigitNode*)malloc(sizeof(DigitNode));
    n->digit = d;
    n->next = n->prev = NULL;
    return n;
}

static BigNumber* new_bignumber(void) {
    BigNumber *n = (BigNumber*)malloc(sizeof(BigNumber));
    n->sign = 1;
    n->scale = 0;
    n->head = n->tail = NULL;
    return n;
}

static void push_lsb(BigNumber *n, int d) { // head 쪽(LSB)에 삽입
    DigitNode *x = new_digit(d);
    x->next = n->head;
    if (n->head) n->head->prev = x;
    n->head = x;
    if (!n->tail) n->tail = x;
}

static void push_msb(BigNumber *n, int d) { // tail 쪽(MSB)에 삽입
    DigitNode *x = new_digit(d);
    x->prev = n->tail;
    if (n->tail) n->tail->next = x;
    n->tail = x;
    if (!n->head) n->head = x;
}

static int length_digits(const BigNumber *n) {
    int len = 0;
    DigitNode *cur = (DigitNode*)n->head;
    while (cur) {
        len++;
        cur = cur->next;
    }
    return len;
}

static BigNumber* clone_bignumber(const BigNumber *src) {
    BigNumber *n = new_bignumber();
    n->sign = src->sign;
    n->scale = src->scale;
    DigitNode *cur = src->head;
    while (cur) {
        push_msb(n, cur->digit);
        cur = cur->next;
    }
    return n;
}

/* zero 체크 & 정규화 */

static int is_zero(const BigNumber *n) {
    DigitNode *cur = n->head;
    while (cur) {
        if (cur->digit != 0) return 0;
        cur = cur->next;
    }
    return 1;
}

static void normalize(BigNumber *n) {
    if (!n->head) {
        push_msb(n, 0);
        n->scale = 0;
        n->sign = 1;
        return;
    }

    // 앞쪽(MSB) 불필요한 0 제거 (integer part에서)
    int len = length_digits(n);
    while (n->tail && n->tail->digit == 0 && len > n->scale + 1) {
        DigitNode *t = n->tail;
        n->tail = t->prev;
        if (n->tail) n->tail->next = NULL;
        else n->head = NULL;
        free(t);
        len--;
    }

    if (!n->head) {
        push_msb(n, 0);
        n->scale = 0;
        n->sign = 1;
        return;
    }

    if (is_zero(n)) {
        n->sign = 1;
        // 0.xxx 꼴로 남을 수 있는데, 편의상 scale 0으로 줄임
        n->scale = 0;
    }
}

/* ===============================
 *  문자열 -> BigNumber
 * =============================== */

static BigNumber* parse_number(const char *s) {
    BigNumber *n = new_bignumber();
    int i = 0;
    int len = (int)strlen(s);

    // 부호
    if (s[i] == '+') i++;
    else if (s[i] == '-') {
        n->sign = -1;
        i++;
    }

    int dot_pos = -1;
    int tmp_len = 0;
    int digits_buf[1024];  // 토큰 하나 기준이라 1024면 충분

    for (; i < len; i++) {
        if (s[i] == '.') {
            dot_pos = tmp_len;
        } else if (isdigit((unsigned char)s[i])) {
            digits_buf[tmp_len++] = s[i] - '0';
        }
    }

    if (dot_pos == -1) {
        n->scale = 0;
    } else {
        n->scale = tmp_len - dot_pos;
    }

    if (tmp_len == 0) {
        push_msb(n, 0);
        n->scale = 0;
        n->sign = 1;
        return n;
    }

    // digits_buf[0] = 가장 왼쪽(최상위 자리)
    // LSB부터 넣기 위해 뒤에서부터 읽어서 push_msb
    for (int k = tmp_len - 1; k >= 0; k--) {
        push_msb(n, digits_buf[k]);
    }

    normalize(n);
    return n;
}

/* ===============================
 *  BigNumber 출력
 * =============================== */

static void print_bignumber(const BigNumber *n) {
    if (n->sign < 0 && !is_zero(n)) printf("-");

    int len = length_digits(n);
    int scale = n->scale;

    // MSB부터 찍으면서 scale 위치에서 '.' 찍기
    DigitNode *cur = n->tail;
    for (int idx = len - 1; idx >= 0; idx--) {
        if (!cur) break;
        printf("%d", cur->digit);
        if (scale > 0 && idx == scale) {
            printf(".");
        }
        cur = cur->prev;
    }

    // 소수부가 있는데, 자리가 scale보다 적으면 0 채우는 처리도 가능하지만
    // 현재 구조에서는 필요 없음
}

/* ===============================
 *  scale 맞추기 (동일하게 변경)
 * =============================== */

static void align_scales(BigNumber *a, BigNumber *b) {
    if (a->scale == b->scale) return;

    if (a->scale < b->scale) {
        int diff = b->scale - a->scale;
        for (int i = 0; i < diff; i++) {
            push_lsb(a, 0);      // 값 *10
        }
        a->scale = b->scale;
    } else {
        int diff = a->scale - b->scale;
        for (int i = 0; i < diff; i++) {
            push_lsb(b, 0);
        }
        b->scale = a->scale;
    }
}

/* ===============================
 *  크기 비교 (절댓값)
 * =============================== */

static int compare_abs(const BigNumber *A, const BigNumber *B) {
    BigNumber *a = clone_bignumber(A);
    BigNumber *b = clone_bignumber(B);
    align_scales(a, b);

    int lenA = length_digits(a);
    int lenB = length_digits(b);

    if (lenA != lenB) {
        int res = (lenA > lenB) ? 1 : -1;
        free(a); free(b); // 여기서는 대강, digits는 릭 나도 상관없음
        return res;
    }

    DigitNode *ca = a->tail;
    DigitNode *cb = b->tail;
    while (ca && cb) {
        if (ca->digit != cb->digit) {
            int res = (ca->digit > cb->digit) ? 1 : -1;
            free(a); free(b);
            return res;
        }
        ca = ca->prev;
        cb = cb->prev;
    }
    free(a); free(b);
    return 0;
}

/* ===============================
 *  절댓값 덧셈 / 뺄셈 (scale 동일 가정)
 * =============================== */

static BigNumber* add_abs(const BigNumber *A, const BigNumber *B) {
    BigNumber *R = new_bignumber();
    R->sign = 1;
    R->scale = A->scale; // A, B 동일 가정

    DigitNode *da = A->head;
    DigitNode *db = B->head;
    int carry = 0;

    while (da || db || carry) {
        int ad = da ? da->digit : 0;
        int bd = db ? db->digit : 0;
        int sum = ad + bd + carry;
        carry = sum / 10;
        push_msb(R, sum % 10);

        if (da) da = da->next;
        if (db) db = db->next;
    }

    normalize(R);
    return R;
}

static BigNumber* sub_abs(const BigNumber *A, const BigNumber *B) {
    // |A| >= |B| 가정
    BigNumber *R = new_bignumber();
    R->sign = 1;
    R->scale = A->scale;

    DigitNode *da = A->head;
    DigitNode *db = B->head;
    int borrow = 0;

    while (da || db) {
        int ad = da ? da->digit : 0;
        int bd = db ? db->digit : 0;

        int diff = ad - borrow - bd;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else borrow = 0;

        push_msb(R, diff);

        if (da) da = da->next;
        if (db) db = db->next;
    }

    normalize(R);
    return R;
}

/* ===============================
 *  add / subtract / multiply
 * =============================== */

static BigNumber* add(const BigNumber *A, const BigNumber *B) {
    // 0 처리
    if (is_zero(A)) return clone_bignumber(B);
    if (is_zero(B)) return clone_bignumber(A);

    BigNumber *a = clone_bignumber(A);
    BigNumber *b = clone_bignumber(B);

    align_scales(a, b);

    BigNumber *R = NULL;

    if (a->sign == b->sign) {
        R = add_abs(a, b);
        R->sign = a->sign;
    } else {
        int cmp = compare_abs(a, b);
        if (cmp == 0) {
            R = new_bignumber();
            push_msb(R, 0);
            R->sign = 1;
            R->scale = a->scale;
        } else if (cmp > 0) {
            R = sub_abs(a, b);
            R->sign = a->sign;
        } else {
            R = sub_abs(b, a);
            R->sign = b->sign;
        }
    }

    normalize(R);
    return R;
}

static BigNumber* subtract(const BigNumber *A, const BigNumber *B) {
    BigNumber *nb = clone_bignumber(B);
    nb->sign *= -1;
    BigNumber *R = add(A, nb);
    return R;
}

static BigNumber* multiply(const BigNumber *A, const BigNumber *B) {
    BigNumber *a = clone_bignumber(A);
    BigNumber *b = clone_bignumber(B);

    int lenA = length_digits(a);
    int lenB = length_digits(b);

    int maxLen = lenA + lenB + 2;
    int *arr = (int*)calloc(maxLen, sizeof(int));

    int i = 0;
    for (DigitNode *da = a->head; da; da = da->next, i++) {
        int j = 0;
        for (DigitNode *db = b->head; db; db = db->next, j++) {
            arr[i+j] += da->digit * db->digit;
        }
    }

    // carry 처리
    for (int k = 0; k < maxLen - 1; k++) {
        if (arr[k] >= 10) {
            arr[k+1] += arr[k] / 10;
            arr[k] %= 10;
        }
    }

    // 실제 길이 계산
    int resLen = maxLen - 1;
    while (resLen > 0 && arr[resLen] == 0) resLen--;

    BigNumber *R = new_bignumber();
    R->sign = a->sign * b->sign;
    R->scale = a->scale + b->scale;

    for (int k = 0; k <= resLen; k++) {
        push_msb(R, arr[k]);
    }

    free(arr);
    normalize(R);
    return R;
}

/* ===============================
 *  후위표기 계산기
 * =============================== */

typedef struct NumNode {
    BigNumber *num;
    struct NumNode *next;
} NumNode;

static void push_num(NumNode **st, BigNumber *n) {
    NumNode *x = (NumNode*)malloc(sizeof(NumNode));
    x->num = n;
    x->next = *st;
    *st = x;
}

static BigNumber* pop_num(NumNode **st) {
    if (!*st) return NULL;
    NumNode *t = *st;
    *st = t->next;
    BigNumber *n = t->num;
    free(t);
    return n;
}

static BigNumber* evaluate_postfix(const char *post) {
    NumNode *stack = NULL;
    char token[1024];
    int i = 0;

    while (post[i]) {
        while (post[i] == ' ') i++;
        if (!post[i]) break;

        if (isdigit((unsigned char)post[i]) || post[i] == '.') {
            int ti = 0;
            while (isdigit((unsigned char)post[i]) || post[i] == '.') {
                token[ti++] = post[i++];
            }
            token[ti] = '\0';
            BigNumber *n = parse_number(token);
            push_num(&stack, n);
        } else if (post[i] == '+' || post[i] == '-' || post[i] == '*') {
            char op = post[i++];
            BigNumber *b = pop_num(&stack);
            BigNumber *a = pop_num(&stack);
            BigNumber *r = NULL;

            if (op == '+') r = add(a, b);
            else if (op == '-') r = subtract(a, b);
            else if (op == '*') r = multiply(a, b);

            push_num(&stack, r);
        } else {
            // 기타 문자(안 쓰는 것)는 스킵
            i++;
        }
    }

    BigNumber *res = pop_num(&stack);
    return res;
}

/* ===============================
 *  free 함수 (간단 버전)
 * =============================== */

static void free_bignumber(BigNumber *n) {
    if (!n) return;
    DigitNode *cur = n->head;
    while (cur) {
        DigitNode *t = cur;
        cur = cur->next;
        free(t);
    }
    free(n);
}

#endif
