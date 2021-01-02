#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"}; //DATATYPE_SIZE 35, MINLE=64


operator_precedence operators[OPERATOR_CNT] = { //operator_cnt=24 연산자 우선순위
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

void compare_tree(node *root1,  node *root2, int *result) //tree 비교
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){ //root1과 root2 중 하나라도 비어있다면 
		*result = false; //result는 0을 가리킨다.
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){ //root1과 root2 중 하나라도 관계 연산자라면
		if(strcmp(root1->name, root2->name) != 0){ //root1과 root2가 같은 문자열이 아니라면

			if(!strncmp(root2->name, "<", 1)) //root2가 "<"와 같다면
				strncpy(root2->name, ">", 1); //root2에 ">"를 복사한다.

			else if(!strncmp(root2->name, ">", 1)) //그렇지 않고 root2가 ">"와 같다면
				strncpy(root2->name, "<", 1); //root2에 "<"를 복사한다.

			else if(!strncmp(root2->name, "<=", 2)) //그렇지 않고 root2가 "<="와 같다면
				strncpy(root2->name, ">=", 2); //root2에 ">="를 복사한다.

			else if(!strncmp(root2->name, ">=", 2)) //그렇지 않고 root2가 ">="와 같다면
				strncpy(root2->name, "<=", 2); //root2에 "<="를 복사한다.

			root2 = change_sibling(root2); //같은 부모를 가진 노드를 root2가 가리키게 한다.
		}
	}

	if(strcmp(root1->name, root2->name) != 0){ //root1과 root2의 문자열이 다르다면->학생답과 정답이 다르다면
		*result = false; //result는 0을 가리킨다. ->틀린거
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL)
		|| (root1->child_head == NULL && root2->child_head != NULL)){ //root1과 root2의 child_head 중 한쪽만 비어있다면 -> 학생답과 정답이 다르다면

		*result = false; //result는 0을 가리킨다. -> 틀린거
		return;
	}

	else if(root1->child_head != NULL){ //그렇지 않고 root1의 child_head가 비어있지 않다면
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ //root1의 child_head와 같은 부모를 가진 노드 갯수와 root2의 child_head와 같은 부모를 가진 노드 갯수가 다르다면
			*result = false; //result는 0을 가리킨다. ->틀린거
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) //root1의 name이 ==이거나 !=라면
		{
			compare_tree(root1->child_head, root2->child_head, result); //재귀적으로 root1과 root2의 child_head를 기준으로 비교

			if(*result == false) //result가 0이라면
			{
				*result = true; //result에 true를 넣고
				root2 = change_sibling(root2); //root2에는 같은 부모를 가진 노드를 가리킨다.
				compare_tree(root1->child_head, root2->child_head, result); //root1의 child_head와 새로운 root2의 child_head를 기준으로 비교
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{ //root1의 name이 다음의 연산자들 중 하나라도 일치한다면
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){
				*result = false; //root1에서 child_head와 같은 부모를 가진 노드의 갯수와 root2에서 child_head와 같은 부모를 가진 노드의 갯수가 같지 않다면 result는 0을 가리킨다.
				return;
			}

			tmp = root2->child_head; //tmp노드는 root2의 child_head를 가리키고

			while(tmp->prev != NULL) //tmp노드의 prev가 비어있지 않는 동안에
				tmp = tmp->prev; //prev를 가리킨다. -> 계속 왼쪽으로 이동

			while(tmp != NULL) //tmp노드가 비어있지 않은 동안
			{
				compare_tree(root1->child_head, tmp, result); //child_head를 가리키는 root1과 tmp를 compare_tree에 넣어 비교한다.
			
				if(*result == true) //result가 가리키는게 1이라면
					break; //반복문에서 나오고
				else{
					if(tmp->next != NULL) //그렇지 않을때 tmp가 가리키는 next가 비어있지 않다면 result가 1을 가리키도록 한다.
						*result = true;
					tmp = tmp->next; //tmp의 next노드를 tmp가 가리키도록 한다.
				}
			}
		}
		else{
			compare_tree(root1->child_head, root2->child_head, result); //root1의 child_head를 가리키는 것과 root2의 child_head를 가리키는 것을 비교
		}
	}	


	if(root1->next != NULL){ //root1의 next가 비어있지않다면 

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){ //root1과 같은 부모를 가진 노드의 갯수와 root2와 같은 부모를 가진 노드의 갯수가 같지 않다면
			*result = false; //result는 0을 가리킨다.
			return;
		}

		if(*result == true) //result가 1을 가리킨다면
		{
			tmp = get_operator(root1); //root1의 연산자를 tmp가 가르키도록 한다
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*") 
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{ //tmp의 name이 다음의 연산자 중 하나라도 같다면 	
				tmp = root2; //tmp는 root2를 가르킨다.
	
				while(tmp->prev != NULL) //tmp의 prev가 비어있지 않는 동안
					tmp = tmp->prev; //tmp의 prev를 tmp가 가리킨다.

				while(tmp != NULL) //tmp가 비어있지 않은 동안
				{
					compare_tree(root1->next, tmp, result); //root1의 next와 tmp를 compare_tree에 넣어 비교한다.

					if(*result == true) //만약 result가 가르키는 것이 1이라면
						break; //반복문에서 나오고
					else{
						if(tmp->next != NULL) //tmp의 next가 비어있지 않다면
							*result = true; //result는 1을 가리킨다.
						tmp = tmp->next; //tmp의 next노드를 tmp가 가리키게 된다.
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result); //tmp가 위의 연산자가 아니라면 root1의 next노드와 root2의 next노드를 compare_tree에 넣어 비교한다.
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]) //정답 문자열을 tokens으로 만드는 함수
{
	char *start, *end;
	char tmp[BUFLEN]; 
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; 
	int row = 0; //열
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;
	
	clear_tokens(tokens); //token 초기화

	start = str; //인자로 받은 배열을 start가 가리킨다.
	
	if(is_typeStatement(str) == 0) //입력받은 문자열의 type확인
		return false; //return 0
	
	while(1)
	{
		if((end = strpbrk(start, op)) == NULL) //start의 문자열에서 op와 일치하는 것이 있는지 확인 있다면 그 문자열을 end가 가리키게 됨
			break; //없다면 나가고

		if(start == end){ //start의 문자열과 end의 문자열이 같다면

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){ //start의 문자열 중 "--"과 같거나 "++"와 같다면
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4)) //start의 문자열 중  "++++"와 같거나 "----"와 같다면
					return false; //return 0

				if(is_character(*ltrim(start + 2))){ //start에 "++"와 "--"을 제외한 문자열을 왼쪽의 공백을 제거하여 그 문자가 숫자나 알파벳이라면 
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])) //열이 0보다 크고 tokens의 마지막 문자가 숫자나 알파벳이라면
						return false; //return 0

					end = strpbrk(start + 2, op); //start에서 "++" 또는 "--"을 제화한 문자열 중 op와 일치하는 것
					if(end == NULL) //일치하는 연산자가 없다면
						end = &str[strlen(str)]; //str의 널문자 주소를 가리킨다.
					while(start < end) { //str의 문자열 끝까지
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) //start에 있는 ++,--바로 전이  공백문자이고 그 줄의 마지막 문자가 숫자나 알파벳이라면
							return false; //return 0
						else if(*start != ' ') //start의 문자가 공백이 아니라면
							strncat(tokens[row], start, 1); //tokens배열에 그 문자를 쓴다. 
						start++; //start의 시작 포인터 위치 이동
					}
				}
				
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ //row가 0보다 크고 다음의 token의 마지막 문자가 숫자나 알파벳이라면
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) //token의 문자열에 "++"가 있거나 "--"가 있다면
						return false; //return 0

					memset(tmp, 0, sizeof(tmp)); //tmp초기화
					strncpy(tmp, start, 2); //tmp에 start의 문자 2개를 쓴다.
					strcat(tokens[row - 1], tmp); //token배열에 tmp를 합친다.
					start += 2; //start의 시작포인터 위치 이동
					row--; //row 감소
				}
				else{
					memset(tmp, 0, sizeof(tmp)); //tmp초기화
					strncpy(tmp, start, 2); //tmp에 start의 문자 2개를 쓴다. 
					strcat(tokens[row], tmp); //tokens배열에 tmp를 합친다.
					start += 2; //start의 시작포인터 위치 이동
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ //start의 문자가 다음의 연산자 중 하나라도 일치한다면

				strncpy(tokens[row], start, 2); //tokens 배열에 다음의 연산자를 쓴다.
				start += 2; //start의 시작포인터 위치 이동
			}
			else if(!strncmp(start, "->", 2)) //start의 문자열에 "->"이 있다면
			{
				end = strpbrk(start + 2, op); //start의 시작포인터를 2칸 이동한 위치의 값이 op의 문자와 일치한다면 그 문자의 위치를 가리킨다.

				if(end == NULL) //일치하는 문자가 없다면
					end = &str[strlen(str)]; //str의 끝 널문자의 위치를 가리킨다.

				while(start < end){ //str 문자열 끝까지 검사
					if(*start != ' ') //start의 문자가 공백문자가 아니라면
						strncat(tokens[row - 1], start, 1); //tokes배열에 start문자를 합친다.
					start++; //start의 시작 포인터 위치 이동
				}
				row--; //row 감소
			}
			else if(*end == '&') //end가 가리키는 문자가 '&'라면
			{
				
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){ //row가 0이거나 tokens배열에 op와 일치한다면
					end = strpbrk(start + 1, op); //start의 포인터 위치를 하나 이동하여 op와 일치한다면 그 문자의 위치를 가리킨다.
					if(end == NULL) //일치하지 않다면
						end = &str[strlen(str)]; //end는 str의 끝 널문자의 위치를 가리킨다.
					
					strncat(tokens[row], start, 1); //token배열에 start의 문자 하나를 합친다.
					start++; //start의 시작포인터 위치 이동

					while(start < end){ //str 문자열 끝까지 검사
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') //start의 시작포인터의 전 문자가 공백문자이고 다음의 tokens의 문자가 '&'가 아니라면 
							return false; //return 0
						else if(*start != ' ') //start 포인터의 가리키는 문자가 공백문자가 아니라면
							strncat(tokens[row], start, 1);  //token배열에 다음의 위치에 start문자를 합친다.
						start++; //start의 시작포인터 위치 이동
					}
				}
				
				else{
					strncpy(tokens[row], start, 1); //다음 token문자에 start 문자를 쓴다.
					start += 1; //start의 시작포인터 위치 이동
				}
				
			}
		  	else if(*end == '*') //end가 가리키는 문자가 '*'라면
			{
				isPointer=0; //초기화

				if(row > 0)
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) {
						if(strstr(tokens[row - 1], datatype[i]) != NULL){ //token배열에 datatype의 문자열이 있다면
							strcat(tokens[row - 1], "*"); //tokens배열에 다음의 위치에 "*" 를 넣고
							start += 1; //start의 시작포인터를 이동
							isPointer = 1;  //pointer가 가리키는게 있음
							break;
						}
					}
					if(isPointer == 1) //pointer
						continue;
					if(*(start+1) !=0) //start의 시작포인터를 이동했을때 그 문자가 null이 아니라면
						end = start + 1; //end에는 그 문자부터의 문자열을 가리키게 된다.

					
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){ //row가 1보다 크고 tokens[r-2]가 "*"이고 그 tokens이 모두 "*"로 이우러져 있다면
						strncat(tokens[row - 1], start, end - start); //"*"로만 이루어진 tokens에 start의 문자를 end문자열과 start 문자열의 차이만큼 합친다.
						row--; //row 감소
					}
					
					
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ //다음 위치에 있는 tokens의 마지막 문자가 SOH라면 
						strncat(tokens[row], start, end - start); //그 다음 tokens에 start의 문자를 end문자열와 start문자열의 차이만큼 합친다.
					}

					
					else if(strpbrk(tokens[row - 1], op) != NULL){	//tokens배열에 op와 같은 문자가 있다면
						strncat(tokens[row] , start, end - start); //그 다음 token에 start의 문자를 end문자열과 start문자열의 차이만큼 합친다.
							
					}
					else
						strncat(tokens[row], start, end - start); //tokens의 다음 위치에 start의 문자를 end문자열과 start문자열의 차이만큼 합친다.

					start += (end - start); //start의 시작포인터를 이동
				}

			 	else if(row == 0)
				{
					if((end = strpbrk(start + 1, op)) == NULL){ //start의 포인터 위치를 하나 이동하여 그 문자열에 op의 문자가 없다면
						strncat(tokens[row], start, 1); //다음의 tokens배열에 start의 문자를 하나 합친다.
						start += 1; //start의 시작포인터 이동
					}
					else{
						while(start < end){ //str의 문자열 끝까지 검사
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1])) //다음 위치에 start의 문자가 공백문자이고 tokens의 마지막 문자가 숫자나 알파벳이라면
								return false; //return 0
							else if(*start != ' ') //start의 문자가 공백문자라면
								strncat(tokens[row], start, 1); //다음의 tokens 위치에 start의 문자 하나를 합친다.
							start++; //start의 시작포인터 이동
						}
						if(all_star(tokens[row])) //token 배열에 row번째 문자가 '*'라면 
							row--; //row 감소
						
					}
				}
			}
			else if(*end == '(') //end가 가리키는 문자가 "("라면 
			{
				lcount = 0; //초기화
				rcount = 0; //초기화
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){ //row가 0보다 크고 다음의 위치의 tokens문자에 '&'가 있거나 다음 위치의 tokens문자가 '*'와 일차한다면 
					while(*(end + lcount + 1) == '(') //end의 포인터를 하나 이동시켜 그 문자가 '('와 같다면
						lcount++; //lcount 증가
					start += lcount; //start의 포인터만큼 이동

					end = strpbrk(start + 1, ")"); //start의 포인터를 하나 이동시켜 가리키는 문자열에서 ")"와 일치한다면 그 문자의 위치를 가리킨다.

					if(end == NULL) //start가 가리키는 문자열에 ")"가 없다면 
						return false; //return 0
					else{
						while(*(end + rcount +1) == ')') //end의 포인터를 하나 이동시켜 그 문자가 ")"와 같다면
							rcount++; //rcount 증가
						end += rcount; //end의 포인터만큼 이동

						if(lcount != rcount) //lcount와 rcount가 같지 않다면 
							return false; //return 0

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ //row가 1보다 크고 tokens의 해당 위치의 마지막 문자가 숫자 또는 알파벳인 경우 또는 row가 1인 경우에는
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); //그 전 tokens에 start를 하나 이동시켜 다음의 길이만큼 합친다.
							row--; //row 감소
							start = end + 1; //start의 포인터를 end+1 만큼 이동시킨다.
						}
						else{
							strncat(tokens[row], start, 1); //tokens배열에 start의 문자열의 한문자를 쓴다.
							start += 1; //start의 포인터를 이동시킨다.
						}
					}
						
				}
				else{
					strncat(tokens[row], start, 1); //tokens배열에 start의 문자열을 한문자를 쓴다.
					start += 1; //start의 포인터를 이동시킨다. 
				}

			}
			else if(*end == '\"') //end가 가리키는 문자가 '\"'라면
			{
				end = strpbrk(start + 1, "\""); //start의 포인터를 하나 이동시켜 "\""의 문자열이 있다면 그 문자열을 가리킨다.
				
				if(end == NULL) //"\""문자열이 없다면
					return false; //return 0

				else{
					strncat(tokens[row], start, end - start + 1); //tokens배열에 start의 문자열을 다음의 길이만큼 합친다.
					start = end + 1; //start의 포인터를 이동시킨다.
				}

			}

			else{
				
				if(row > 0 && !strcmp(tokens[row - 1], "++")) //row>0고 tokens배열에 "++"가 없다면
					return false; //return 0

				
				if(row > 0 && !strcmp(tokens[row - 1], "--")) //row>0이고 tokens배열에 "--"가 없다면
					return false; //return 0
	
				strncat(tokens[row], start, 1); //tokens배열에 start의 문자열의 한문자를 쓴다.
				start += 1; //start의 포인터를 이동시킨다.
				
			
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){ //배열tokens에 "-","+","--","++"가 하나라도 있다면


				
					if(row == 0) 
						row--; //row 감소

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ //tokens의 마지막 문자가 숫자나 알파벳이라면
					
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL) //tokens배열에 '++'와 '--'가 없다면
							row--; //row감소
					}
				}
			}
		}
		else{ 
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) //tokens[row-1]이 모두"*"로 되어있고 row가 1보다 크고 그 전 토큰의 문자가 숫자나 알파벳이라면
				row--; //row 감소

			if(all_star(tokens[row - 1]) && row == 1) //row가 1이고 tokens[0]가 '*'가 아니라면 
				row--;	//row 감소

			for(i = 0; i < end - start; i++){
				if(i > 0 && *(start + i) == '.'){ //i>0고 start의 포인터를 i만큼 이동한 문자가 '.'가 같다면 
					strncat(tokens[row], start + i, 1); //tokens배열에 start의 포인터를 i만큼 이동한 문자열의 한문자를 쓴다.

					while( *(start + i +1) == ' ' && i< end - start ) //start의 시작포인터를 i+1만큼 이동시킨 문자가 공백문자이고 i<end-start보다 작은 동안
						i++; //i증가
				}
				else if(start[i] == ' '){ //start의 i번째 문자가 공백문자라면
					while(start[i] == ' ') //start의 i번째 문자가 공백문자인 동안
						i++; //i증가
					break;
				}
				else
					strncat(tokens[row], start + i, 1); //tokens배열에 start[i]의 문자를 쓴다.
			}

			if(start[0] == ' '){ //start의 첫 문자가 공백문자라면
				start += i; //start의 포인터는 i만큼 이동한다.
				continue; 
			}
			start += i; //start의 위치포인터를 i만큼 이동시킨다.
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //tokens[row]의 문자열에 오른쪽 여백을 제거하고 왼쪽 여백을 제거한 문자열을 tokens[row]에 쓴다.

		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 2]) //row가 0보다 크고 다음의 위치의 token배열의 마지막 문자가 숫자나 알파벳, gcc나 datatpye인 경우
				&& (is_typeStatement(tokens[row - 1]) == 2  
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) //다음의 tokens 배열에 "("가 있다면
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0) //그 다음 tokens이 "struct"가 아니고 "usigned"가 아니라면
					return false; //return 0
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row가 1이고 tokens배열의 마지막 문자가 숫자나 알파벳인 경우
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	//첫번째 토큰에 "extern","unsigned"가 없고 datatype문자열이 아니라면
					return false; //return 0
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ //row가 1보다 클때 tokens의 문자열이 gcc,datatype라면
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) //다음 tokens 배열에 "unsigned","extern"이 없다면
					return false; //retun 0
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){ //tokens의 첫번째 문자열이 "gcc"인 경우
			clear_tokens(tokens); //tokens 초기화
			strcpy(tokens[0], str);	//tokens의 첫번째 문자열에 str의 문자열을 쓴다.
			return 1;
		} 

		row++; //row==1
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  //row>1일때 tokens의 문자열이 '*'이고 그 다음의 문자열의 마지막 문자가 숫자나 알파벳이 아니라면
		row--;	//row 감소			
	if(all_star(tokens[row - 1]) && row == 1)   //row가 1일때 tokens의 첫번째 문자열이 다 '*'이라면
		row--;	//row 감소

	for(i = 0; i < strlen(start); i++) //start의 문자열 길이만큼 반복  
	{
		if(start[i] == ' ')  //공백문자라면
		{
			while(start[i] == ' ') //공백문자인 동안
				i++; //i증가 
			if(start[0]==' ') { //start의 첫 문자가 공백문자라면
				start += i; //i만큼 start의 포인터위치 이동
				i = 0; //i 초기화
			}
			else
				row++; //row증가
			
			i--; //i감소
		} 
		else
		{
			strncat(tokens[row], start + i, 1); //row번째 tokens에 start의 포인터를 i만큼 이동시킨 뒤 그 문자를 쓴다.
			if( start[i] == '.' && i<strlen(start)){ //start[i]번째 문자가 '.'이고 i가 start의 문자열 길이보다 작다면
				while(start[i + 1] == ' ' && i < strlen(start)) //그 다음 문자가 공백문자이고 i가 start의 문자열의 길이보다 작을 동안
					i++; //i를 증가시킨다. 

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //tokens[row]에 있는 문자열의 오른쪽, 왼쪽 공백을 제거한 문자열을 다시 tokens[row]에 쓴다.

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){ //row가 0보다 클때 tokens[row]에 "lpthread"가 있고 그 전 위치에는 "-"가 있다면
			strcat(tokens[row - 1], tokens[row]); //"-"가 있던 배열에 "lpthead"도 합친다.
			memset(tokens[row], 0, sizeof(tokens[row])); //"lpthead"가 있던 tokens은 초기화
			row--; //row감소
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) //row가 0보다 클때 tokens[row]의 마지막 문자가 숫자나 알파벳이고
				&& (is_typeStatement(tokens[row - 1]) == 2  //그 전 문자열이 gcc,datatype일때
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) //그 datatype을 가진 문자열의 마지막 문자가 숫자,알파벳이거나 '.'이라면
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){
			
			if(row > 1 && strcmp(tokens[row-2],"(") == 0) //row가 1보다 크고 다음 tokens의 문자열이 "("일때
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0) //그 다음 문자열이 "struct","unsigned"가 아니라면
					return false; //return 0
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row가 1일때 tokens[1]의 마지막 문자가 null문자가 아니라면
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) //그 전 tokens의 문자열이 "extern","unsigned"가 아니고 datatype도 아니라면
					return false; //return 0
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ //row가 1보다 클때 tokens의 문자열이 gcc, datatype이라면
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) //그 전 문자열이 "unsigned","extern"가 아닐때
					return false; //return 0
			}
		} 
	}


	if(row > 0) //row가 0보다 클때
	{

		
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ //tokens의 첫 문자열이 "#include"이거나 "include" 또는 "struct"라면
			clear_tokens(tokens); //tokens초기화
			strcpy(tokens[0], remove_extraspace(str)); //첫번째 tokens에 문자열 내에 공백문자를 제거한 문자열을 쓴다.
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){ //tokens의 첫 문자열이 datatype이거나 "extern"인 경우
		for(i = 1; i < TOKEN_CNT; i++){ //TOKEN_CNT만큼 반복
			if(strcmp(tokens[i],"") == 0)  //tokens의 문자열이 비어있다면
				break; //나가고	       

			if(i != TOKEN_CNT -1 ) //i가 마지막 토큰이 아니라면
				strcat(tokens[0], " "); //tokens의 첫 문자열에 공백문자 넣기
			strcat(tokens[0], tokens[i]); //tokens의 첫 문자열에 tokens[i]번째 문자열 합치기
			memset(tokens[i], 0, sizeof(tokens[i])); //tokens[i] 초기화
		}
	}
	
	
	while((p_str = find_typeSpecifier(tokens)) != -1){  //p_str은 형 지정자의 위치
		if(!reset_tokens(p_str, tokens)) //형 지정자를 기준으로 토큰이 초기화되지 않는다면
			return false;
	}

	
	while((p_str = find_typeSpecifier2(tokens)) != -1){  //struct 형 이라면
		if(!reset_tokens(p_str, tokens)) //형 지정자를 기준으로 토큰이 초기화되지 않는다면
			return false;
	}
	
	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses)
{
	node *cur = root;
	node *new;
	node *saved_operator;
	node *operator;
	int fstart;
	int i;

	while(1)	
	{
		if(strcmp(tokens[*idx], "") == 0) //토큰이 없다면
			break; //나가고
	
		if(!strcmp(tokens[*idx], ")")) //토큰이 ')' 라면
			return get_root(cur); //그 토큰의 root 노드를 리턴

		else if(!strcmp(tokens[*idx], ",")) //토큰이 ","라면 
			return get_root(cur); //그 토큰의 root노드를 리턴

		else if(!strcmp(tokens[*idx], "(")) //토큰이 "("라면
		{
			
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){ //그 전 토큰이 연산자나 ","이 아니라면
				fstart = true; //tree만들기 시작

				while(1)
				{
					*idx += 1; // 인덱스 1추가

					if(!strcmp(tokens[*idx], ")")) //"(" 토큰의 바로 다음이 ")"이라면
						break; //나가고
					
					new = make_tree(NULL, tokens, idx, parentheses + 1); //아니라면 그 트리에는 괄호를 하나 가지고 있다고 생각하고 다시 재귀적으로 반복
					
					if(new != NULL){ //새롭게 만든 tree가 비어있지않다면
						if(fstart == true){ //tree만들기 시작
							cur->child_head = new; //현재 root노드의 child_head에 new의 반환노드를 넣고
							new->parent = cur; //그 new의 paret에 현재의 root노드를 넣어서 연결한다.
	
							fstart = false; //이제 그 노드들이 연결되어있으므로 
						}
						else{
							cur->next = new; //현재의 노드의 다음 노드에 new의 root노드를 넣고
							new->prev = cur; //new의 pre노드에는 현재노드를 넣는다.
						}

						cur = new; //현재노드는 new의 root노드가 된다.
					}

					if(!strcmp(tokens[*idx], ")")) //바로 다음 토큰이 ")"라면 
						break; //나가고
				}
			}
			else{ //그 전 토큰이 연산자 또는 ","라면
				*idx += 1; //인덱스에 1추가
	
				new = make_tree(NULL, tokens, idx, parentheses + 1); //다음 인덱스를 기준으로 토큰의 root노드를 구한다.

				if(cur == NULL) //root가 비어있다면
					cur = new; //현재노드는 new의 반환노드를 가리키게 되고

				else if(!strcmp(new->name, cur->name)){ //new의 name과 현재의 노드의 name이 같다면
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||")  //new의 name이 다음 중 하나라도 같다면
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&")) 
					{
						cur = get_last_child(cur); //자식 노드 중 가장 오른쪽에 위치한 노드

						if(new->child_head != NULL){ //new의 child_head가 비어있지 않다면
							new = new->child_head; //그child_head를 가리키게 되고

							new->parent->child_head = NULL; //그 노드의 부모 노드의 child_head를 비운다.
							new->parent = NULL; //new의 parent도 비우고
							new->prev = cur; //new의 이전 노드는 자식 노드 중 가장 오른쪽에 위치한 노드를 넣고
							cur->next = new; //cur의 오른쪽에는 new노드를 넣는다.
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*")) //new가 "+","*"연산자라면
					{
						i = 0; 

						while(1)
						{
							if(!strcmp(tokens[*idx + i], "")) //다음 tokens이 비어있다면
								break; //나가고

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) //tokens이 연산자이고 ")"가 아니라면 
								break; //나가고

							i++; //위의 두 조건이 아니면 i는 계속 증가
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name)) //그 다음 토큰보다 new 노드가 연산자 우선순위가 낮다면
						{
							cur = get_last_child(cur); //현재 노드의 child중 가장 오른쪽에 위치한 것
							cur->next = new; //cur의 오른쪽 노드에 new노드를 넣는다.
							new->prev = cur; //new의 왼족 노드에 cur노드를 넣는다.
							cur = new; //cur가 new의 노드를 가리키게 된다.
						}
						else //연산자 우선순위가 높다면
						{
							cur = get_last_child(cur); //현재 노드의 child 중 가장 오른쪽에 위치한 것

							if(new->child_head != NULL){ //new의 child_head가 비어있지 않다면
								new = new->child_head; //new는 그 child_head를 가리키게 된다.

								new->parent->child_head = NULL; //new의 parent의 child_head를 비우고
								new->parent = NULL; //그 parent도 비운다.
								new->prev = cur; //new의 왼쪽 노드에 cur를 넣고
								cur->next = new; //cur의 오른쪽 노드에 new를 넣는다.
							}
						}
					}
					else{
						cur = get_last_child(cur); //현재 노드의 child 중 가장 오른쪽에 위치한 것
						cur->next = new; //cur의 오른쪽에는 next노드를 넣고
						new->prev = cur; //new의 왼쪽에는 cur노드를 넣는다.
						cur = new; //cur가 new노드를 가리키게 한다.
					}
				}
	
				else 
				{
					cur = get_last_child(cur); //현재 노드의 child 중 가장 오른쪽에 위치한 것

					cur->next = new; //현재 노드의 오른쪽에 new 노드를 넣는다.
					new->prev = cur; //new 노드의 왼쪽에 cur노드를 넣는다.
	
					cur = new; //cur가 new 노드를 가리키게 된다. 
				}
			}
		}
		else if(is_operator(tokens[*idx])) //토큰이 연산자라면
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{ //토큰이 다음의 연산자 중 하나라면
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx])) //현재 노드의 이름이 위의 연산자 중 하나라면
					operator = cur; //연산자 노드가 현재 노드가 됨 
		
				else
				{
					new = create_node(tokens[*idx], parentheses); //다음의 연산자를 가진 노드를 새로 생성
					operator = get_most_high_precedence_node(cur, new); //operator 노드는 cur와 new의 최상의 노드를 가리킨다. 

					if(operator->parent == NULL && operator->prev == NULL){ //operator의 상위노드가 없고 이전 노드가 없다면

						if(get_precedence(operator->name) < get_precedence(new->name)){ //operator의 연산자와 new의 연산자 중 operator의 우선순위가 더 높다면 
							cur = insert_node(operator, new); //현재노드는 삽입한 노드를 가리키게 된다.
						}

						else if(get_precedence(operator->name) > get_precedence(new->name)) //operator와 new의 상위노드를 비교하였을때 name의 우선순위가 더 높다면
						{
							if(operator->child_head != NULL){ //operator의 child_head가 있다면
								operator = get_last_child(operator); //operator는 operator의 가장 오른쪽에 있는 자식노드를 가리키게 한다.
								cur = insert_node(operator, new); //현재노드는 삽입한 노드를 가리키게 된다.
							}
						}
						else //둘의 우선순위가 같다면
						{
							operator = cur; //operator는 cur의 노드를 가리키게 된다.
	
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx])) //operator가 연산자이고 tokens의 연산자와 같다면
									break; //나가고
						
								if(operator->prev != NULL) //operator의 이전노드가 있다면
									operator = operator->prev; //operator는 operator의 이전노드를 가리키게 된다
								else //이전노드가 없다면
									break; //나간다.
							}

							if(strcmp(operator->name, tokens[*idx]) != 0) //operator와 연산자와 tokens의 연산자가 같지 않다면
								operator = operator->parent; //operator는 operator의 상위노드를 가리킨다.

							if(operator != NULL){ //operator가 비어있지 않다면
								if(!strcmp(operator->name, tokens[*idx])) //operator의 연산자와 tokens이 같다면
									cur = operator; //현재 노드는 operator를 가리킨다.
							}
						}
					}

					else
						cur = insert_node(operator, new); //현재노드는 삽입한 노드를 가리키게 된다.
				}

			}
			else 
			{
				new = create_node(tokens[*idx], parentheses); //new는 새로 생성한 노드를 가리키게 된다.

				if(cur == NULL) //cur가 비어있다면
					cur = new; //cur는 새로 생성한 노드를 가리키게 한다

				else
				{
					operator = get_most_high_precedence_node(cur, new); //operator는 cur와 new의 최상위노드를 가리킨다.

					if(operator->parentheses > new->parentheses) //operator의 괄호 갯수가 new의 괄호갯수보다 크다면
						cur = insert_node(operator, new); //cur는 삽입한 노드를 가리키게 된다.

					else if(operator->parent == NULL && operator->prev ==  NULL){ //operator가 최상위노드라면
					
						if(get_precedence(operator->name) > get_precedence(new->name)) //operator의 연산자와 new의 연산자 중 new의 우선순위가 더 높다면
						{
							if(operator->child_head != NULL){ //operator의 child_head가 비어있지 않다면
	
								operator = get_last_child(operator); //operator는 operator의 가장 마지막에 있는 노드를 가리키게 된다.
								cur = insert_node(operator, new); //현재노드는 삽입한 노드를 가리키게 된다.
							}
						}
					
						else	
							cur = insert_node(operator, new); //cur는 operator를 삽입한 new 노드를 가리키게 된다.
					}
	
					else
						cur = insert_node(operator, new); //cur는 operator를 삽입한 new노드를 가리키게 된다.
				}
			}
		}
		else 
		{
			new = create_node(tokens[*idx], parentheses); //tokens을 이름으로 갖는 새로운 노드를 생성

			if(cur == NULL) //cur가 비어있다면
				cur = new; //cur는 새로 만든 노드를 가리키게 된다.

			else if(cur->child_head == NULL){ //cur의 child_head가 비어있다면
				cur->child_head = new; //cur는 child_head에 new노드를 가리키게 된다
				new->parent = cur; //new의 parent 노드는 cur를 가리키게 된다.

				cur = new; //cur는 new노드를 가리키게 된다.
			}
			else{

				cur = get_last_child(cur); //cur의 자식 노드 중 가장 마지막에 있는 노드

				cur->next = new; //cur의 다음 노드에 new노드 넣기
				new->prev = cur; //new의 이전 노드에 cur노드 넣기

				cur = new; //new노드를 가리킴
			}
		}

		*idx += 1; //인덱스 1 증가
	}

	return get_root(cur);
}

