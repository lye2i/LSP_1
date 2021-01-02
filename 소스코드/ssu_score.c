#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM]; //ssu_score.h에 있는 구조체
extern char id_table[SNUM][10]; 

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN]; //학생 디렉토리
char ansDir[BUFLEN]; //답 디렉토리
char errorDir[BUFLEN]; //에러 디렉토리
char threadFiles[ARGNUM][FILELEN];//thread 파일

int eOption = false;
int tOption = false;

void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){ //입력받은 인자 중 "-h"가 있다면
			print_usage(); //함수호출
			return;
		}
	}

	memset(saved_path, 0, BUFLEN); //saved_path 메모리를 0으로 BUFLEN만큼(1024) 초기화
	if(argc >= 3 && strcmp(argv[1], "-c") != 0){ //인자의 갯수가 3보다 크거나 같고 첫번째로 받은 인자가 "-c"와 같지 않다면
		strcpy(stuDir, argv[1]); //stuDir에 첫번째 인자를 복사하고
		strcpy(ansDir, argv[2]); //ansDir에 두번째 인자를 복사한다.
	}

	if(!check_option(argc, argv)) //만약 check_option(옵션확인하는함수)의 반환값이 false가 아니라면 프로그램을 종료한다.
		exit(1);

	getcwd(saved_path, BUFLEN); //saved_path에 현재 작업 디렉토리에 대한 전체 경로 이름을 얻는다.

	if(chdir(stuDir) < 0){ //stuDir로 현재 작업 디렉토리를 변경
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN); //stuDir에 현재 작업 디렉토리에 대한 전체 경로 이름을 얻는다.

	chdir(saved_path); //saved_path로 현재 작업 디렉토리를 변경한다.

	if(chdir(ansDir) < 0){ //ansDir로 현재 작업 디렉토리를 변경한다. 
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN); //ansDir에 현재 작업 디렉토리에 대한 전체 경로 이름을 얻는다.

	chdir(saved_path); //saved_path로 현재 작업 디렉토리를 변경한다.

	set_scoreTable(ansDir); // 점수테이블을 만든다.
	set_idTable(stuDir); //stuDir에 id테이블을 만든다.

	printf("grading student's test papers..\n");
	score_students(); //채점

	return;
}

int check_option(int argc, char *argv[]) //옵션을 확인하는 함수
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:th")) != -1) //세번째 인자에 포함된 문자를 받는다면 해당문자의 값을 c에 넣고 -1이 아닐때까지 반복
	{
		switch(c){ 
			case 'e': //e를 입력받았을 때는 eoption이 true가 되고 
				eOption = true;
				strcpy(errorDir, optarg); //errorDir에 optarg(인수를 필요로하는 옵션을 처리할때 인수를 가리킴)를 복사한다.

				if(access(errorDir, F_OK) < 0) //만약 errorDir가 존재하지 않는다면
					mkdir(errorDir, 0755); //errorDir 다음의 접근권한을 갖는 디렉토리를 생성한다.
				else{ //errorDir가 이미 존재한다면
					rmdirs(errorDir); //해당 디렉토리를 제거
					mkdir(errorDir, 0755); //다음의 접근권한을 갖는 디렉토리를 생성한다.
				}
				break;
			case 't': //t를 입력받았을 때는 tOption이 true가 되고
				tOption = true;
				i = optind; //인수로 받은 문자열 중에 처리하지 못한 인수의 인덱스 번호
				j = 0;

				while(i < argc && argv[i][0] != '-'){ //문자열이 '-'로 시작하지 않는 optind가 존재하는 동안

					if(j >= ARGNUM) //j가 가변인자 최대 갯수를 초과한 경우 출력
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(threadFiles[j], argv[i]); //그렇지 않다면 threadFiles에 인자로 받은 문자열을 복사한다.
					i++; 
					j++;
				}
				break;

			case '?': //인식되지 않는 옵션
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}


int is_exist(char (*src)[FILELEN], char *target) //파일이 존재하는지 확인
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM) // 5보다 i가 크면 
			return false; 
		else if(!strcmp(src[i], "")) //존재하지 않는다면
			return false;
		else if(!strcmp(src[i++], target)) //존재한다면
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir) //파일 이름을 정하여 scoreTable 만들기
{
	char filename[FILELEN];

	sprintf(filename, "%s" ,"score_table.csv"); //filename에 "score_table.csv"을 쓴다.

	if(access(filename, F_OK) == 0) //점수테이블파일이 존재한다면 
		read_scoreTable(filename); //파일의 내용을 읽는다.
	else{ //존재하지 않는다면

		make_scoreTable(ansDir); //점수 테이블을 만든다.
		write_scoreTable(filename); //점수 테이블에 데이터를 쓴다.
	}
}

