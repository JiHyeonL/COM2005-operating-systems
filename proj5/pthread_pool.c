/*
 * Copyright(c) 2021-2023 All rights reserved by Jihyeon Lee.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */
#include "pthread_pool.h"
#include <stdlib.h>


/*
 * 풀에 있는 일꾼(일벌) 스레드가 수행할 함수이다.
 * FIFO 대기열에서 기다리고 있는 작업을 하나씩 꺼내서 실행한다.
 * 대기열에 작업이 없으면 새 작업이 들어올 때까지 기다린다.
 * 이 과정을 스레드풀이 종료될 때까지 반복한다.
 */
static void *worker(void *param)
{
	// 스레드풀 생성
    pthread_pool_t *pool = param;
    
    // 스레드풀 POOL_DISCARD 종료될 때까지 반복
    while(pool->running < 2) {
        // 대기열 접근을 위한 lock
		pthread_mutex_lock(&pool->mutex);   	
    	// 종료 명령 없고 대기열에 작업 없는 경우 wait
		while (pool->q_len == 0 && pool->running == 0){
            pthread_cond_wait(&pool->full, &pool->mutex);
        }		
    	// 바로 중단 
		if(pool->running == 2) {
			pthread_mutex_unlock(&pool->mutex);
			pthread_exit(NULL);
		}
		// 모두 실행 후 종료 
		else if(pool->running == 1 && pool->q_len == 0) {
			pthread_mutex_unlock(&pool->mutex);
			pthread_exit(NULL);			
		}
				
    	// 작업 하나 꺼낸 뒤 다음 실행 위치, 대기열 길이 재설정
    	task_t task = (pool->q)[pool->q_front];
    	pool->q_front = (pool->q_front + 1) % pool->q_size;
    	pool->q_len = pool->q_len -1;
    	// 작업 실행
    	task.function(task.param);
		// 락 해제 
    	pthread_cond_signal(&pool->empty);
    	pthread_mutex_unlock(&pool->mutex);
    }
    pthread_exit(NULL);
}

/*
 * 스레드풀을 생성한다. bee_size는 일꾼(일벌) 스레드의 개수이고, queue_size는 대기열의 용량이다.
 * bee_size는 POOL_MAXBSIZE를, queue_size는 POOL_MAXQSIZE를 넘을 수 없다.
 * 일꾼 스레드와 대기열에 필요한 공간을 할당하고 변수를 초기화한다.
 * 일꾼 스레드의 동기화를 위해 사용할 상호배타 락과 조건변수도 초기화한다.
 * 마지막 단계에서는 일꾼 스레드를 생성하여 각 스레드가 worker() 함수를 실행하게 한다.
 * 대기열로 사용할 원형 버퍼의 용량이 일꾼 스레드의 수보다 작으면 효율을 극대화할 수 없다.
 * 이런 경우 사용자가 요청한 queue_size를 bee_size로 상향 조정한다.
 * 성공하면 POOL_SUCCESS를, 실패하면 POOL_FAIL을 리턴한다.
 */
int pthread_pool_init(pthread_pool_t *pool, size_t bee_size, size_t queue_size)
{
    // 사이즈 체크 
    if(bee_size > POOL_MAXBSIZE || queue_size > POOL_MAXQSIZE)
    	return POOL_FAIL;
    if(queue_size < bee_size)
    	queue_size = bee_size;
    	
    // 일꾼 스레드와 대기열에 필요한 공간 할당
    pool->bee = (pthread_t *) malloc(sizeof(pthread_t) * bee_size);
    pool->q = (task_t *) malloc(sizeof(task_t) * queue_size);
    // 변수 초기화
    pool->running = 0;                 
    pool->q_size = queue_size;             
    pool->q_front = 0;            
    pool->q_len = 0;                  
    pool->bee_size = bee_size;    
    // 일꾼 스레드 동기화 용 lock, cond 초기화
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->full, NULL);
    pthread_cond_init(&pool->empty, NULL);
    
    // 일꾼 스레드 생성 후 각 스레드가 worker() 실행
    for(int i = 0; i < bee_size; i++) {
    	pthread_create(&(pool->bee[i]), NULL, worker, pool);
    }
	return POOL_SUCCESS;    	
}

