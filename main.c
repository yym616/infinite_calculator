#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calculate.h"
#include "preprocessing.h"

char* read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("file open failed");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(size + 1);
    fread(buf, 1, size, fp);
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("사용법: %s <입력파일>\n", argv[0]);
        return 0;
    }

    char* input = read_file(argv[1]);
    char* expr = preprocess(input);    // 공백 제거 + 암시적 곱셈 삽입
    char* postfix = infix_to_postfix(expr);

    BigNumber* result = evaluate_postfix(postfix);

    printf("Result = ");
    print_bignumber(result);
    printf("\n");

    free(input);
    free(expr);
    free(postfix);

    free_bignumber(result);

    return 0;
}
