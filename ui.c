#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int getch() {
    int c;
    struct termios oldattr, newattr;

    tcgetattr(STDIN_FILENO, &oldattr); // 현재 터미널 설정 읽음
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO); // CANONICAL과 ECHO 끔
    newattr.c_cc[VMIN] = 1;  // 최소 입력 문자 수를 1로 설정
    newattr.c_cc[VTIME] = 0; // 최소 읽기 대기 시간을 0으로 설정
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr); // 터미널에 설정 입력
    c = getchar();                              // 키보드 입력 읽음
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr); // 원래의 설정으로 복구
    return c;
}

char *p[10] = {"mypc", "LOL", "paint", "internet", "study"};
int flag = 0;
char cc;

void colorOn() { printf("\033[1;33m"); }

void colorOff() { printf("\033[0m"); }

void getFlag() {
    if (cc == 'a') {
        if (flag != 0)
            flag--;
    } else if (cc == 's') {
        if (flag != 5)
            flag++;
    }
}
void start() {
    printf("-------------------------------------------------------------------"
           "--------------------------------\n");
    printf("|");
    for (int i = 0; i < 5; i++) {
        if (flag == i)
            colorOn();
        printf("%-10s", p[i]);
        if (flag == i)
            colorOff();
    }
    printf("\n");
    printf("-------------------------------------------------------------------"
           "--------------------------------\n");
}

int main() {
    char mov;
    while (1) {
        start();
        cc = getch();

        getFlag();
        system("clear");
    }
    return 0;
}
