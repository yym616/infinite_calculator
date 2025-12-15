#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* ===============================
 *  절댓값 연산
 * =============================== */

static BigNumber* add_abs(const BigNumber *A, const BigNumber *B) {
    BigNumber *R = new_bignumber();
    R->scale = A->scale;
    int carry = 0;
    DigitNode *a = A->head, *b = B->head;

    while (a || b || carry) {
        int s = (a?a->digit:0) + (b?b->digit:0) + carry;
        carry = s / 10;
        push_msb(R, s % 10);
        if (a) a = a->next;
        if (b) b = b->next;
    }
    normalize(R);
    return R;
}

static BigNumber* sub_abs(const BigNumber *A, const BigNumber *B) {
    BigNumber *R = new_bignumber();
    R->scale = A->scale;
    int borrow = 0;
    DigitNode *a = A->head, *b = B->head;

    while (a || b) {
        int d = (a?a->digit:0) - borrow - (b?b->digit:0);
        if (d < 0) { d += 10; borrow = 1; } else borrow = 0;
        push_msb(R, d);
        if (a) a = a->next;
        if (b) b = b->next;
    }
    normalize(R);
    return R;
}

/* ===============================
 *  사칙연산
 * =============================== */

static BigNumber* add(const BigNumber *A, const BigNumber *B) {
    BigNumber *a = clone_bignumber(A);
    BigNumber *b = clone_bignumber(B);
    align_scales(a, b);

    BigNumber *R;
    if (a->sign == b->sign) {
        R = add_abs(a, b);
        R->sign = a->sign;
    } else {
        int c = compare_abs(a, b);
        if (c == 0) {
            R = new_bignumber();
            push_msb(R, 0);
        } else if (c > 0) {
            R = sub_abs(a, b);
            R->sign = a->sign;
        } else {
            R = sub_abs(b, a);
            R->sign = b->sign;
        }
    }

    free_bignumber(a);
    free_bignumber(b);
    normalize(R);
    return R;
}

static BigNumber* subtract(const BigNumber *A, const BigNumber *B) {
    BigNumber *nb = clone_bignumber(B);
    nb->sign *= -1;
    BigNumber *R = add(A, nb);
    free_bignumber(nb);
    return R;
}

static BigNumber* multiply(const BigNumber *A, const BigNumber *B) {
    BigNumber *a = clone_bignumber(A);
    BigNumber *b = clone_bignumber(B);

    int la = length_digits(a), lb = length_digits(b);
    int *arr = calloc(la + lb + 2, sizeof(int));

    int i = 0;
    for (DigitNode *da = a->head; da; da = da->next, i++) {
        int j = 0;
        for (DigitNode *db = b->head; db; db = db->next, j++) {
            arr[i + j] += da->digit * db->digit;
        }
    }

    for (int k = 0; k < la + lb + 1; k++) {
        arr[k+1] += arr[k] / 10;
        arr[k] %= 10;
    }

    BigNumber *R = new_bignumber();
    R->sign = a->sign * b->sign;
    R->scale = a->scale + b->scale;

    for (int k = 0; k < la + lb + 1; k++)
        push_msb(R, arr[k]);

    free(arr);
    free_bignumber(a);
    free_bignumber(b);
    normalize(R);
    return R;
}

static int calc_divzero = 0;

static void reset_calc_error(void) {
    calc_divzero = 0;
}

static int get_calc_divzero(void) {
    return calc_divzero;
}

static void strip_leading_zeros_int(int *a, int *len) {
    int i = 0;
    while (i < *len - 1 && a[i] == 0) i++;
    if (i > 0) {
        memmove(a, a + i, (*len - i) * sizeof(int));
        *len -= i;
    }
}

static int cmp_int(const int *a, int la, const int *b, int lb) {
    while (la > 1 && a[0] == 0) { a++; la--; }
    while (lb > 1 && b[0] == 0) { b++; lb--; }
    if (la != lb) return (la > lb) ? 1 : -1;
    for (int i = 0; i < la; i++) {
        if (a[i] != b[i]) return (a[i] > b[i]) ? 1 : -1;
    }
    return 0;
}

