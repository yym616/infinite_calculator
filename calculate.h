#ifndef CALCULATE_H
#define CALCULATE_H

#include <stdio.h>

//자료 구조
typedef struct DigitNode {
    int digit;
    struct DigitNode *next;
    struct DigitNode *prev;
} DigitNode;

typedef struct BigNumber {
    int sign;
    int scale;
    DigitNode *head;   // LSB
    DigitNode *tail;   // MSB
} BigNumber;

//free 함수
void free_bignumber(BigNumber *n);

//기본 유틸
DigitNode* new_digit(int d);
BigNumber* new_bignumber(void);
void push_lsb(BigNumber *n, int d);
void push_msb(BigNumber *n, int d);
int length_digits(const BigNumber *n);

//bignumber 복사 -> 원본 훼손 방지
BigNumber* clone_bignumber(const BigNumber *src);

//0인지 확인, 정규화
int is_zero(const BigNumber *n);
void normalize(BigNumber *n);

//문자열->BihNumber
BigNumber* parse_number(const char *s);

//BihNumber 출력
void print_bignumber(const BigNumber *n);

//소수점 맞춤
void align_scales(BigNumber *a, BigNumber *b);

//절댓값 비교
int compare_abs(const BigNumber *A, const BigNumber *B);

//후위식 메인 계산 함수
BigNumber* evaluate_postfix(const char *post);

#endif