node *change_sibling(node *parent) //같은 부모를 가지고 있는 노드로 바꾼다.
{
	node *tmp;
	
	tmp = parent->child_head; //인자로 받은 노드의 child_head를 가리킨다.

	parent->child_head = parent->child_head->next; //child_head가 child_head의 다음 노드를 가리킨다.
	parent->child_head->parent = parent; //child_head와 child_head의 next가 같은 부모를 가리킨다.
	parent->child_head->prev = NULL; //child_head의 next의 prev를 초기화한다.

	parent->child_head->next = tmp; //parent의 child_head의 next의 next가 parent의 child_head가 된다.
	parent->child_head->next->prev = parent->child_head; //tmp의 prev는 parent의 child_head를 가리키게 된다.
	parent->child_head->next->next = NULL; //초기화
	parent->child_head->next->parent = NULL; //초기화		

	return parent; //바꾼 노드를 반환
}

node *create_node(char *name, int parentheses) //새로운 노드 생성
{
	node *new;

	new = (node *)malloc(sizeof(node)); //동적할당
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1)); //name의 동적할당
	strcpy(new->name, name); //인자로 받은 name의 문자열을 new의 name에 쓴다.

	new->parentheses = parentheses; //new의 괄호도 인자로 받은 값
	new->parent = NULL; //new의 parent는 초기화
	new->child_head = NULL; //new의 child_head도 초기화
	new->prev = NULL; //new의 pre 초기화
	new->next = NULL; //new의 next 초기화

	return new; //새로 만든 노드를 리턴
}

