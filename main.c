#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
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
    start();
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
        system("clear");
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

        system("clear");
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
int i = 0;      // 파일및 디렉토리 개수
int flag = 0;   // 커서 인자
char cc;        // 커서 입력
char s[1024];   // ls 출력결과를 임시로 저장할 문자열
char *ls[50];   // 최종적으로 이 문자열배열에 저장될거임
char cwd[1024]; //현재 디렉토리 주소

int rmdirs(const char *path, int force) {
    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;
    struct stat buf;
    char filename[1024];

    if ((dir_ptr = opendir(path)) == NULL) {
        return unlink(path);
    }
    while ((file = readdir(dir_ptr)) != NULL) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
            continue;
        }
        sprintf(filename, "%s/%s", path, file->d_name);
        if (lstat(filename, &buf) == -1) {
            continue;
        }
        if (S_ISDIR(buf.st_mode)) {
            if (rmdirs(filename, force) == -1 && !force) {
                return -1;
            }
        } else if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            if (unlink(filename) == -1 && !force) {
                return -1;
            }
        }
    }
    closedir(dir_ptr);
    return rmdir(path);
}

int isdir(const char *path) { //디렉토리인지 확인
    struct stat s;
    stat(path, &s);
    if (S_ISDIR(s.st_mode))
        return 1;
    else {
        return 0;
    }
}

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

void colorOn() { printf("\033[1;33m"); }
void colorOn2() { printf("\033[1;34m"); }
void colorOff() { printf("\033[0m"); }
void colorOff2() { printf("\033[0m"); }

void getFlag() {
    system("clear");
    if (cc == 'a') { // 왼쪽 으로 한칸 이동
        if (flag != 0)
            flag--;
    } else if (cc == 'd') { // 오른쪽 으로 한칸 이동
        if (flag != i - 2)
            flag++;
    } else if (cc == 's') { // 아래쪽으로 한칸 이동
        if (flag < i - 5)
            flag += 4;
    } else if (cc == 'w') { // 위쪽으로 한칸 이동
        if (flag >= 4)
            flag -= 4;
    } else if (cc == '\n') {
        char newcwd[1024];
        char cat[1024] = "cat ";
        strcpy(newcwd, cwd);
        strcat(newcwd, "/");
        strcat(newcwd, ls[flag]);
        if (isdir(newcwd)) {
            chdir(ls[flag]);
            flag = 0; //초기화
            system("clear");
        } else {
            strcat(cat, ls[flag]);
            system("clear");
            system(cat);
        }
    } else if (cc == 'b') {
        chdir("..");
        system("clear");
    } else if (cc == 'r') {
        char newcwd[1024];
        strcpy(newcwd, cwd);
        strcat(newcwd, "/");
        strcat(newcwd, ls[flag]);
        if (isdir(newcwd)) {
            rmdirs(newcwd, 1);
        } else
            remove(newcwd);
    } else if (cc == 'c') {

        userin(prompt);

        procline();
    }
}

void start() {

    int fd = open("temp", O_CREAT | O_RDWR, 0755); // temp파일 열기
    int original_stdout = dup(1); //원래의 표준출력저장
    dup2(fd, 1);                  //표준출력을 파일로
    system("ls");                 //출력 이때 파일로 써짐
    fflush(stdout);
    close(fd);                //파일닫고
    dup2(original_stdout, 1); //저장했던 원래의 표준출력 되돌리고

    if (0 < (fd = open("temp", O_RDONLY))) { //파일 다시열어서 s에 저장
        read(fd, s, sizeof(s));
        close(fd);
    }

    char *ptr =
        strtok(s, "\n"); // s에 있는내용을 \n단위로 끊어서 ls[]에 저장하는코드
    int max = -1;
    i = 0;
    while (ptr != NULL) {
        ls[i] = ptr;
        ptr = malloc(1024);
        ptr = strtok(NULL, "\n");
        i++;
    }
    // system("clear");
    printf("-------------------------------------------------------------------"
           "--------------------------------\n");

    getcwd(cwd, sizeof(cwd));
    printf(" ");
    printf("%s\n", cwd);

    printf("-------------------------------------------------------------------"
           "--------------------------------\n");
    printf(" ");
    for (int k = 0; k < i - 1; k++) {
        char newcwd[1024];
        strcpy(newcwd, cwd);
        strcat(newcwd, "/");
        if (flag == k) {
            colorOn();
            printf("%-25s", ls[k]);
            colorOff();
            continue;
        } else if (isdir(strcat(newcwd, ls[k]))) {
            colorOn2();
            printf("%-25s", ls[k]);
            colorOff2();
        } else {
            printf("%-25s", ls[k]);
        }
    }
    printf("\n");
    printf("-------------------------------------------------------------------"
           "--------------------------------\n");
}

int main() {
    char mov;
    system("clear");
    while (1) {
        start();
        cc = getch();
        system("clear");
        getFlag();
    }
    return 0;
}
