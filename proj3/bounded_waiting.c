/*
 * Copyright(c) 2021-2023 All rights reserved by Jihyeon Lee.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#define N 8             /* 스레드 개수 */
#define RUNTIME 100000  /* 출력량을 제한하기 위한 실행시간 (마이크로초) */

/*
 * ANSI 컬러 코드: 출력을 쉽게 구분하기 위해서 사용한다.
 * 순서대로 BLK, RED, GRN, YEL, BLU, MAG, CYN, WHT, RESET을 의미한다.
 */
char *color[N+1] = {"\e[0;30m","\e[0;31m","\e[0;32m","\e[0;33m","\e[0;34m","\e[0;35m","\e[0;36m","\e[0;37m","\e[0m"};

/*
 * turn은 임계구역에 들어가야 하는 스레드 i를 의미한다.
 * alive 값이 false가 될 때까지 스레드 내의 루프가 무한히 반복된다.
 */
bool alive = true;
atomic_bool lock = false;
bool expected = false;
int turn = 0;

/*
 * N 개의 스레드가 임계구역에 배타적으로 들어가기 위해 스핀락을 사용하여 동기화한다.
 */
void *worker(void *arg)
{
    int i = *(int *)arg;
    while (alive) {
    /* 
     * 임계구역에 스레드가 없을 경우 turn인 스레드가 진입 
     * 스레드의 i가 turn이 아니거나 lock이 잠겨있다면 진입 불가 
     */
	expected = false;
	while (turn != i || !atomic_compare_exchange_weak(&lock, &expected, true))
	  expected = false;          
	/*
	 * 임계구역: 알파벳 문자를 한 줄에 40개씩 10줄 출력한다.
	 */ 
	for (int k = 0; k < 400; ++k) {
		printf("%s%c%s", color[i], 'A'+i, color[N]);
		if ((k+1) % 40 == 0)
			printf("\n");
	}
	/*
	 * turn을 다음 스레드 순서로 바꿔준다.
	 * 알파벳 출력이 끝났으므로 lock을 해제한다.
	 */
	turn = (i + 1) % N;
	lock = false;         
	/*
	 * 임계구역이 성공적으로 종료되었다.
	 */
    }
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t tid[N];
    int i, id[N];

    /*
     * N 개의 자식 스레드를 생성한다.
     */
    for (i = 0; i < N; ++i) {
        id[i] = i;
        pthread_create(tid+i, NULL, worker, id+i);
    }
    /*
     * 스레드가 출력하는 동안 RUNTIME 마이크로초 쉰다.
     * 이 시간으로 스레드의 출력량을 조절한다.
     */
    usleep(RUNTIME);
    /*
     * 스레드가 자연스럽게 무한 루프를 빠져나올 수 있게 한다.
     */
    alive = false;
    /*
     * 자식 스레드가 종료될 때까지 기다린다.
     */
    for (i = 0; i < N; ++i)
        pthread_join(tid[i], NULL);
    /*
     * 메인함수를 종료한다.
     */
    return 0;
}