int get_precedence(char *op) //연산자 우선순위 구하기
{
	int i; 

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op)) //연산자 struct구조체에 인자로 받은 연산자가 존재한다면
			return operators[i].precedence; //그 연산자의 우선순위 구하기
	}
	return false;
}

int is_operator(char *op) //연산자인지 확인
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL) //연산자 구조체에 연산자가 없다면
			break;
		if(!strcmp(operators[i].operator, op)){ //연산자 구조체에 있는 연산자와 인자로 받은 연산자가 같다면
			return true; //return true
		}
	}

	return false;
}

void print(node *cur) //현재 노드 출력
{
	if(cur->child_head != NULL){ //현재 노드의 child_head가 비어있지 않다면
		print(cur->child_head); //child_head를 출력
		printf("\n"); //줄바꿈
	}

	if(cur->next != NULL){ //현재 노드의 next가 비어있지 않다면
		print(cur->next); //next 노드 출력
		printf("\t"); //공백
	}
	printf("%s", cur->name); //현재 노드 출력
}

node *get_operator(node *cur) //operator 구하기
{
	if(cur == NULL) //현재 노드가 비어있다면
		return cur;

	if(cur->prev != NULL) //현재 노드의 바로 전 노드가 비어있지 않다면
		while(cur->prev != NULL) //비어있지 않은 동안
			cur = cur->prev; //전 노드를 계속 가리키게 된다.

	return cur->parent; //child_head가 되어서 그때의 parent를 반환
}