/*
 * 스레드풀에서 실행시킬 함수와 인자의 주소를 넘겨주며 작업을 요청한다.
 * 스레드풀의 대기열이 꽉 찬 상황에서 flag이 POOL_NOWAIT이면 즉시 POOL_FULL을 리턴한다.
 * POOL_WAIT이면 대기열에 빈 자리가 나올 때까지 기다렸다가 넣고 나온다.
 * 작업 요청이 성공하면 POOL_SUCCESS를 리턴한다.
 */
int pthread_pool_submit(pthread_pool_t *pool, void (*f)(void *p), void *p, int flag)
{
    // 대기열 접근을 위한 lock
    pthread_mutex_lock(&pool->mutex);
    // 대기열 꽉 찬 상황에서 flag 값이
    if (pool->q_len == pool->q_size) {
    	// 1) POOL_NOWAIT(바로 빠져나옴)
		if(flag == POOL_NOWAIT) {
			pthread_mutex_unlock(&pool->mutex);
			return POOL_FULL;
		}
		// 2) POOL_WAIT(대기열 빌때까지 기다림)
		else if(flag == POOL_WAIT) {
			while((pool->q_len == pool->q_size))
				pthread_cond_wait(&pool->empty,&pool->mutex);
		}
	}
    // 작업 함수 설정
    task_t task;
    task.function = f;
    task.param = p;
    // 대기열에 작업 넣기
    int task_back = (pool->q_front + pool->q_len) % pool->q_size;
	pool->q[task_back] = task;
	pool->q_len = pool->q_len + 1;    
    // 락 해제 
    pthread_cond_signal(&pool->full);
    pthread_mutex_unlock(&pool->mutex);
    
    return POOL_SUCCESS;
}

/*
 * 스레드풀을 종료한다. 일꾼 스레드가 현재 작업 중이면 그 작업을 마치게 한다.
 * how의 값이 POOL_COMPLETE이면 대기열에 남아 있는 모든 작업을 마치고 종료한다.
 * POOL_DISCARD이면 대기열에 새 작업이 남아 있어도 더 이상 수행하지 않고 종료한다.
 * 부모 스레드는 종료된 일꾼 스레드와 조인한 후에 스레드풀에 할당된 자원을 반납한다.
 * 스레드를 종료시키기 위해 철회를 생각할 수 있으나 바람직하지 않다.
 * 락을 소유한 스레드를 중간에 철회하면 교착상태가 발생하기 쉽기 때문이다.
 * 종료가 완료되면 POOL_SUCCESS를 리턴한다.
 */
int pthread_pool_shutdown(pthread_pool_t *pool, int how)
{
    
    // 대기열 접근을 위한 lock
    pthread_mutex_lock(&pool->mutex);
    // 작업중인 스레드 작업 마치게 함. how 값이
    // 1) POOL_COMPLETE(대기열의 모든 작업 마침)
    if(how == POOL_COMPLETE) {
    	pool->running = 1;
		pthread_cond_broadcast(&pool->full);
		pthread_cond_broadcast(&pool->empty);
    }
    // 2) POOL_DISCARD(대기열 작업 수행 없이 종료)
    else if(how == POOL_DISCARD) {
    	pool->running = 2;
		pthread_cond_broadcast(&pool->full);
		pthread_cond_broadcast(&pool->empty);
    } 	
    // 락 해제 
    pthread_mutex_unlock(&pool->mutex);
    // 일꾼 스레드 join 
    for(int i = 0; i < pool->bee_size; i++) {
    	pthread_join((pool->bee)[i],NULL);
    }
    // 할당한 자원 반납
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->full);
    pthread_cond_destroy(&pool->empty);
    free(pool->bee);
    free(pool->q);
    
    return POOL_SUCCESS;   
}
