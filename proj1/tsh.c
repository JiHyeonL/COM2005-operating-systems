/*
 * Copyright(c) 2023 All rights reserved by Jihyeon Lee.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>





#define MAX_LINE 80             /* 명령어의 최대 길이 */


/*
 * cmdexec - 명령어를 파싱해서 실행한다.
 * 스페이스와 탭을 공백문자로 간주하고, 연속된 공백문자는 하나의 공백문자로 축소한다. 
 * 작은 따옴표나 큰 따옴표로 이루어진 문자열을 하나의 인자로 처리한다.
 * 기호 '<' 또는 '>'를 사용하여 표준 입출력을 파일로 바꾸거나,
 * 기호 '|'를 사용하여 파이프 명령을 실행하는 것도 여기에서 처리한다.
 */
	

static void cmdexec(char *cmd)
{

    char *argv[MAX_LINE/2+1];   /* 명령어 인자를 저장하기 위한 배열 */
    int argc = 0;               /* 인자의 개수 */
    char *p, *q;                /* 명령어를 파싱하기 위한 변수 */
  
    int l = 0;		/* '<' 체크용 위치 저장 변수 */
    int r = 0;		/* '>' 체크용 위치 저장 변수 */
    int pp = 0;		/* '|' 체크용 위치 저장 변수 */


    /*
     * 명령어 앞부분 공백문자를 제거하고 인자를 하나씩 꺼내서 argv에 차례로 저장한다.
     * 작은 따옴표나 큰 따옴표로 이루어진 문자열을 하나의 인자로 처리한다.
     */
    p = cmd; p += strspn(p, " \t");	// strspn: 문자까지의 거리 반환. 문자세트에 없는 문자 발견하면 종료
    do {
        /*
         * 공백문자, 큰 따옴표, 작은 따옴표가 있는지 검사한다.
         */ 
        q = strpbrk(p, " \t\'\"");	// strpbrk: 공백,큰&작은 따옴표 중 가장 먼저 발견된 문자 q에 대입
        /*
         * 공백문자가 있거나 아무 것도 없으면 공백문자까지 또는 전체를 하나의 인자로 처리한다.
         */
        if (q == NULL || *q == ' ' || *q == '\t') {
            q = strsep(&p, " \t");	// strsep: \t 기준으로 문자열 분리
            if (*q) argv[argc++] = q;	// q가 공백 문자 아니면 argv에 넣음
        }
        /*
         * 작은 따옴표가 있으면 그 위치까지 하나의 인자로 처리하고, 
         * 작은 따옴표 위치에서 두 번째 작은 따옴표 위치까지 다음 인자로 처리한다.
         * 두 번째 작은 따옴표가 없으면 나머지 전체를 인자로 처리한다.
         */
        else if (*q == '\'') {
            q = strsep(&p, "\'");
            if (*q) argv[argc++] = q;
            q = strsep(&p, "\'");
            if (*q) argv[argc++] = q;
        }

        /*
         * 큰 따옴표가 있으면 그 위치까지 하나의 인자로 처리하고, 
         * 큰 따옴표 위치에서 두 번째 큰 따옴표 위치까지 다음 인자로 처리한다.
         * 두 번째 큰 따옴표가 없으면 나머지 전체를 인자로 처리한다.
         */
        else {
            q = strsep(&p, "\"");
            if (*q) argv[argc++] = q;
            q = strsep(&p, "\"");
            if (*q) argv[argc++] = q;
		}
		
		
		/* 
		 * 리다이렉션, 파이프 발견 시 인덱스 저장
		 */
		if (*q == '<') {	/* '<' index 저장 */
			l = argc-1;		
		}

		if (*q == '>') {	/* '>' index 저장 */
			r = argc-1;		
		}
		
		if (*q == '|') {	/* '|' index 저장 */
			pp = argc-1;	
			break;			/* 파이프 등장하면 커맨드 슬라이싱 중단 */
		}
      
    } while (p);	
    /* strsep 함수에 p가 사용되기 때문에 p값은 계속 작아진다. 슬라이싱이 끝나면 p = NULL이 된다. */
    
    argv[argc] = NULL;	
	
	
	/*
	 * 파이프 발견 
	 */
	if(pp > 0) {	
		argv[pp] = NULL;	/* | 기호 없애주기 */
		int fd[2];
		
		if (pipe(fd) == -1) {		/* 파이프 생성 실패 */
			printf("PIPE A ERROR\n");
			fprintf(stderr, "Pipe A failed");
			exit(1);
		}

		pid_t pid;
		pid = fork();	/* 자식 생성 */
		
		if (pid < 0) {	/* 자식 생성 실패 */
			printf("FORK ERROR\n");	
			fprintf(stderr, "Fork failed");
			exit(1);
		}
		
		/* 
		 * 자식 프로세스. int main까지 합하면 손자. 
		 * sprintf를 사용하여 포인터 배열 argv를 문자열로 바꾼 후(command1 역할)
		 * buffer에 저장해 cmdexec 함수 호출
		 */
		else if (pid == 0) {	
		  	char buffer[MAX_LINE];
		  	int leng = 0;
		  	
		  	for (int i = 0; i < argc; i++) {	
		  		leng += sprintf(buffer+leng,"%s",argv[i]);
		  	}
			
			dup2(fd[1], 1); /* 1 = stdout 대신 fd[1] 사용(write) */
		  	close(fd[0]);
		  	
		  	cmdexec(buffer);
		}
		
		/* 
		 * 부모 프로세스. int main까지 합하면 자식.
		 * 자식이 끝날 때 까지 기다린 뒤 아직 슬라이싱 되지 않은 커맨드 문자열인 
		 * p(command_x 역할)를 사용해 cmdexec 함수 호출 
		 */
		else {		
			  wait(NULL);
		      dup2(fd[0], 0); /* 0 = stdin 대신 fd[0] 사용(read) */
		      close(fd[1]);

		      cmdexec(p);
		}
	}
	
	
	/* 
	 * 리다이렉션 발견
	 */	
	if (l > 0) {	/* '<'가 있다면 */
		argv[l] = NULL;		/* < 기호 없애주기 */
		int fd_l = open(argv[l+1], O_RDWR | O_CREAT, 0644);	/* 읽어 올 파일 열기 */
		dup2(fd_l, STDIN_FILENO);		/* 열린 파일에서 입력 읽기 */
		close(fd_l); 
	}
	
	if (r > 0) {	/* '>'가 있다면 */
		argv[r] = NULL;		/* > 기호 없애주기 */
		int fd_r = open(argv[r+1], O_RDWR | O_CREAT, 0644);	/* 저장할 파일 생성 */
		dup2(fd_r, STDOUT_FILENO);	/* 생성한 파일에 출력 저장 */
		close(fd_r); 
	}
	
	
	/* 
	 * 명령 실행
	 */
	if (argc > 0) {
		execvp(argv[0], argv);	
	}
}