node *get_root(node *cur) //root 구하기
{
	if(cur == NULL) //현재 노드가 비어있다면
		return cur; //리턴

	while(cur->prev != NULL) //현재 노드의 바로 전 노드가 비어있지 않은 동안
		cur = cur->prev; //전 노드를 계속 가리키게 된다.

	if(cur->parent != NULL) //child_head로 갔을때 그 parent가 비어있지 않다면
		cur = get_root(cur->parent); //그 parent의 root노드를 구한다.

	return cur; //구한 root노드 반환
}

node *get_high_precedence_node(node *cur, node *new) //상위 노드를 얻는 함수
{
	if(is_operator(cur->name)) //cur가 연산자라면
		if(get_precedence(cur->name) < get_precedence(new->name)) //cur의 연산자 우선순위와 new의 연산자 우선순위를 비교한다.
			return cur; //new가 더 크다면 cur를 리턴

	if(cur->prev != NULL){ //cur의 왼쪽 노드가 있다면
		while(cur->prev != NULL){ //cur의 왼쪽 노드가 비어있지 않은 동안
			cur = cur->prev; //cur의 왼쪽 노드를 가리키게 됨
			
			return get_high_precedence_node(cur, new); //재귀적으로 반복
		}


		if(cur->parent != NULL) //cur의 부모노드가 비어있지 않다면
			return get_high_precedence_node(cur->parent, new); //재귀적으로 반복
	}

	if(cur->parent == NULL) //cur의 parent가 비어있다면
		return cur; //cur를 리턴
}

