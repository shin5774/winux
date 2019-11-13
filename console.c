
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define EOL 1
#define ARG 2
#define AMPERSAND 3
#define SEMICOLON 4
#define MAXARG 512
#define MAXBUF 512
#define FOREGROUND 0
#define BACKGROUND 1
#define PIPE 5

int childpid;                 // childpid를저장
int childpid2;                // childpid2를 저장
char *pipearg[2][MAXARG + 1]; // 파이프 라인 입력받았을때 명령어 저장할 곳
static char inpbuf[MAXBUF], tokbuf[2 * MAXBUF], *ptr = inpbuf, *tok = tokbuf;
char *prompt = "Command> ";
static char special[] = {' ', '\t', '&', ';', '\n', '\0'};
int inarg(char c);

int userin(char *p) {
    int c, count;
    ptr = inpbuf;
    tok = tokbuf;
    printf("%s", p);
    count = 0;
    while (1) {
        if ((c = getchar()) == EOF) {
            printf("eof = %c\n", c);
            return (EOF);
        }
        if (count < MAXBUF)
            inpbuf[count++] = c;
        if (c == '\n' && count < MAXBUF) {
            inpbuf[count] = '\0';
            return count;
        }
        if (c == '\n') {
            printf("smallsh: input line too long\n");
            count = 0;
            printf("%s", p);
        }
    }
}

int gettok(char **outptr) {
    int type;
    *outptr = tok;
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;
    *tok++ = *ptr;
    switch (*ptr++) {
    case '|': // pipe라인을 받으면 type에 PIPE대입
        type = PIPE;
        break;
    case '\n':
        type = EOL;
        break;
    case '&':
        type = AMPERSAND;
        break;
    case ';':
        type = SEMICOLON;
        break;
    default:
        type = ARG;
        while (inarg(*ptr))
            *tok++ = *ptr++;
    }
    *tok++ = '\0';
    return type;
}

int inarg(char c) {
    char *wrk;
    for (wrk = special; *wrk; wrk++) {
        if (c == *wrk)
            return (0);
    }
    return (1);
}

int procline(void) {

    int ntype = 0;
    int check;

    char *arg[MAXARG + 1];
    int toktype;
    int narg;
    int type;
    check = 0;
    narg = 0;

    for (;;) {
        toktype = gettok(&arg[narg]);
        switch (toktype) {

        case PIPE:
            for (int i = 0; i < narg; i++) {
                pipearg[0][i] = arg[i];
            }
            pipearg[0][narg] = NULL;
            narg = 0;
            check = toktype; // pipe를 입력받았는지 확인하는 변수 check
            break;

        case ARG:
            if (narg < MAXARG)
                narg++;
            break;
        case EOL:
        case SEMICOLON:
        case AMPERSAND:
            if (toktype == AMPERSAND)
                type = BACKGROUND;
            else
                type = FOREGROUND;

            if (narg != 0) {

                arg[narg] = NULL;

                if (check == PIPE) { // pipe를 받으면 pipearg에 첫번째 명령 //
                                     // (파이프 왼쪽)) 문자를 대입한다
                    for (int i = 0; i < narg; i++)
                        pipearg[1][i] = arg[i];
                    pipeexe();
                } else {

                    runcommand(arg, type);
                }
            }

            if (toktype == EOL) {
                return;
            }
            narg = 0;
            break;
        }
    }
}

int pipeexe() {

    pid_t pid[2];
    int i = 0, j;
    int fd[2], status;
    if (pipe(fd) == -1) {
        perror("error!");
        exit(1);
    }
    switch (pid[0] = fork()) {
    case -1:
        perror("error!");
        exit(1);
    case 0:
        close(1);   //표준출력 차단
        dup(fd[1]); //표준출력을 파이프의 입력으로
        close(fd[0]);
        close(fd[1]);

        execvp(pipearg[0][0], pipearg[0]); // pipe에 첫번째 명령 전달
    }
    switch (pid[1] = fork()) {
    case -1:
        perror("error!");
        exit(1);
    case 0:
        close(0);   //표준입력 차단
        dup(fd[0]); //표준입력을 파이프의 입력으로
        close(fd[0]);
        close(fd[1]);

        sleep(3); //^c를 누를 여유를 만듬

        execvp(pipearg[1][0],
               pipearg[1]); //전달받은 첫번째 명령과 함께 두번째 명령 실행
    }
    childpid = pid[1]; //부모 프로세스는 자식 프로세스 의 pid를
                       //저장받음(handler에서 쓰임)
    close(fd[0]);
    close(fd[1]);

    while (wait((int *)0) != -1)
        ;
    childpid = 0;
    return 0;
}

int runcommand(char **cline, int where) {
    int pid;
    int status;
    switch (pid = fork()) {
    case -1:
        perror("smallsh");
        return (-1);
    case 0:
        sleep(3); //^c를 누를 여유를 만듬
        execvp(*cline, cline);
        perror(*cline);
        exit(1);
    }
    childpid2 =
        pid; //부모 프로세스는 자식프로세스의 pid를 저장함 handler에서 쓰임
    if (where == BACKGROUND) {
        printf("[Process id %d]\n", pid);
        return (0);
    }

    if (waitpid(pid, &status, 0) == -1) {

        return (-1);
    } else {
        childpid2 = 0;
        return (status);
    }
}

// int redirect(char **temp, int i, int where) {
//     char *temp1[MAXARG], *temp2[MAXARG];
//     int j;
//     pid_t pid;
//     int fileId;

//     for (j = 0; j <= i - 2; j++)
//         temp1[j] = temp[j];
// }

// void handler(int sig) {

//     if (sig == SIGINT) {

//         if (childpid != 0) { // pipe에서 명령어가 실행중일경우
//                              // 명령어실행(자식)프로세스를 kill함
//             kill(childpid, 9);
//             printf("\n");
//         } else if (childpid2 != 0) { // pipe가 아닌경우임
//             kill(childpid2, 9);
//             printf("\n");
//         }
//     }
// }
int main() {
    // signal(SIGINT, handler); //^c 시그널을 받음

    while (userin(prompt) != EOF)
        procline();
}
