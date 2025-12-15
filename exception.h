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
static const char* error_msg(ErrorCode c) {
    switch (c) {
        case ERR_PAREN:    return "괄호 오류";
        case ERR_NUMBER:   return "숫자 오류";
        case ERR_OPERATOR: return "연산자 오류";
        case ERR_CHAR:     return "문자 오류";
        case ERR_UNKNOWN:  return "ERROR";
        case OK:           return NULL;   //정상일 땐 NULL
        default:           return "ERROR";
    }
}

//수식 검증
static ErrorCode error_test(const char *s) {
    if (!s) return ERR_UNKNOWN;

    int i = 0;
    int paren = 0;
    int sawToken = 0;

    /*prevType: 
                0= 시작, 연산자, '(',
                1= NUMBER,
                2= ')'  */
    int prevType = 0;

    while (s[i]) {
        char c = s[i];

        // 공백 스킵
        if (c==' ' || c=='\t' || c=='\n' || c=='\r') {
             i++; continue; 
        }

        //허용 문자 확인
        if (!(isdigit((unsigned char)c) || c=='.' || c=='+' || c=='-' || c=='*' || c=='/' || c=='(' || c==')')) {
            return ERR_CHAR;
        }

        // '('
        if (c == '(') {
            paren++;
            prevType = 0;
            sawToken = 1;
            i++;
            continue;
        }

        // ')'
        if (c == ')') {
            if (prevType == 0) return ERR_PAREN;
            paren--;
            if (paren < 0) return ERR_PAREN;
            prevType = 2;
            sawToken = 1;
            i++;
            continue;
        }

        // 연산자
        if (c=='+' || c=='-' || c=='*' || c=='/') {
            sawToken = 1;

            // 시작/연산자/'(' 뒤에 '*'는 올 수 없음
            if (prevType == 0 && (c == '*' || c == '/')) return ERR_OPERATOR;

            // 시작/연산자/'(' 뒤에 +,- 는 단항 가능
            if (prevType == 0 && (c=='+' || c=='-')) {
                int k = i + 1;
                while (s[k] && (s[k]==' '||s[k]=='\t'||s[k]=='\n'||s[k]=='\r')) k++;
                char n = s[k];

                if (!(isdigit((unsigned char)n) || n=='.' || n=='(')) return ERR_OPERATOR;

                i++;
                continue;
            }

            //숫자 또는 ')' 뒤의 연산자는 정상 이항 연산자
            if (prevType == 1 || prevType == 2) {
                prevType = 0;
                i++;
                continue;
            }

            return ERR_OPERATOR;
        }

        // 숫자 토큰
        if (isdigit((unsigned char)c) || c=='.') {
            sawToken = 1;

            int dotCount = 0;
            int digitCount = 0;

            while (s[i] && (isdigit((unsigned char)s[i]) || s[i]=='.')) {
                if (s[i]=='.') dotCount++;
                else digitCount++;
                if (dotCount > 1) return ERR_NUMBER;
                i++;
            }

            if (digitCount == 0) return ERR_NUMBER;
            prevType = 1;
            continue;
        }

        i++;
    }

    if (!sawToken) return ERR_UNKNOWN;
    if (paren != 0) return ERR_PAREN;
    if (prevType == 0) return ERR_OPERATOR;
    return OK;
}