node *get_most_high_precedence_node(node *cur, node *new) //최상위 노드를 얻는 함수
{
	node *operator = get_high_precedence_node(cur, new); //연산자 노드는 cur와 new중에 상위노드를 가리킨다. 
	node *saved_operator = operator; //연산자 저장해놓기

	while(1)
	{
		if(saved_operator->parent == NULL) //saved_operator의 parent가 비어있다면 최상위노드이므로
			break; //나가기 

		if(saved_operator->prev != NULL) //saved_operator의 이전 노드가 있다면
			operator = get_high_precedence_node(saved_operator->prev, new); //operator는 saved_operator의 이전 노드와 new노드 중 상위노드를 가리킨다.

		else if(saved_operator->parent != NULL) //그렇지 않고 saved_operator의 상위노드가 있다면
			operator = get_high_precedence_node(saved_operator->parent, new); //operator는 saved_operator의 상위노드와 new노드 중 상위노드를 가리킨다.

		saved_operator = operator; //새로 탐색을 위해 saved_operator는 operator를 가리킨다.
	}
	
	return saved_operator; //최상위노드 리턴
}

node *insert_node(node *old, node *new) //노드 삽입
{
	if(old->prev != NULL){ //인자로 받은 old의 이전 노드가 있다면
		new->prev = old->prev; //new의 이전 노드는 old의 이전 노드를 가리키게 된다.
		old->prev->next = new; //old의 이전 노드의 오른쪽 노드는 new이고
		old->prev = NULL; //old의 이전노드를 비운다.
	}

	new->child_head = old; //new의 child_head가 old이고
	old->parent = new; //old의 parent가 new 노드를 가리킨다.

	return new; //new 리턴
}