void read_scoreTable(char *path) //점수 테이블 읽는 함수
{
	FILE *fp;
	char qname[FILELEN]; //문제번호
	char score[BUFLEN]; //점수
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){ //파일을 읽기 전용으로 오픈, 성공 시 FILE 구조체 변수 생성 그 구조체의 포인터를 리턴
		fprintf(stderr, "file open error for %s\n", path); //에러 시 오류메시지 출력
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){ //fp의 내용을 다음의 서식화된 형태로 qname, score에 쓰고 파일의 끝에 도달하지 않을 때까지 반복
		strcpy(score_table[idx].qname, qname); //score_table의 qname포인터에 문제번호를 쓴다.
		score_table[idx++].score = atof(score); //score_table의 score에 실수 score을 넣는다.
	}

	fclose(fp); //오픈된 파일 스트림을 닫는다
}

void make_scoreTable(char *ansDir) //점수 테이블 만드는 함수
{
	int type, num; 
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirp;
	DIR *dp, *c_dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type(); //점수 테이블의 type를 가져옴

	if(num == 1) //blank, program 각각의 점수 입력
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore); //blank question의 점수 입력
		printf("Input value of program question : ");
		scanf("%lf", &pscore); //program question의 점수 입력
	}

	if((dp = opendir(ansDir)) == NULL){ //ansDir 디렉토리의 구조체 포인터 리턴
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL) //open된 디렉토리에서 각각의 항목을 읽는다.
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //'.'과 '..'는 현재 폴더와 이전 폴더를 가리키므로 제외해서 검색한다.
			continue;

		sprintf(tmp, "%s/%s",ansDir,dirp->d_name); //tmp에 그 파일의 이름을 쓴다.


		if((type = get_file_type(dirp->d_name)) < 0) //dirp의 파일이름을 검사하여 파일의 종류를 알아냄, 에러 시 -1리턴
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name); //score_table의 qname에 dirp의 파일이름을 복사한다.
	}

	closedir(dp); //dp 디렉토리를 닫음
	sort_scoreTable(idx); //생성된 테이블 수만큼 비교하여 score_table 정렬

	for(i = 0; i < idx; i++) //생성된 테이블 수 만큼 반복
	{
		type = get_file_type(score_table[i].qname); //score_table의 qname에 있는 값을 검사하여 파일의 종류를 알아냄

		if(num == 1) //점수 테이블의 type이 1이라면
		{
			if(type == TEXTFILE) //qname에 있는 문자열이 TEXTFILE과 같다면 
				score = bscore; //score에는 blank의 점수
			else if(type == CFILE) //qname에 있는 문자열이 CFILE과 같다면
				score = pscore; //score에는 programm의 점수
		}
		else if(num == 2) //그렇지 않고 점수 테이블의 type이 2라면
		{
			printf("Input of %s: ", score_table[i].qname); //table의 qname을 출력하고 
			scanf("%lf", &score); //해당하는 점수를 입력한다.
		}

		score_table[i].score = score; //각 테이블의 score에 입력받은 score을 넣는다.
	}
}

void write_scoreTable(char *filename) //score_table 쓰기
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]); //score_table의 크기를 score_table[0]의 크기로 나눈 후 그 몫을 num에 넣는다. -> score_table 갯수 구하기

	if((fd = creat(filename, 0666)) < 0){ //filename에 주어진 인자의 접근권한을 갖는 파일을 생성한다.
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0) //score_table의 score이 0이라면 반복문을 나간다
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score); //tmp에 각각의  score_table의 qname과 score의 소숫점 자리 수 1까지 쓴다.
		write(fd, tmp, strlen(tmp)); //fd의 파일에 tmp의 데이터를 tmp에 문자열 길이만큼 쓴다
	}

	close(fd); //파일을 닫는다.
}