static void sub_int_inplace(int *a, int *la, const int *b, int lb) {
    int ia = *la - 1;
    int ib = lb - 1;
    int borrow = 0;

    while (ia >= 0) {
        int av = a[ia] - borrow;
        int bv = (ib >= 0) ? b[ib] : 0;
        int d = av - bv;
        if (d < 0) { d += 10; borrow = 1; }
        else borrow = 0;
        a[ia] = d;
        ia--;
        ib--;
    }

    strip_leading_zeros_int(a, la);
}

static void mul_int_digit(const int *b, int lb, int digit, int *out, int *lo) {
    if (digit == 0) {
        out[0] = 0;
        *lo = 1;
        return;
    }

    int carry = 0;
    int k = 0;
    for (int i = lb - 1; i >= 0; i--) {
        int v = b[i] * digit + carry;
        out[k++] = v % 10;
        carry = v / 10;
    }
    while (carry) {
        out[k++] = carry % 10;
        carry /= 10;
    }

    for (int i = 0; i < k / 2; i++) {
        int t = out[i];
        out[i] = out[k - 1 - i];
        out[k - 1 - i] = t;
    }
    *lo = k;
}

static void bn_to_int_msb(const BigNumber *n, int **out, int *len) {
    int L = length_digits(n);
    if (L <= 0) {
        *out = malloc(sizeof(int));
        (*out)[0] = 0;
        *len = 1;
        return;
    }
    int *a = malloc(L * sizeof(int));
    int i = 0;
    for (DigitNode *c = n->tail; c; c = c->prev) a[i++] = c->digit;
    int la = L;
    strip_leading_zeros_int(a, &la);
    *out = a;
    *len = la;
}

static BigNumber* build_bn_from_int_msb(const int *q, int lq, int sign, int scale) {
    BigNumber *R = new_bignumber();
    R->sign = sign;
    R->scale = scale;
    for (int i = lq - 1; i >= 0; i--) push_msb(R, q[i]);
    normalize(R);
    return R;
}

static BigNumber* divide(const BigNumber *A, const BigNumber *B) {
    if (is_zero(B)) {
        calc_divzero = 1;
        return NULL;
    }

    const int PREC = 30;

    int *a = NULL, la = 0;
    int *b = NULL, lb = 0;
    bn_to_int_msb(A, &a, &la);
    bn_to_int_msb(B, &b, &lb);

    int na = la + B->scale + PREC;
    int nb = lb + A->scale;

    int *num = calloc(na, sizeof(int));
    int *den = calloc(nb, sizeof(int));

    memcpy(num, a, la * sizeof(int));
    for (int i = la; i < na; i++) num[i] = 0;

    memcpy(den, b, lb * sizeof(int));
    for (int i = lb; i < nb; i++) den[i] = 0;

    strip_leading_zeros_int(num, &na);
    strip_leading_zeros_int(den, &nb);

    int *quot = malloc((na + 1) * sizeof(int));
    int lq = 0;

    int *rem = malloc((na + 1) * sizeof(int));
    int lr = 1;
    rem[0] = 0;

    int *tmp = malloc((nb + 2) * sizeof(int));
    int lt = 0;

    for (int i = 0; i < na; i++) {
        if (!(lr == 1 && rem[0] == 0)) {
            rem[lr] = num[i];
            lr++;
        } else {
            rem[0] = num[i];
            lr = 1;
        }
        strip_leading_zeros_int(rem, &lr);

        int qd = 0;
        if (cmp_int(rem, lr, den, nb) >= 0) {
            for (int dgt = 9; dgt >= 1; dgt--) {
                mul_int_digit(den, nb, dgt, tmp, &lt);
                if (cmp_int(rem, lr, tmp, lt) >= 0) {
                    qd = dgt;
                    sub_int_inplace(rem, &lr, tmp, lt);
                    break;
                }
            }
        }
        quot[lq++] = qd;
    }

    strip_leading_zeros_int(quot, &lq);
    if (lq == 0) { quot[0] = 0; lq = 1; }

    int sign = A->sign * B->sign;
    BigNumber *R = build_bn_from_int_msb(quot, lq, sign, PREC);

    free(a);
    free(b);
    free(num);
    free(den);
    free(quot);
    free(rem);
    free(tmp);

    normalize(R);
    return R;
}