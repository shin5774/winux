#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_SPACE 98
int getch() {
    int c;
    struct termios oldattr, newattr;

    tcgetattr(STDIN_FILENO, &oldattr); // í˜„ì¬ í„°ë¯¸ë„ ì„¤ì • ì½ìŒ
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO); // CANONICALê³¼ ECHO ë”
    newattr.c_cc[VMIN] = 1;  // ìµœì†Œ ì…ë ¥ ë¬¸ì ìˆ˜ë¥¼ 1ë¡œ ì„¤ì •
    newattr.c_cc[VTIME] = 0; // ìµœì†Œ ì½ê¸° ëŒ€ê¸° ì‹œê°„ì„ 0ìœ¼ë¡œ ì„¤ì •
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr); // í„°ë¯¸ë„ì— ì„¤ì • ì…ë ¥
    c = getchar();                              // í‚¤ë³´ë“œ ì…ë ¥ ì½ìŒ
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr); // ì›ë˜ì˜ ì„¤ì •ìœ¼ë¡œ ë³µêµ¬
    return c;
}

char *p[20] = {"calculate","movie","battleGround","mypc", "LOL", "paint", "internet", "study","asdf"};

int flag = 0;
char cc;


int getsize(){	//ÆÄÀÏ ÀÌ¸§À» ´ãÀº ¹è¿­ÀÇ ±æÀÌ 
	char** ptr=p;
	int size=0;
	while(*ptr!=NULL){
		size++;
		*ptr++;
	}
	return size;
}

int getmax(){	//ÆÄÀÏ ÀÌ¸§Áß °¡Àå ±ä ÀÌ¸§ÀÇ ±æÀÌ +2 ¹İÈ¯ 
	int max=0;
	for(int i=1;i<getsize();i++){
		if(strlen(p[max])<strlen(p[i]))
			max=i;
	}
	return strlen(p[max])+2;
}

void colorOn() { printf("\033[1;33m"); }
void colorOff() { printf("\033[0m"); }

void getFlag() {
    if (cc == 'a') {
        if (flag != 0)
            flag--;
    } else if (cc == 's') {
        if (flag != getsize())
            flag++;
    }
}

void start() {
    int space=getmax();
    int num=MAX_SPACE / space;
    int sur=MAX_SPACE % space;

    printf("----------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < getsize();i++) {
	/*
	if((i+1)% num==1)
		printf("|");
	*/	 
	if (flag == i)
            colorOn();
	printf("%-*s",space,p[i]);
	if (flag == i)
            colorOff();

	if((i+1)% num==0)
		printf("\n");
    }
    printf("\n");
    printf("----------------------------------------------------------------------------------------------------\n");
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
