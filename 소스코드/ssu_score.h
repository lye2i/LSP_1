#ifndef MAIN_H_
#define MAIN_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef STDOUT
	#define STDOUT 1
#endif
#ifndef STDERR
	#define STDERR 2
#endif
#ifndef TEXTFILE
	#define TEXTFILE 3
#endif
#ifndef CFILE
	#define CFILE 4
#endif
#ifndef OVER
	#define OVER 5 //실행시간이 5를 넘으면 안됨
#endif
#ifndef WARNING
	#define WARNING -0.1 //컴파일에서 감점
#endif
#ifndef ERROR
	#define ERROR 0 //컴파일에서 채점
#endif

#define FILELEN 64
#define BUFLEN 1024
#define SNUM 100
#define QNUM 100
#define ARGNUM 5

struct ssu_scoreTable{
	char qname[FILELEN];
	double score;
};

void ssu_score(int argc, char *argv[]); //전체적인 ssu_score 실행함수
int check_option(int argc, char *argv[]); //옵션 확인
void print_usage(); //h옵션 입력 시 사용법 출력

void score_students(); //학생들 문제 채점파일
double score_student(int fd, char *id); //학생들 문제 채점
void write_first_row(int fd); //첫번째 열은 문제번호들과 sum

char *get_answer(int fd, char *result); //문자열 구하기
int score_blank(char *id, char *filename); //blank문제 점수 구하기
double score_program(char *id, char *filename); //program문제 점수 구하기
double compile_program(char *id, char *filename); //compile확인
int execute_program(char *id, char *filname); //실행결과 확인
pid_t inBackground(char *name); //채점 실행시에 정답파일 실행
double check_error_warning(char *filename); //컴파일 시 error와 warning 확인
int compare_resultfile(char *file1, char *file2); //실행결과 비교

void do_cOption(char (*ids)[FILELEN]); //cOption
int is_exist(char (*src)[FILELEN], char *target); //채점파일 존재 확인

int is_thread(char *qname); //t옵션에서 lpthread 사용
void redirection(char *command, int newfd, int oldfd); //파일에 재지정
int get_file_type(char *filename); //file의 type구하기
void rmdirs(const char *path); //디렉토리 삭제
void to_lower_case(char *c); //소문자로 변환

void set_scoreTable(char *ansDir); //scoreTable 파일 만들기
void read_scoreTable(char *path); //scoreTable파일 읽기
void make_scoreTable(char *ansDir);//scoreTable 만들기
void write_scoreTable(char *filename); //파일에 scoreTable 쓰기
void set_idTable(char *stuDir); //idTable파일 만들기
int get_create_type(); //table타입 확인

void sort_idTable(int size); //idTable 정렬
void sort_scoreTable(int size); //scoreTable 정렬
void get_qname_number(char *qname, int *num1, int *num2); //문제번호 구하기

#endif