/*
 * 기능이 간단한 유닉스 셸인 tsh (tiny shell)의 메인 함수이다.
 * tsh은 프로세스 생성과 파이프를 통한 프로세스간 통신을 학습하기 위한 것으로
 * 백그라운드 실행, 파이프 명령, 표준 입출력 리다이렉션 일부만 지원한다.
 */
int main(void)
{
    char cmd[MAX_LINE+1];       /* 명령어를 저장하기 위한 버퍼 */
    int len;                    /* 입력된 명령어의 길이 */
    pid_t pid;                  /* 자식 프로세스 아이디 */
    int background;             /* 백그라운드 실행 유무 */
    
    /*
     * 종료 명령인 "exit"이 입력될 때까지 루프를 무한 반복한다.
     */
    while (true) {
        /*
         * 좀비 (자식)프로세스가 있으면 제거한다.
         */
        pid = waitpid(-1, NULL, WNOHANG);	// -1:특정 프로세스가 아니라 아무 child도 상관없음 -> non blocking(no stop)
        if (pid > 0)
            printf("[%d] + done\n", pid);	// zombie process end. if erase zombie code, use "exit"
        /*
         * 셸 프롬프트를 출력한다. 지연 출력을 방지하기 위해 출력버퍼를 강제로 비운다.
         */
        printf("tsh> "); fflush(stdout);
        /*
         * 표준 입력장치로부터 최대 MAX_LINE까지 명령어를 입력 받는다.
         * 입력된 명령어 끝에 있는 새줄문자를 널문자로 바꿔 C 문자열로 만든다.
         * 입력된 값이 없으면 새 명령어를 받기 위해 루프의 처음으로 간다.
         */

	
        len = read(STDIN_FILENO, cmd, MAX_LINE);
        if (len == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        cmd[--len] = '\0';
        if (len == 0)		// input 없으면 새 명령어 받으러 감
            continue;


        /*
         * 종료 명령이면 루프를 빠져나간다.
         */
        if(!strcasecmp(cmd, "exit"))	// 종료
            break;
        /*
         * 백그라운드 명령인지 확인하고, '&' 기호를 삭제한다.
         */
        char *p = strchr(cmd, '&');
        if (p != NULL) {		// 포그라운드일 경우 
            background = 1;
            *p = '\0';
        }
        else
            background = 0;
        /*
         * 자식 프로세스를 생성하여 입력된 명령어를 실행하게 한다.
         */
        if ((pid = fork()) == -1) {
            perror("fork");		// 화면상에 오류 메세지 fork 출력
            exit(EXIT_FAILURE);
        }
        /*
         * 자식 프로세스는 명령어를 실행하고 종료한다.
         */
        else if (pid == 0) {
            cmdexec(cmd);
            exit(EXIT_SUCCESS);
        }
        /*
         * 포그라운드 실행이면 부모 프로세스는 자식이 끝날 때까지 기다린다.
         * 백그라운드 실행이면 기다리지 않고 다음 명령어를 입력받기 위해 루프의 처음으로 간다. 		 
         * 만약 자식이 zombie되면? => 위에 없애는 과정 있음
         */
        else if (!background)	// 포그라운드 실행(background = 0)
            waitpid(pid, NULL, 0);
    }
    return 0;
}