void set_idTable(char *stuDir) //id_table만들기
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){ //stuDir 디렉토리의 구조체 포인터 리턴
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){ //오픈된 디렉토리의 각각의 항목을 읽는다.
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //'.'과 '..'는 현재 폴더와 이전 폴더를 가리키므로 제외해서 검색한다
			continue; 

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); //tmp에 stuDir과 파일의 이름을 쓴다.
		stat(tmp, &statbuf); //tmp의 파일의 정보를 담은 stat 구조체를 리턴한다

		if(S_ISDIR(statbuf.st_mode)) //디렉토리 파일이라면
			strcpy(id_table[num++], dirp->d_name); //id_table을 하나 추가하고 그 파일의 이름을 복사한다.
		else
			continue;
	}

	sort_idTable(num); //id_table의 정렬
}

void sort_idTable(int size) //id_table 정렬
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){ 
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){ //id_table의 문자열을 비교하여 [j]가 [j+1]보다 큰 경우에는 그 문자열을 서로 바꾼다.
				strcpy(tmp, id_table[j]); //tmp에 id_table[j]의 내용을 복사
				strcpy(id_table[j], id_table[j+1]); //id_table[j]에 id_table[j+1]을 복사
				strcpy(id_table[j+1], tmp); //id_table[j+1]에 tmp 내용을 복사
			}
		}
	}
}

void sort_scoreTable(int size) //점수 테이블 정렬
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2; 
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){ 
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2); //score_table의 qname이 가리키는 값과
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2); //반복적으로 qname이 가리키는 값들을


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){ //두 점수 값을 비교

				memcpy(&tmp, &score_table[j], sizeof(score_table[0])); //정렬을 위해 tmp에 score_table[j]의 데이터를 score_table[0]의 크기만큼 초기화
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0])); // score_table[j]에는 그 다음인 score_table[j]의 데이터를 score_table[0]의 크기만큼 최소화
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0])); //score_table[j+1]에는 tmp에 써놨던 score_table[j]를 score_table의 크기만큼 초기화
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2) //문제 번호 구하기
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); //dup에 qname이 가리키는 문자열을 qname 문자열의 길이만큼 복사한다.
	*num1 = atoi(strtok(dup, "-.")); //dup에서 "-."을 기준으로 자른 문자열을 반환하여 그 문자열을 정수로 변환하여 num1이 가리키게 한다.
	
	p = strtok(NULL, "-."); //남은 문자열을 "-."을 기준으로 자르고 그 문자열을 포인터 반환한다.
	if(p == NULL) //가리키는 문자열이 없다면
		*num2 = 0; //num2가 가리키는 값은 0이다.
	else //가리키는 문자열이 있다면 
		*num2 = atoi(p); //num2가 가리키는 값은 p가 가리키는 문자열을 정수로 형변환 한 것이다.
}

int get_create_type() //table을 만들기 전 점수를 받을 type를 생성
{
	int num;

	while(1) //선택할 때까지 무한반복
	{
		printf("score_table.csv file doesn't exist in TREUDIR!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n"); 
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num); 

		if(num != 1 && num != 2) //num이 1과 2가 모두 아닌 경우
			printf("not correct number!\n");
		else
			break;
	}

	return num; //1,2 중 선택한 값 리턴
}

void score_students() //학생들 채점결과
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]); //학생의 수 구하기

	if((fd = creat("score.csv", 0666)) < 0){ //다음의 접근권한을 갖는 "score.csv" 파일을 생성한다.
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); //fd의 첫번째 행을 쓴다.

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], "")) //각 id_table이 비어있다면
			break; //반복문 나가기

		sprintf(tmp, "%s,", id_table[num]); //tmp에 id_table의 문자열을 쓴다.
		write(fd, tmp, strlen(tmp)); //fd에 tmp에 내용을 쓴다.

		score += score_student(fd, id_table[num]); //총 학생의 점수를 모두 더한다.
	}

	printf("Total average : %.2f\n", score / num); //전체 평균을 출력

	close(fd); //파일을 닫는다.
}

