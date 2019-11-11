#include<stdio.h>
#include<string.h>

/*
	문자열 p : 현재 디렉토리에 존재하는 파일들 목록
	flag : 색이 칠해질 p의 인덱스
	cc : 방향키 (임시적으로 a 와 s로 움직임)
*/

char* p[10]={"mypc","LOL","paint","internet","study"};
int flag=0;
char cc;

//색상 on
void colorOn(){
	printf("\033[1;33m");
}

//색상 off
void colorOff(){
	printf("\033[0m");
}

//flag값 조정
void getFlag(){
	if(cc=='a'){
		if(flag!=0)
			flag--;
	}else if(cc=='s'){
		if(flag!=5)
			flag++;
	}
}

//ui 출력 함수
void start(){
	printf("---------------------------------------------------------------------------------------------------\n");
	printf("|");
	for(int i=0;i<5;i++){		//일시적으로 5번
		if(flag==i)				//flag 와 i가 일치할 시 색상 on
			colorOn();
		printf("%-10s",p[i]);	//p출력
		if(flag==i)
			colorOff();			//색상 off
	}
	printf("\n");
	printf("---------------------------------------------------------------------------------------------------\n");
}


//main
int main(){
	while(1){	
		start();
		cc=getchar();	//a or s 입력
		getchar();		//뉴라인 제거
		getFlag();		//
	}
	return 0;
}