node *get_last_child(node *cur) //마지막 child를 구하는 함수
{
	if(cur->child_head != NULL) //현재 노드의 child_head가 비어있지 않다면
		cur = cur->child_head; //현재 노드는 그 child_head가 되고

	while(cur->next != NULL) //child_head의 next가 존재하는 동안
		cur = cur->next; //cur는 그 노드의 next가 된다.

	return cur; //last_child return
}

int get_sibling_cnt(node *cur) //같은 parent를 가리키는 노드의 갯수 
{
	int i = 0;

	while(cur->prev != NULL) //현재 노드의 prev가 비어있지 않은 동안
		cur = cur->prev; 

	while(cur->next != NULL){ //현재 노드의 next가 비어있지 않은 동안
		cur = cur->next;
		i++;
	}

	return i;
}

void free_node(node *cur) //노드 메모리 해제
{
	if(cur->child_head != NULL) //노드의 child_head가 비어있지 않다면
		free_node(cur->child_head); //재귀적으로 돌린다.

	if(cur->next != NULL) //노드의 다음 노드가 비어있지 않다면
		free_node(cur->next); //재귀적으로 돌린다.

	if(cur != NULL){ //노드가 비어있지 않다면
		cur->prev = NULL; //이전 노드도 비우고
		cur->next = NULL; //next 노드도 비우고
		cur->parent = NULL; //parent 노드도 비우고
		cur->child_head = NULL; //child_head도 비움
		free(cur); //메모리 해제
	}
}


