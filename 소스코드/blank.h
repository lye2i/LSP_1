#ifndef BLANK_H_ //BLANK_H_를 define하지 않았다면
#define BLANK_H_

#ifndef true //true를 define하지 않았다면 true는 1로 define
	#define true 1
#endif
#ifndef false //false를 define하지 않았다면 false는 0로 define
	#define false 0
#endif
#ifndef BUFLEN //BUFLEN을 define하지 않았다면 1024로 define
	#define BUFLEN 1024
#endif

#define OPERATOR_CNT 24
#define DATATYPE_SIZE 35
#define MINLEN 64
#define TOKEN_CNT 50

typedef struct node{
	int parentheses; //괄호
	char *name; 
	struct node *parent;
	struct node *child_head;
	struct node *prev;
	struct node *next;
}node;

typedef struct operator_precedence{//연산자 우선순위
	char *operator;
	int precedence;
}operator_precedence;

void compare_tree(node *root1,  node *root2, int *result); //트리 비교
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses); //토큰으로 트리 만들기
node *change_sibling(node *parent); //인자로 받은 노드를 기준으로 같은 부모를 가지고 있는 노드로 바꾼다.
node *create_node(char *name, int parentheses); //새로운 노드 생성
int get_precedence(char *op); //연산자 우선순 구하기
int is_operator(char *op); //연산자 확인
void print(node *cur); //출력
node *get_operator(node *cur); //연산자 구하기
node *get_root(node *cur); //root노드 구하기
node *get_high_precedence_node(node *cur, node *new); //상위 노드 구하기
node *get_most_high_precedence_node(node *cur, node *new); //최상위 노드 구하기
node *insert_node(node *old, node *new); //노드 삽입
node *get_last_child(node *cur); //가장 오른쪽에 위치한 child 노드 구하기위
void free_node(node *cur); //노드, 메모리 초기화
int get_sibling_cnt(node *cur); //같은 부모를 가진 노드 갯수 구하기 

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]); //토큰 만들기
int is_typeStatement(char *str); //type확인
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]); //형 지정자 구하기
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]); //struct 형 확인
int is_character(char c); //문자가 숫자나 알파벳으로 이루어져있는지
int all_star(char *str); //문자열이 *로만 이루어져 있는지 확인
int all_character(char *str); //문자열 확인
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]); //인자로 받은 위치를 기
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]); //토큰 초기화
int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]); //토큰의 갯수 구하기
char *rtrim(char *_str); //오른쪽 공백 제거
char *ltrim(char *_str); //왼쪽 공백 제거
void remove_space(char *str); //공백문자 제거
int check_brackets(char *str); //괄호 확인
char* remove_extraspace(char *str); //문자열 내 공백문자 제거

#endif
