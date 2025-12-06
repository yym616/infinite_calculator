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
    int is_negative;    // 음수 여부
} BigNumber;

/* ==================== BigNumber 생성/유틸 ==================== */

static BigNumber* create_bignumber() {
    BigNumber* bn = malloc(sizeof(BigNumber));
    bn->head = bn->tail = NULL;
    bn->scale = 0;
    bn->is_negative = 0;
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

    // 음수 처리
    int start_index = 0;
    if (token[0] == '-') {
        bn->is_negative = 1;
        start_index = 1;
    }

    for (size_t i = start_index; i < len; i++) {
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

/* ==================== 비교 함수 ==================== */
/* 8A > B이면 1, A == B이면 0, A < B이면 -1을 반환*/
static int compare_bignumber(const BigNumber* A, const BigNumber* B) {
    if (!A || !B) return 0;
    
    // 총 길이와 눈금을 구하려면...
    int lenA = 0, lenB = 0;
    for (DigitNode* p = A->head; p; p = p->next) lenA++;
    for (DigitNode* p = B->head; p; p = p->next) lenB++;
    
    // 분수 부분의 자릿수
    int fracA = A->scale;
    int fracB = B->scale;
    // 정수 부분의 자릿수
    int intA_len = lenA - fracA;
    int intB_len = lenB - fracB;
    
    // 정수 부분 길이 비교
    if (intA_len > intB_len) return 1;
    if (intA_len < intB_len) return -1;
    
    // 각 자릿수 비교, compare digit by digit
    int max_int = intA_len;  // since they're equal
    //소수점 이하 최대 길이 확인, Determine the maximum fractional part length
    int max_frac = (fracA > fracB) ? fracA : fracB;
    int total_len = max_int + max_frac;
    
    //  자릿수를 맞추기 위한 임시 배열 생성, Create temporary arrays to align digits
    int* a = calloc(total_len, sizeof(int));
    int* b = calloc(total_len, sizeof(int));
    
    // Fill A
    DigitNode* pA = A->head;
    for (int i = 0; i < intA_len; i++) {
        a[i] = pA->digit;
        pA = pA->next;
    }
    for (int i = 0; i < fracA; i++) {
        a[max_int + i] = pA->digit;
        pA = pA->next;
    }
    
    // Fill B
    DigitNode* pB = B->head;
    for (int i = 0; i < intB_len; i++) {
        b[i] = pB->digit;
        pB = pB->next;
    }
    for (int i = 0; i < fracB; i++) {
        b[max_int + i] = pB->digit;
        pB = pB->next;
    }
    
    // 자릿수별로 비교하다, Compare digit by digit
    int result = 0;
    for (int i = 0; i < total_len; i++) {
        if (a[i] > b[i]) {
            result = 1;
            break;
        } else if (a[i] < b[i]) {
            result = -1;
            break;
        }
    }
    
    free(a);
    free(b);
    return result;
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
        int in_fraction = (pos_from_right <= frac_res);
        append_digit(res, d, in_fraction);
    }

    free(R);
    return res;
}

/* ==================== 뺄셈 subtract(a, b) ==================== */
/* A - B를 계산하는 함수 */
BigNumber* subtract(const BigNumber* A, const BigNumber* B) {
    if (!A || !B) return NULL;
    
    /* 어느 쪽이 더 큰지 결정하기, Compare to determine which is larger */
    int comp = compare_bignumber(A, B);
    
    const BigNumber* larger;
    const BigNumber* smaller;
    int result_negative = 0;
    
    if (comp >= 0) {
        // 경우: A >= B...
        larger = A;
        smaller = B;
        result_negative = 0; // 양수의 결과
    } else {
        // 경우: A < B...
        larger = B;
        smaller = A;
        result_negative = 1; //음수의 결과(나중에 표시됨)
    }
    
    //정렬을 위한 길이와 스케일 구하기
    int lenL = 0, lenS = 0;
    for (DigitNode* p = larger->head; p; p = p->next) lenL++;
    for (DigitNode* p = smaller->head; p; p = p->next) lenS++;
    
    int fracL = larger->scale;
    int fracS = smaller->scale;
    
    int intL = lenL - fracL;
    int intS = lenS - fracS;
    
    /* 결과의 소수 자릿수 결정 (둘 중 최대값), Determine result's fractional digits (max of both) */
    int frac_res = (fracL > fracS ? fracL : fracS);
    /* 결과의 정수 자릿수 결정 (둘 중 최대값), Determine result's integer digits (max of both) */
    int int_res = (intL > intS ? intL : intS);
    
    /* 총 길이 */
    int L = int_res + frac_res;
    
    /* 정렬된 배열 생성, Create aligned arrays */
    int* l = calloc(L, sizeof(int));
    int* s = calloc(L, sizeof(int));
    
    /* 최대값 배열에 숫자 입력, Fill larger number */
    {
        DigitNode* p = larger->head;
        //오프셋 구하기: 정수 자릿수 부족시 0 패딩
        //Calculate offset: if larger has fewer integer digits, pad with zeros
        int off = int_res - intL;

        //정수 부분 채우기, fill integer part
        for (int i = 0; i < intL; i++) {
            l[off + i] = p->digit;
            p = p->next;
        }
        //소수 부분 채우기, fill fractional part
        for (int i = 0; i < fracL; i++) {
            l[int_res + i] = p->digit;
            p = p->next;
        }
    }
    
    /* 더 작은 숫자 배열 채우기, Fill smaller number array */
    {
        DigitNode* p = smaller->head;
        int off = int_res - intS; //  작은 숫자용 패딩

        for (int i = 0; i < intS; i++) {
            s[off + i] = p->digit;
            p = p->next;
        }
        for (int i = 0; i < fracS; i++) {
            s[int_res + i] = p->digit;
            p = p->next;
        }
    }
    
    /* 차용 처리 뺄셈, Perform subtraction with borrowing */
    int* R = calloc(L, sizeof(int)); // result array
    int borrow = 0;
    
    for (int i = L - 1; i >= 0; i--) {
        // 뺄셈: larger_digit - smaller_digit - previous_borrow
        int diff = l[i] - s[i] - borrow;
        
        if (diff < 0) {
            // 다음 높은 자릿수에서 빌려야 함, Need to borrow from next higher digit
            diff += 10; //  현재 자릿수에 10 더하기
            borrow = 1; // Set borrow for next digit
        } else {
            borrow = 0;
        }
        
        R[i] = diff;
    }
    
    free(l);
    free(s);
    
    /* BigNumber의 결과 만들기 */
    BigNumber* res = create_bignumber();
    res->is_negative = result_negative;
    
    //  결과에서 선행 0 건너뛰기, Skip leading zeros in result
    int start_index = 0;
    while (start_index < L && R[start_index] == 0) {
        start_index++;
    }
    
    //  모든 자릿수가 0이면 (결과가 0),
    if (start_index == L) {
        append_digit(res, 0, 0);
    } else {
        // 결과 배열을 연결 리스트 자릿수로 변환
        // Convert result array to linked list digits
        for (int i = start_index; i < L; i++) {
            // 이 자릿수가 소수 부분인지 확인
            // Determine if this digit is in fractional part
            int pos_from_right = L - i;  //from right end
            int in_fraction = (pos_from_right < frac_res);
            append_digit(res, R[i], in_fraction);
        }
    }
    
    free(R);
    return res;
}

/* ==================== calculate(postfix) ==================== */

static int is_op(const char* t, size_t len) {
    return (len == 1 && (t[0] == '+' || t[0] == '-' || t[0] == '*' || t[0] == '/'));
}

BigNumber* calculate(const char* post) {
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
            BigNumber* r = NULL;
            
            switch (s[0]) {
                case '+':
                    r = add(a, b);
                    break;
                case '-':
                    r = subtract(a, b);
                    break;
                // TODO:다른 연산자 구현
                case '*':
                case '/':
                default:
                    printf("Operator '%c' not implemented yet\n", s[0]);
                    r = create_bignumber();  // Return 0 as placeholder
                    append_digit(r, 0, 0);
                    break;
            }
            
            if (a) free_bignumber(a);
            if (b) free_bignumber(b);
            push_big(&st, r);
        } else {
            BigNumber* x = make_bignumber_from_token(s, len);
            push_big(&st, x);
        }
    }

    return pop_big(&st);
}

/* ==================== print_bignumber ==================== */

void print_bignumber(const BigNumber* bn) {
    if (!bn) {
        printf("(null)");
        return;
    }

    // 모든 자릿수가 0인지 확인
    // Check if all digits are zero
    int all_zero = 1;
    for (DigitNode* p = bn->head; p; p = p->next) {
        if (p->digit != 0) {
            all_zero = 0;
            break;
        }
    }
    
    if (all_zero) {
        printf("0");
        return;
    }

    //  음수 부호 출력, Print negative sign
    if (bn->is_negative) {
        printf("-");
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
