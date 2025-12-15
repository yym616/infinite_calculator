#ifndef OPERATION_H
#define OPERATION_H

#include "calculate.h"

//사칙연산

BigNumber* add(const BigNumber *A, const BigNumber *B);
BigNumber* subtract(const BigNumber *A, const BigNumber *B);
BigNumber* multiply(const BigNumber *A, const BigNumber *B);
BigNumber* divide(const BigNumber *A, const BigNumber *B);

void reset_calc_error(void);
int get_calc_divzero(void);

#endif