int is_character(char c) //문자 c 확인
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); //문자 c가 숫자, 영어의 대소문자라면 아스키코드 리턴 
}

int is_typeStatement(char *str) //gcc,datatype인지 확인하는 함수
{ 
	char *start; 
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 
	
	start = str; //인자로 받은 문자열을 start가 가리킨다
	strncpy(str2,str,strlen(str)); //str2 배열에 str의 문자열을 쓴다.
	remove_space(str2); //공백 문자 제거

	while(start[0] == ' ') //start의 첫 문자가 공백문자라면
		start += 1; //다음 위치로 이동

	if(strstr(str2, "gcc") != NULL) //str2의 문자열에서 gcc로 시작하는 문자열 검색
	{
		strncpy(tmp2, start, strlen("gcc")); //tmp2배열에 start의 gcc 길이만큼의 데이터를 쓴다.
		if(strcmp(tmp2,"gcc") != 0) //tmp2의 문자열이 gcc가 아닌 경우
			return 0; //0리턴
		else
			return 2; //아니라면 2리턴
	}
	
	for(i = 0; i < DATATYPE_SIZE; i++)
	{
		if(strstr(str2,datatype[i]) != NULL) //datatype의 문자열이 str2의 문자열에 있다면
		{	
			strncpy(tmp, str2, strlen(datatype[i])); //tmp 배열에 datatype의 길이만큼 str2의 데이터를 쓴다.
			strncpy(tmp2, start, strlen(datatype[i])); //tmp2 배열에 datatype의 길이만큼 start의 데이터를 쓴다.
			
			if(strcmp(tmp, datatype[i]) == 0) //tmp와 datatype의 문자열이 같다면
				if(strcmp(tmp, tmp2) != 0) //tmp와 tmp2의 문자열이 다르다면
					return 0;  //0 리턴
				else
					return 2; //2 리턴
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) //형지정자 찾기
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0) //tokens의 첫문자열이 아닌 다른 문자열에 datatype이 있다면
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") //그 전 문자열이 "("이고 그 다음 문자열이 ")"이고
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' //")"의 다음 문자열의 첫문자가 '&','*',')','(','-','+'이거나 숫자 또는 영문이라면 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0])))  
					return i; //i return
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) //struct 형 지정자 찾기
{
    int i, j;

   
    for(i = 0; i < TOKEN_CNT; i++)
    {
        for(j = 0; j < DATATYPE_SIZE; j++)
        {
            if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1]))  //tokens[i]의 문자열이 "struct"이고 i+1이 TOKEN_CNT보다 작고 그 다음 문자열의 마지막 문자가 숫자나 영문이라면
                    return i; // i return
        }
    }
    return -1;
}

int all_star(char *str) //"*"로만 이루어져있는지 확인
{
	int i;
	int length= strlen(str); //인자로 받은 포인터 배열의 길이
	
 	if(length == 0)	//배열에 문자가 없다면 
		return 0;
	
	for(i = 0; i < length; i++) 
		if(str[i] != '*') //배열에 '*' 문자가 없다면
			return 0;
	return 1; //인자로 받은 배열이 모두 '*'로 이루어져 있다면 1

}