double score_student(int fd, char *id) //학생들 채점 결과 계산
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //score_table의 갯수 구하기

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0) //각 score_table의 score이 0인 경우에는 아래의 과정을 하지 않는다.
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); //tmp에 stuDir/학번/문제번호를 쓴다.

		if(access(tmp, F_OK) < 0) //파일이 존재하지 않는다면
			result = false; //result는 0이다.
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0) //score_table의 문제번호 타입을 구하고 오류시 아래의 과정을 하지 않는다.
				continue;
			
			if(type == TEXTFILE) //TEXTFILE인 경우 result에 blank 점수를 넣고
				result = score_blank(id, score_table[i].qname);
			else if(type == CFILE) //CFILE인 경우 result에 program 점수를 넣는다.
				result = score_program(id, score_table[i].qname);
		}

		if(result == false) //result가 false라면
			write(fd, "0,", 2); //점수는 0이다.
		else{
			if(result == true){ //result가 true라면
				score += score_table[i].score; //double형 score에 각 score_table의 score과 더한 값으로 할당한다.
				sprintf(tmp, "%.2f,", score_table[i].score); //tmp에 score_table의 score를 소숫점 1자리까지의 값을 쓴다.
			}
			else if(result < 0){ //result가 음수라면
				score = score + score_table[i].score + result; //double형 score에 각 score_table의 score과 result, double형 score를 더한 값을 할당한다.
				sprintf(tmp, "%.2f,", score_table[i].score + result); //tmp에 score_table의 score와 result를 더한 값을 소숫점 2자리까지의 값으로 쓴다.
			}
			write(fd, tmp, strlen(tmp)); //fd에 tmp의 데이터를 문자열길이만큼 쓴다.
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); //채점을 진행하면서 각 학생의 점수를 출력

	sprintf(tmp, "%.2f\n", score); //tmp에 score을 쓴다.
	write(fd, tmp, strlen(tmp)); //fd에 tmp의 내용을 문자열 길이만큼 쓴다.

	return score; //계산한 점수 반환
}

void write_first_row(int fd) //채점결과파일의 첫번째 열은 문제번호들과 마지막에 sum이어야함 
{
	int i; 
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //점수 테이블의 갯수 구하기

	write(fd, ",", 1); //fd에 ","을 쓴다.

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0) //각 score_table의 score이 0이라면 아래의 과정을 실행하지 않는다.
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname); //tmp에 score_table의 문제번호를 쓴다. 
		write(fd, tmp, strlen(tmp)); //fd에 tmp의 문자열을 쓴다.
	}
	write(fd, "sum\n", 4); //fd에 문자열 "sum\n"을 쓴다.
}

char *get_answer(int fd, char *result) //답 문자열 복사
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN); //result를 BUFLEN만큼 초기화
	while(read(fd, &c, 1) > 0) //fd의 데이터를 반복해서 한글자씩 읽는다.
	{
		if(c == ':') //만약 읽은 글자가 콜론(:)이라면
			break; //반복문을 나가고
		
		result[idx++] = c; //아니라면 해당 인덱스의 result 배열에 글자를 쓴다.
	}
	if(result[strlen(result) - 1] == '\n') //result의 마지막이 개행문자라면
		result[strlen(result) - 1] = '\0'; //문자열 종료

	return result; //result 배열 반환
}

