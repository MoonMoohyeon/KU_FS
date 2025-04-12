#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXS 1024

// statically defined input array (예시: 알파벳 반복 문자열)
char input[MAXS] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int pipefd[2];
int readv;

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("not valid argc\n");
        return -1;
    }

    char* target = argv[1];
    int processNum = atoi(argv[2]);
    int length = strlen(target);
    int targetLength = length;

    // 각 프로세스가 탐색할 기본 길이 (배열을 균등 분할)
    int dlen = MAXS / processNum;
    // 나머지 길이 (마지막 프로세스에 추가)
    int mod = MAXS - processNum * dlen;
    // dividedLength는 dlen에 target의 길이를 더한 값으로 설정 (경계 넘침 고려)
    int dividedLength = dlen + length;

    // 프로세스 수가 너무 많아 target이 포함될 수 없는 경우 체크
    if(length * processNum > MAXS) {
        printf("not valid pnum\n");
        return -1;
    }

    if(pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // startingInterval: 각 자식 프로세스가 탐색할 시작 인덱스 간격 (기본 dlen로 설정)
    int startingInterval = dlen;

    // pid 배열은 원래 코드에서 MAXS 크기로 선언되어 있으므로 그대로 사용
    pid_t pid[MAXS];

    for(int i = 0; i < processNum; i++) {
        // 자식가 사용할 텍스트 버퍼: dividedLength에 추가로 마지막 프로세스의 mod를 고려하여 할당
        char text[dividedLength + 1 + mod];
        if((pid[i] = fork()) == 0) {  // 자식 프로세스
            if(i == processNum - 1) {
                // 마지막 프로세스: 남은 모든 문자를 포함하도록 mod를 추가함
                printf("enter point\n");
                strncpy(text, input + startingInterval * i, dividedLength + mod);
                text[dividedLength + mod] = '\0';
            } else {
                strncpy(text, input + startingInterval * i, dividedLength);
                text[dividedLength] = '\0';
            }
            // 자식 프로세스에서 탐색할 문자열 길이 (경계 고려)
            int local_dlen = strlen(text);
            printf("text = %s, dividedLength = %d, dlen = %d, startingInterval*i = %d\n",
                   text, dividedLength, local_dlen, startingInterval * i);

            // 선형 탐색: text 내에서 target 문자열을 찾음
            for(int j = 0; j <= local_dlen - targetLength; j++) {
                int k;
                for(k = 0; k < targetLength; k++) {
                    if(text[j + k] != target[k])
                        break;
                }
                if(k == targetLength) {
                    int temp = j + i * startingInterval;
                    if(write(pipefd[1], &temp, sizeof(int)) == sizeof(int)) {
                        printf("write success %d\n", temp);
                    }
                }
            }
            close(pipefd[1]);
            close(pipefd[0]);
            exit(0);
        }
    }  // 자식 프로세스 종료

    for(int i = 0; i < processNum; i++) { // 부모 프로세스: 모든 자식이 종료될 때까지 기다림
        wait(NULL);
    }

    close(pipefd[1]);

    // 파이프에서 자식들이 전달한 인덱스 값을 읽어 배열에 저장
    int readArray[1024];
    int count = 0;
    while(read(pipefd[0], &readv, sizeof(int)) > 0) {
        readArray[count++] = readv;
    }
    close(pipefd[0]);

    // 오름차순 정렬 (선택 정렬 사용)
    for (int i = 0; i < count - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < count; j++) {
            if (readArray[j] < readArray[minIndex]) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            int temp = readArray[i];
            readArray[i] = readArray[minIndex];
            readArray[minIndex] = temp;
        }
    }

    printf("Found indices: "); // 결과 출력: 찾은 인덱스들을 오름차순으로 출력
    for(int i = 0; i < count; i++) {
        printf("%d ", readArray[i]);
    }
    printf("\n");
    return 0;
}
