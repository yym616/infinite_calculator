#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calculate.h"
#include "preprocessing.h"
#include "exception.h"
#include <stdlib.h>

int main(void) {
    system("chcp 65001");
    system("cls");
    int problem = 0;        //문제 번호
    char *line;             //한 줄=한 문제

    while ((line = read_line_dynamic(stdin)) != NULL) {
        if (is_blank_line(line)) {      //빈 줄은 건너 뜀
            free(line);
            continue;
        }

        problem++;

        //예외처리
        ErrorCode ec = error_test(line);
        if (ec != OK) {
            char *expr0 = preprocess(line);
            const char *msg = error_msg(ec);
            printf("문제%d:%s\n", problem, expr0);
            printf("ERROR:%s\n", msg ? msg : "ERROR");
            printf("---------------------------------------------------------------\n");
            free(line);
            free(expr0);
            continue;
        }

        //전처리: 계산에 실제 사용하는 식(공백 제거 + 괄호 곱셈 반영)
        char *expr = preprocess(line);
        char *postfix = infix_to_postfix(expr);

        reset_calc_error();

        //메인 후위식 계산
        BigNumber *result = evaluate_postfix(postfix);

        //문제와 정답 출력
        printf("문제%d:%s\n", problem, expr);
        if (result) {
            printf("정답:");
            print_bignumber(result);
            printf("\n");
        } 
        else {
            if (get_calc_divzero()) printf("ERROR:나눗셈 오류\n");
            else printf("정답:ERROR\n");
        }
        printf("---------------------------------------------------------------\n");

        //동적 메모리 반환
        free(line);
        free(expr);
        free(postfix);
        free_bignumber(result);
    }

    return 0;
}