int score_blank(char *id, char *filename) //blank점수 매기기(학번, 문제번호)
{
	char tokens[TOKEN_CNT][MINLEN]; 
	node *std_root = NULL, *ans_root = NULL; //학생의 root노드와 답의 root노드는 비어있다.
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN]; //학생의 답과 정답의 답의 내용을 담는 배열
	char qname[FILELEN]; //문제번호
	int fd_std, fd_ans; //학생 파일디스크립터, 정답 파일디스크립터
	int result = true; 
	int has_semicolon = false; //문자열의 마지막이 ;인지 확인

	memset(qname, 0, sizeof(qname)); //qname 배열 초기화
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //qname 배열에 filename에 있는 데이터를 filename의 문자열 길이에서 '.'으로 시작하는 문자열의 길이를 뺀 수 만큼 쓴다.

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename); //tmp 배열에 stuDir/학번/파일이름을 쓴다.
	fd_std = open(tmp, O_RDONLY); //읽기 전용을 학생답파일을 오픈한다.
	strcpy(s_answer, get_answer(fd_std, s_answer)); //s_answer배열에 학생의 답 문자열을 복사한다.  

	if(!strcmp(s_answer, "")){ //s_answer배열이 비어있다면
		close(fd_std); //파일을 닫고 false반환
		return false;
	}

	if(!check_brackets(s_answer)){ //괄호가 제대로 있지 않다면
		close(fd_std); //파일을 닫고 false 반환
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer))); //s_answer 문자열의 오른쪽 공백을 제거한 후 왼쪽 공백을 제거한 문자열을 s_answer에 복사한다.

	if(s_answer[strlen(s_answer) - 1] == ';'){ //s_answer의 마지막이 ';'라면
		has_semicolon = true; //true로 바꾸고
		s_answer[strlen(s_answer) - 1] = '\0'; //s_answer 문자열 종료
	}

	if(!make_tokens(s_answer, tokens)){ //학생의 답을 토큰으로 만들기
		close(fd_std); //파일을 닫고 false 반환
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0); // 토큰을 통해 트리를 만든다.

	sprintf(tmp, "%s/%s", ansDir, filename); //tmp배열에 ansDir/파일이름의 형식으로 쓴다.
	fd_ans = open(tmp, O_RDONLY); //읽기 전용으로 정답파일을 오픈한다.

	while(1)
	{
		ans_root = NULL; //초기화
		result = true; 

		for(idx = 0; idx < TOKEN_CNT; idx++) 
			memset(tokens[idx], 0, sizeof(tokens[idx])); //tokens 배열 초기화

		strcpy(a_answer, get_answer(fd_ans, a_answer)); //a_answer 배열에 정답의 답 문자열을 복사한다.

		if(!strcmp(a_answer, "")) //a_answer이 비어있다면
			break; //나가고

		strcpy(a_answer, ltrim(rtrim(a_answer))); //a_answer 문자열의 오른쪽 공백을 제거한 후 왼쪽 공백을 제거한 문자열을 a_answer에 복사한다.

		if(has_semicolon == false){ //;을 갖지 않았다면
			if(a_answer[strlen(a_answer) -1] == ';') //마지막에 ;을 넣어준다.
				continue;
		}

		else if(has_semicolon == true) //;을 가졌다면
		{
			if(a_answer[strlen(a_answer) - 1] != ';') //a_answer문자열의 마지막이 ;과 같지 않다면 다시 돌아간다
				continue; 
			else
				a_answer[strlen(a_answer) - 1] = '\0'; //a_answer문자열의 마지막이 ;과 같다면 문자열 종료
		}

		if(!make_tokens(a_answer, tokens)) //정답의 문자열을 토큰으로 만든다.
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0); //정답 문자열로 만든 토큰으로 tree를 만든다.

		compare_tree(std_root, ans_root, &result); //정답 트리와 학생 답 트리를 비교한다.

		if(result == true){ //result가 true라면
			close(fd_std); //fd_std의 파일을 닫고
			close(fd_ans); //fd_ans의 파일을 닫는다.

			if(std_root != NULL) //std_root가 가리키는 노드가 비어있지 않다면
				free_node(std_root); //std_rott노드를 초기화시켜 메모리 해제
			if(ans_root != NULL) //ans_root가 가리키는 노드가 비어있지 않다면
				free_node(ans_root); //ans_root노드를 초기화시켜 메모리 해제
			return true;

		}
	}
	
	close(fd_std); //fd_std의 파일을 닫고
	close(fd_ans); //fd_ans의 파일을 닫는다.

	if(std_root != NULL) //std_root가 가리키는 노드가 비어있지 않다면
		free_node(std_root); //blank.c에 있는 함수 free_node를 실행한다.
	if(ans_root != NULL) //ans_root를 가리키는 노드가 비어있지 않다면
		free_node(ans_root); //blank.c에 있는 함수 free_node를 실행한다.

	return false; //false 반환
}

