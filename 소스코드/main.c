#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ssu_score.h"

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t; //실행시작하기 전, 후 시간
	gettimeofday(&begin_t, NULL); //실행 전 시간

	ssu_score(argc, argv);

	gettimeofday(&end_t, NULL); //실행 후 시간
	ssu_runtime(&begin_t, &end_t); //실행시간 구하기

	exit(0);
}

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t) //실행시간 구하기
{
	end_t->tv_sec -= begin_t->tv_sec; //실행이 끝났을 때의 초에서 실행 시작 전 초를 빼서 실행시간을 구한다.

	if(end_t->tv_usec < begin_t->tv_usec){ //실행 전의 usec가 더 크다면
		end_t->tv_sec--; //end의 초를 하나 감소하여
		end_t->tv_usec += SECOND_TO_MICRO; //단위를 바꿔 usec에 더한다.
	}

	end_t->tv_usec -= begin_t->tv_usec; //실행 후에서 전을 뺀 시간을 usec 단위로 구한다.
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}
