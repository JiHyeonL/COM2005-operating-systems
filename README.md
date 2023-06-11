# COM2005_operating-systems
COM2005 운영체제 과제입니다.

## proj1
기능이 제한적인 작은 유닉스 셸을 구현하는 것이다. 
과제의 목적은 프로세스 생성과 프로세스간 통신을 익히는 데 있다. <br/>
셸 구현을 통해 프로세스 생성, 명령어 실행, 표준 입출력 변경, 파이프를 통한 프로세스간 통신을 배운다.


## proj2 
다중 스레드를 생성하고 운영하는 법을 학습한다. <br/>
이를 위해 여러 개의 스레드를 생성하여 스도쿠 퍼즐의 해가 올바른지 검증한다. <br/>
검증에 사용하는 스레드는 다음과 같이 세가지 종류가 있다.
- 모든 가로 줄이 올바른지 검증하는 1개의 스레드
- 모든 세로 줄이 올바른지 검증하는 1개의 스레드
- 각각의 부분 격자가 올바른지 검증하는 9개의 스레드

## proj3
스핀락은 락의 경쟁이 낮거나 락을 소유하는 시간이 짧은 환경에서 유용한 방식이다. <br/>
이번 과제는 아래에 서술한 두 가지 문제를 스핀락을 사용하여 해결한다. <br/>
- 공유버퍼 (bounded-buffer) 문제: 여러 개의 생산자와 소비자가 원형버퍼를 공유한 상태
에서 생산과 소비를 동시에 진행할 때 발생하는 문제이다.
- 유한대기 (bounded-waiting) 문제: 여러 개의 스레드가 임계구역에 동시에 접근할 경우
락을 사용하여 상호배타를 보장할 수 있다. 아무리 길어도 어떤 정해진 차례 안에서는 스레드가 락을
얻을 수 있도록 보장하는 것이 유한대기 문제이다.

## proj4 
CS 접근을 통제할 수 있는 스레드 동기화 코드를 완성하고,
reader-writer 문제를 다음 네 가지 버전으로 설계하고 구현한다. <br/>
- Reader 선호 (조건변수 버전): POSIX 조건변수를 사용하여 reader 선호 방식을 구현한다.
단, 조건변수를 조회하기 위한 뮤텍스락 (mutex lock) 한 개 이외에는 어떠한 뮤텍스락이나
세마포를 사용할 수 없다.
- Writer 선호 (뮤텍스락 버전): POSIX 뮤텍스락을 사용하여 writer 선호 방식을 구현한다.
단, POSIX 조건변수나 세마포를 사용할 수 없다.
- Writer 선호 (조건변수 버전): POSIX 조건변수를 사용하여 writer 선호 방식을 구현한다.
단, 조건변수를 조회하기 위한 뮤텍스락 한 개 이외에는 어떠한 뮤텍스락이나 세마포를
사용할 수 없다.
- 공정한 reader-writer (뮤텍스락 버전): POSIX 뮤텍스락을 사용하여 공정한 reader-writer
방식을 구현한다. 단, POSIX 조건변수나 세마포를 사용할 수 없다

## proj5 
스레드풀은 스레드의 생성과 관리에 필요한 사용자의 부담과 비용을 줄이고, 시스템의 스레
드 수가 제한된 범위를 넘지 않도록 조절하는 유용한 방식이다. 사용자가 작업을 스레드풀에
제출하면 풀 안에 있는 스레드가 그 작업을 실행한다. 제출된 작업은 대기열에서 기다리다 한가
한 스레드에 의해 선택되어 실행된다. 대기열에 더 수행할 작업이 없으면 스레드는 새 작업이
들어와 깨워줄 때까지 대기상태로 있는다. Pthread와 POSIX 조건변수를 사용하여
스레드풀을 구현한다.