double score_program(char *id, char *filename) //program 점수 매기기
{
	double compile;
	int result;

	compile = compile_program(id, filename); //compile에 compile_program값 리턴 

	if(compile == ERROR || compile == false) //리턴값이 ERROR나 fasle라면
		return false; //그 프로그램은 complile이 안되므로 0
	
	result = execute_program(id, filename); //실행프로그램 확인

	if(!result) //result가 0이라면
		return false;

	if(compile < 0) //warning으로 점수가 감점되었다면
		return compile; //그 점수를 리턴

	return true; //컴파일 문제가 없고 실행에도 문제가 없다면 return 1
}

int is_thread(char *qname) //thread 확인
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]); //threadfile의 갯수구하기

	for(i = 0; i < size; i++){ 
		if(!strcmp(threadFiles[i], qname)) //threadfile의 문자열과 문제이름과 같은 경우가 있다면
			return true; //true 반환
	}
	return false; //아니라면 false
}

double compile_program(char *id, char *filename) //컴파일 확인
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN]; //파일, 실행파일
	char command[BUFLEN];
	char qname[FILELEN]; //문제 번호
	int isthread; 
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname)); //qname 초기화
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //qname 배열에 filename에 있는 데이터를 filename의 문자열 길이에서 '.'으로 시작하는 문자열의 길이를 뺀 수 만큼 쓴다.
	
	isthread = is_thread(qname); //threadfile과 문제번호 문자열이 같은지 판단

	sprintf(tmp_f, "%s/%s", ansDir, filename); //tmp_f에 다음의 형식의 문자열을 ANS,파일 이름을 쓴다. ->각 문제에 대한 정답프로그램
	sprintf(tmp_e, "%s/%s.exe", ansDir,qname); //tmp_e에 다음의 형식에 문자열을 ANS,,문제번호을 쓴다. ->각 문제에 대한 정답프로그램의 실행파일

	if(tOption && isthread) //tOption과 isthread 모두 true라면
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //command에 다음 형식의 문자열을 쓴다. ->lpthread옵션
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //command에 다음 형식의 문자열을 쓴다. ->아니라면 컴파일만

	sprintf(tmp_e, "%s/%s_error.txt", ansDir,qname); //tmp_e에 다음 형식의 에러메시지를 쓴다.
	fd = creat(tmp_e, 0666); //인자로 받은 접근권한을 갖는 tmp_e파일을 생성한다.

	redirection(command, fd, STDERR); //command 명령어를 실행하여 STDERR의 파일 디스크립터를 받는다.
	size = lseek(fd, 0, SEEK_END); //fd의 파일크기 
	close(fd); //fd의 파일을 닫는다.
	unlink(tmp_e); //tmp_e로는 파일에 접근할 수 없게 한다.

	if(size > 0) //파일의 크기가 0보다 크다면
		return false; //false 리턴

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); //tmp_f에 다음 형식의 문자열을 쓴다. -> 학생이 작성해서 답안으로 제출한 프로그램
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); //tmp_e에 다음 형식의 문자열을 쓴다. -> 학생이 작성해서 답으로 제출한 프로그램의 실행파일

	if(tOption && isthread) //tOption과 isthread 모두 true라면
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f); //command에 다음 형식의 문자열을 쓴다. -> lpthread 옵션
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //command에 다음 형식의 문자열을 쓴다. -> 아니라면 컴파일만

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir,id, qname); //tmp_f에 다음 형식의 에러메시지를 쓴다.
	fd = creat(tmp_f, 0666); //인자로 받은 접근권한을 갖는 tmp_f파일을 생성한다.

	redirection(command, fd, STDERR); //command명령어를 실행하여 fd에 STDERR를 쓴다.
	size = lseek(fd, 0, SEEK_END); //fd의 파일 크기
	close(fd); //파일을 닫는다.

	if(size > 0){ //파일의 크기가 0보다 크다면
		if(eOption) //eOption을 실행하면
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); //tmp_e에 다음 형식의 문자열을 쓴다. 
			if(access(tmp_e, F_OK) < 0) //tmp_e의 파일이 존재하지 않는다면
				mkdir(tmp_e, 0755); //다음의 접근권한을 가진 tmp_e 파일 디렉토리를 만든다.

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname); //tmp_e에 errorDir/학번/문제번호를 쓴다.
			rename(tmp_f, tmp_e); //tmp_f 파일의 이름을 tmp_e로 변경한다.

			result = check_error_warning(tmp_e); //result에 tmp_e에 대한 error나 warning 리턴
		}
		else{ //그렇지 않다면
			result = check_error_warning(tmp_f); //result에 tmp_f에 대한 error나 warning 리턴
			unlink(tmp_f); //tmp_f로 파일을 접근 할 수 없도록 함
		}

		return result; //에러나 경고 리턴
	}

	unlink(tmp_f); //tmp_f로 파일을 접근할 수 없도록 함
	return true; //컴파일의 문제가 없으므로 1 리턴
}

