#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <ctype.h>

//에러 코드
typedef enum {
    OK = 0,
    ERR_PAREN,     // 괄호 오류
    ERR_NUMBER,    // 숫자 오류(소수점 2개 등)
    ERR_OPERATOR,  // 연산자 오류(끝이 연산자, 연산자 과다 등)
    ERR_CHAR,      // 허용되지 않은 문자
    ERR_UNKNOWN    // 그 외
} ErrorCode;

//error 메시지
const char* error_msg(ErrorCode c);

//수식 검증
ErrorCode error_test(const char *s);

#endif