int all_character(char *str) //문자열 확인
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0;
	
}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) //인자로 받은 토큰을 기준으로 초기화
{
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0; //왼쪽 , 오른쪽
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){ 
		if(!strcmp(tokens[start], "struct")) {	//tokens의 지정자가 "struct"라면	
			strcat(tokens[start], " "); //지정자가 있는 배열에 공백문자를 합치고
			strcat(tokens[start], tokens[start+1]); //그 다음 문자열도 합친다. 

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ //지정자의 다음 문자열부터
				strcpy(tokens[i], tokens[i + 1]); //지정자가 있는 배열의 다음 배열 위치에 그 다음 문자열을 쓴다.
				memset(tokens[i + 1], 0, sizeof(tokens[0])); //지정자의 문자열이 있는 배열의 2만큼 더 간 tokens을 초기화
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) {	//tokens의 지정자가 "unsigned"이고 그 다음 tokens은 ")"라면	
			strcat(tokens[start], " "); //지정자가 있는 배열에 공백문자를 추가하고
			strcat(tokens[start], tokens[start + 1]); //그 다음 문자열 ")"도 합치고	     
			strcat(tokens[start], tokens[start + 2]); //그 다음 문자열도 합친다.

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ //지정된 다음 문자열부터
				strcpy(tokens[i], tokens[i + 1]); //지정자가 있는 배열의 다음 배열 위치에 그 다음 문자열을 쓴다.
				memset(tokens[i + 1], 0, sizeof(tokens[0])); //지정자의 문자열이 있는 배열의 2만큼 더 간 tokens 초기화
			}
		}

     		j = start + 1; //지정자가 있는 문자열의 다음 문자열의 위치          
        	while(!strcmp(tokens[j], ")")){ //j번째 위치 토큰이 ")"라면
                	rcount ++; //rcount 하나 증가
                	if(j==TOKEN_CNT) //j가 마지막 토큰이라면
                        	break; //나가고
                	j++; //j 하나 증가
        	}
	
		j = start - 1; //지정자가 있는 문자열의 전 문자열의 위치
		while(!strcmp(tokens[j], "(")){ //j번째 위치 토큰이 "("라면
        	        lcount ++; //lcount 하나 증가
                	if(j == 0) //j가 0이라면
                        	break; //나가고
               		j--; //j 하나 감소
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0) //j가 0이 아닐때 tokens[j]의 문자열 마지막 문자가 숫자나 영문인 경우 또는 j가 0일때
			lcount = rcount; //같다.

		if(lcount != rcount ) //같지 않다면
			return false; //return 0;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){ //지정자의 위치에서 lcount를 뺀 값이 0보다 크고 tokens의 첫문자열이 sizeof라면 
			return true; //return 1
		}
		
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) { //그렇지 않고 지정자가 "unsigned"인 경우, 지정자가 "struct"이고 그 다음 토큰이 ")"인 경우
			strcat(tokens[start - lcount], tokens[start]); //지정자의 앞의 문자열과 지정자의 문자열을 합친다.
			strcat(tokens[start - lcount], tokens[start + 1]); //합친 문자열에 지정자 뒤의 문자열도 합친다. 
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]); //지정자 문자열에 지정자 뒤에 문자열을 쓴다.
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) { 
				strcpy(tokens[i], tokens[i + lcount + rcount]); //지정자 뒤에 토큰을 지정자 앞에 토큰에 쓰고
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0])); //지정자 뒤에 토큰 초기화
			}


		}
 		else{
			if(tokens[start + 2][0] == '('){ //지정자 토큰에서 오른쪽으로 두번째 위치한 토큰이 '('이라면
				j = start + 2; //j는 그 위치가 되고
				while(!strcmp(tokens[j], "(")){ //tokens[j]가 "("일때
					sub_lcount++; //필수적으로 필요한 괄호 외에 또 괄호가 있음
					j++; //토큰 하나 이동
				} 	
				if(!strcmp(tokens[j + 1],")")){ //'('토큰에서 오른쪽으로 두번째 위치한 토큰이 ")"라면
					j = j + 1; //토큰 하나 이동
					while(!strcmp(tokens[j], ")")){ //tokens[j]가 ")"일때
						sub_rcount++; //필수적으로 필요한 괄호 외에 또 괄호가 있음
						j++; //토큰 하나 이동
					}
				}
				else //'('과 쌍을 이루지 않으면
					return false; //return 0

				if(sub_lcount != sub_rcount) //두개의 갯수가 같아야 하므로
					return false; //return 0
				
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]); //지정자 토큰에서 오른쪽으로 두번째 위치한 토큰에 그 다음 토큰을 쓴다
				for(int i = start + 3; i<TOKEN_CNT; i++) 
					memset(tokens[i], 0, sizeof(tokens[0])); //'(' 뒤의 토큰배열 초기화

			}
			strcat(tokens[start - lcount], tokens[start]); //지정자 토큰의 앞 토큰과 지정자 토큰을 합치고
			strcat(tokens[start - lcount], tokens[start + 1]); //그 토큰에 지정자 토큰의 뒤 토큰도 합치고
			strcat(tokens[start - lcount], tokens[start + rcount + 1]); //그 토큰에 지정자 토큰에서 오른쪽으로 두번째에 위치한 토큰도 합친다.
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {
				strcpy(tokens[i], tokens[i + lcount + rcount +1]); //지정자 토큰에서 오른쪽으로 세번째에 위치한 토큰을 지정자 토큰에 합친다.
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0])); //지정자 토큰에서 오른쪽으로 세번째에 위치한 토큰을 초기화한다.

			}
		}
	}
	return true; //return 1
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN]) //토큰 초기화
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i])); //token 초기화
}

char *rtrim(char *_str) //오른쪽 공백 지우기
{
	char tmp[BUFLEN]; 
	char *end; //_str의 문자열을 가리킴

	strcpy(tmp, _str); //tmp에 _str문자열 내용을 쓴다
	end = tmp + strlen(tmp) - 1; //end은 tmp의 마지막 문자열을 가리킨다
	while(end != _str && isspace(*end)) //end가 공백문자인 동안
		--end; //end 하나씩 감소

	*(end + 1) = '\0'; //end가 가리키는 문자의 다음 포인터는 개행문자를 가리킨다
	_str = tmp; //_srt는 오른쪽 공백을 제거한 tmp를 가리키게 된다.
	return _str;
}

char *ltrim(char *_str) //왼쪽 공백 지우기
{
	char *start = _str; //_str의 문자열을 가리킴

	while(*start != '\0' && isspace(*start)) //start가 문자열의 끝이 아니고 공백문자인 동안
		++start; //하니씩 증가
	_str = start; //왼쪽 공백을 제거한 문자열을 가리키게 된다
	return _str;
}

char* remove_extraspace(char *str) //문자열 내 빈칸 지우기
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN); //동적할당
	char *start, *end;  
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){ //str의 문자열에 "include<"가 있다면
		start = str; //str문자열 가리킴
		end = strpbrk(str, "<"); //"<"가 첫문자인 문자열을 가리킴
		position = end - start; //"include"의 길이
	
		strncat(temp, str, position); //temp에 str문자열을 position만큼 쓴다.
		strncat(temp, " ", 1); //공백문자
		strncat(temp, str + position, strlen(str) - position + 1); //temp에 str에서 position만큼 이동하여을 다음의 문자열 길이만큼 쓴다.

		str = temp; //공백이 포함된 문자열		
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ') //공백문자라면
		{
			if(i == 0 && str[0] ==' ') //첫문자가 공백문자라면
				while(str[i + 1] == ' ') //다음의 문자가 공백문자인 동안
					i++; //i하나 증가
			else{ //첫문자가 공백문자가 아니고 다른 문자가 공백문자인 경우
				if(i > 0 && str[i - 1] != ' ') //그 전 문자가 공백문자가 아니라면
					str2[strlen(str2)] = str[i]; //str문자열에서 i만큼 이동한 문자열을 str2가 가리킨다.
				while(str[i + 1] == ' ') //다음 문자가 공백문자라면
					i++; //i 하나 증가
			} 
		}
		else
			str2[strlen(str2)] = str[i]; //문자열에 공백문자가 없다면 그 문자열을 그대로 가리킨다.
	}

	return str2;
}



void remove_space(char *str) //공백문자 제거
{
	char* i = str; //i,j는 입력받은 인자를 가리킴
	char* j = str;
	
	while(*j != 0) //j의 문자열의 끝이 아닌 동안
	{
		*i = *j++; //i는 j가 가리키는 문자열의 문자 하나씩을 가리키고 j의 위치는 그 다음으로 이동
		if(*i != ' ') //i가 공백문자가 아니라면
			i++; //i의 위치는 그 다음으로 이동
	}
	*i = 0; //i의 문자열의 끝
}

int check_brackets(char *str) //괄호 확인
{
	char *start = str;
	int lcount = 0, rcount = 0;
	
	while(1){
		if((start = strpbrk(start, "()")) != NULL){ //start의 문자열에 "()"가 있다면
			if(*(start) == '(') //start의 시작포인터가 "("라면
				lcount++; //왼쪽 갯수 증가
			else 
				rcount++; //오른쪽 갯수 증가

			start += 1; //start의 시작포인터 이동		
		}
		else
			break; //나가기
	}

	if(lcount != rcount) //괄호의 쌍이 같지 않다면
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]) //토큰의 갯수 세기
{
	int i;
	
	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], "")) //토큰이 비어있다면
			break; //나가기

	return i;
}