double check_error_warning(char *filename)
{
	FILE *fp; 
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){ //파일을 읽기 전용으로 오픈하여 FILE 구조체의 포인터를 리턴
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){ //fp가 가리키는 파일로부터 문자열로 변환하여 tmp를 읽는다.
		if(!strcmp(tmp, "error:")) //tmp와 문자열 "error:"가 같다면
			return ERROR; //에러 리턴
		else if(!strcmp(tmp, "warning:")) //tmp와 문자열 "warning:"가 같다면
			warning += WARNING; //double형 warning에 계속 추가한다.
	}

	return warning; //총 warning의 값을 리턴
}

int execute_program(char *id, char *filename) //실행 확인
{
	char std_fname[BUFLEN], ans_fname[BUFLEN]; //학생답, 정답
	char tmp[BUFLEN];
	char qname[FILELEN]; //문제번호
	time_t start, end; //실행 시간 측정
	pid_t pid; 
	int fd;

	memset(qname, 0, sizeof(qname)); //qname 초기화
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //qname에 filename에 있는 데이터를 filename의 문자열 길이에서 '.'으로 시작하는 문자열의 길이를 뺀 수 만큼 쓴다.

	sprintf(ans_fname, "%s/%s.stdout", ansDir,qname); //ans_fname에 다음 형식의 문자열을 쓴다. -> 정답 결과
	fd = creat(ans_fname, 0666); // 주어진 접근권한을 갖는 파일을 생성한다.

	sprintf(tmp, "%s/%s.exe", ansDir, qname); //tmp에 다음 형식의 문자열을 쓴다. -> 정답프로그램의 실행파일
	redirection(tmp, fd, STDOUT); //tmp를 fd에 쓴다.
	close(fd); //fd의 파일 닫는다.

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); //std_fname에 다음 형식의 문자열을 쓴다. -> 학생 결과
	fd = creat(std_fname, 0666); //주어진 접근권한을 갖는 파일을 생성한다.

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); //tmp에 다음 형식의 문자열을 쓴다, -> 학생 실행파일

	start = time(NULL); //실행 전 시간
	redirection(tmp, fd, STDOUT); //tmp를 fd에 쓴다.
	
	sprintf(tmp, "%s.stdexe", qname); //문제번호의 실행파일
	while((pid = inBackground(tmp)) > 0){ //pid가 0보다 큰 동안
		end = time(NULL); //실행 후 시간

		if(difftime(end, start) > OVER){ //실행시간이 5초 이상 걸린다면 
			kill(pid, SIGKILL); //프로그램 종료
			close(fd); //fd의 파일을 닫는다. 
			return false; //실행이 안되므로 0 리턴
		}
	}

	close(fd); //fd의 파일을 닫는다.

	return compare_resultfile(std_fname, ans_fname); //학생결과와 정답결과가 같다면 true
}

