#include <stdio.h>
#include <stdlib.h>
#include "preprocessing.h"
#include "calculate.h"
int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "사용법: %s <파일이름>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("파일 오픈 실패");
        return 1;
    }

    char* line = read_line(fp);
    if (line == NULL) {
        printf("입력을 읽지 못했습니다.\n");
        fclose(fp);
        return 1;
    }

    // 개행(\n)까지 같이 읽혀 있으면 길이/출력에 영향 줄 수 있으니 정리
    int len = (int)strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
        len--;
    }

    char* post = postfix(line, len);
    if (post == NULL) {
        printf("postfix 변환 실패\n");
        free(line);
        fclose(fp);
        return 1;
    }

    printf("infix  : %s\n", line);
    printf("postfix: %s\n", post);  // 🔥 여기서 결과 확인

    BigNumber* res = calculate(post);
    print_bignumber(res);
    free(post);
    free(line);
    fclose(fp);
    return 0;
}