pid_t inBackground(char *name) //파일을 실행 백그라운드로 진행
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp)); //tmp 초기화
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); //인자의 접근권한을 가진 읽기쓰기전용의 파일을 오픈하고 이때 이미 파일이 존재한다면 그 전의 데이터를 없애고 새로 만든다.

	sprintf(command, "ps | grep %s", name); //command에 다음 형식의 문자열을 쓴다 -> 현재 실행중인 프로세스에 name이 있는지 확인
	redirection(command, fd, STDOUT); //command를 fd에 쓴다.

	lseek(fd, 0, SEEK_SET); //fd의 오프셋 처음으로 이동
	read(fd, tmp, sizeof(tmp)); //fd의 데이터를 tmp에 tmp의 크기만큼 읽는다.

	if(!strcmp(tmp, "")){ //tmp가 비어있다면
		unlink("background.txt"); //background.txt로 파일에 접근하지 못하게 한다.
		close(fd); //fd의 파일을 닫는다.
		return 0;
	}

	pid = atoi(strtok(tmp, " ")); //tmp를 공백문자 기준으로 잘라 그 문자열을 정수로 형변환 한 것을 pid에 넣는다.
	close(fd); //fd의 파일을 닫는다.

	unlink("background.txt"); //background.txt로 파일이 접근 할 수 없도록 한다.
	return pid; //pid 리턴 
}

int compare_resultfile(char *file1, char *file2) //파일 결과 비교
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); //읽기 전용으로 file1의 파일 오픈
	fd2 = open(file2, O_RDONLY); //읽기 전용으로 file2의 파일 오픈

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){ //fd1의 파일을 한글자씩 읽어서 
			if(c1 == ' ') //공백문자가 나온다면
				continue; 
			else 
				break; 
		}
		while((len2 = read(fd2, &c2, 1)) > 0){ //fd2의 파일을 한글자씩 읽어서
			if(c2 == ' ') //공백문자가 나온다면
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0) //fd1과 fd2 모두 파일의 끝에 도달하면
			break;

		to_lower_case(&c1); //c1을 소문자로 바꾼다.
		to_lower_case(&c2); //c2를 소문자로 바꾼다.

		if(c1 != c2){ //c1과 c2가 같지 않다면
			close(fd1); //fd1의 파일을 닫고
			close(fd2); //fd2의 파일을 닫는다.
			return false;
		}
	} //c1과 c2가 같다
	close(fd1); //fd1의 파일을 닫고
	close(fd2); //fd2의 파일을 닫는다
	return true;
}

void redirection(char *command, int new, int old) //재지향
{
	int saved;

	saved = dup(old); //STDERR를 복사하여 새 파일 디스크립터 리턴
	dup2(new, old); //리턴값을 old로 지정하고 new의 파일디스크립터 복사

	system(command); //command의 명령어 실행

	dup2(saved, old); //리턴값을 old로 지정하여 saved 파일디스크립터 복사
	close(saved); //saved의 파일을 닫는다.
}

int get_file_type(char *filename)
{
	char *extension = strrchr(filename, '.'); //filename에서 '.'로 시작하는 문자열 검색, 포인터 반환

	if(!strcmp(extension, ".txt")) //포인터가 가르키는 문자열에서 ".txt"와 일치한다면 TEXTFILE
		return TEXTFILE;
	else if (!strcmp(extension, ".c")) //그렇지 않고 ".c"와 일치한다면 CFILE
		return CFILE;
	else
		return -1; //둘다 아니면 에
}

void rmdirs(const char *path) //디렉토리 삭제
{
	struct dirent *dirp; 
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN];
	
	if((dp = opendir(path)) == NULL) //path DIR의 구조체를 리턴
		return;

	while((dirp = readdir(dp)) != NULL) //오픈된 디렉토리의 파일을 읽는다.
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) //'.'과 '..'는 현재 폴더와 이전 폴더를 가리키므로 제외해서 검색한다
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name); //tmp에 다음 형식의 문자열을 쓴다. -> 디렉토리의 경로

		if(lstat(tmp, &statbuf) == -1) //tmp의 파일 자체에 대한 정보를 리턴
			continue;

		if(S_ISDIR(statbuf.st_mode)) //디렉토리인 경우
			rmdirs(tmp); //tmp를 삭제
		else
			unlink(tmp); //정규파일이면 tmp로 해당 파일의 접근하지 못하도록 한다.
	}

	closedir(dp); //dp의 디렉토리를 닫는다.
	rmdir(path); //빈 디렉토리 삭제
}

void to_lower_case(char *c)
{
	if(*c >= 'A' && *c <= 'Z') //문자 c가 대문자라면
		*c = *c + 32; //소문자로 바꿔준다
}

void print_usage() //h 옵션->사용법 출력
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong questions\n");
	printf(" -h                print usage\n");
}
