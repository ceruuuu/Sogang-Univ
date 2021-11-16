#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "20181618.h"

int main(){
	int i;
	int k_inst = 0;
	hash* temp;
	char inst[50];
	char* t_str;

	readfile(); // opcode.txt를 읽고 저장
	mem_reset(); // memory 초기화

	history = (his*)malloc(sizeof(his));
	history->next = NULL; // 명령어 history list의 head 초기화

	cur_add = 0; end_flag = 0; cur_i = 0;

	while(end_flag ==  0){ // end_flag가 1이 될때까지 반복
		printf("sicsim> ");
		fgets(inst, 50, stdin);
		inst[strlen(inst)-1] = '\0';

		k_inst = instruction_check(inst); // 올바른 명령어인지, 어떤 명령어인지 반환받는다
		
		if(k_inst != -1)
			add_history(inst); // 올바른 명령어일경우 history에 추가
	
		switch(k_inst){
			case 0: // help
				printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\n");
				printf("f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");			
				break;
			case 1: // dir
				show_directory();
				break;
			case 2: // quit
				end_flag = 1;
				break;
			case 3: // history
				print_history();
				break;	
			case 41: // dump
				Dump(1);
				break;
			case 42: // dump start
				Dump(2);
				break;
			case 43: // dump start, end
				Dump(3);
				break;
			case 5: // dump address, value
				edit();
				break;
			case 6: //fill start, end, value
				fill();
				break;
			case 7: // reset
				mem_reset();
				break;
			case 8: // opcode
				t_str = hash_function(mne);
				if(strcmp(t_str, "\0") == 0) printf(" not exist!\n"); // 리스트에 없을때
				else printf("opcode is %s\n", t_str);
				break;
			case 9: // opcode list
				for(i=0; i<20;i++){
					printf(" %d ",i);

					temp = h_table[i]->next;
					while(temp != NULL){
						printf("-> [%s,%s] ", temp->mnemonic, temp->op);
						temp = temp->next;
					}
					printf("\n");
				}
				break;
			default:
				break;
		}
	}

	//동적할당한 메모리들을 free해준다.
	free_history();
	free_opcode();

	return 0;
}

// =========opcode.txt 파일 열고 읽는 함수=========
//		파일의 정보를 읽고 리스트에 저장한다. 
//		반환값은 없다.
// =========================================
void readfile(){
	FILE* f = fopen("opcode.txt","r");
	char buf[25];

	int i, idx, flag = 0;
	int cnt=0;
	int key = 0;

	if(f==NULL){ // 파일 오픈 실패
		printf(" file open error!\n");
		end_flag = 1;
		return ;
	}

	for(i=0;i<20;i++){
		h_table[i] = (hash*)malloc(sizeof(hash));
		h_table[i]->next = NULL;
	}

	hash *temp;

	while(cnt<58){
		fgets(buf, 25, f); // 한 줄 읽기

		hash *node = (hash*)malloc(sizeof(hash));
		node->next = NULL;
		node->op[0] = buf[0]; node->op[1] = buf[1]; // opcode 저장

		i=1; idx=0; flag=0;
		while(1){ // 명령어 저장
			i++;
			if(buf[i] >= 'A' && buf[i] <= 'Z'){
				node->mnemonic[idx++] = buf[i];
				flag = 1;
			}
			if(flag == 1 && buf[i] == '\t') break;
		}

		key = (node->mnemonic[0]-'A')%21;

		temp = h_table[key];
		while(temp->next != NULL){
			temp = temp->next;
		}

		temp->next = node;

		cnt++;
	}
	fclose(f);

}

// =========hash 함수=========
//	key는 명령의 첫번째 알파벳
//	(key-'A')%21의 값으로 인덱싱하여
//	value를 반환한다.
// ==========================
char* hash_function(char* str){
	int key = (str[0]-'A')%21; // key는 입력받은 명령어의 첫번째 알파벳
							   // 배열의 index는 알파벳 순서로 하되,
							   // W는 20을 넘어가므로 %21를 통해 인덱싱한다.
	hash* temp = h_table[key];

	while(temp != NULL){
		if(strcmp(str, temp->mnemonic) == 0){
			return temp->op;
		}
		temp = temp->next;
	}

	return "\0"; // 검색결과 존재하지 않을경우 NULL
}

// =========현재 디렉토리의 파일 출력=========
//		현재 디렉토리의 경로를 추출하여
//		그 안의 모든 파일을 출력한다.
// ==================================
void show_directory(){
	DIR* dir_p = NULL;
	struct dirent* entry = NULL;
	struct stat buf;

	char *path;
	int cnt = 0;

	path = getcwd(NULL, 500); // 현재 디렉토리의 절대경로 추출

	dir_p = opendir(path); // 경로를 통해 디렉토리를 open한다

	while((entry = readdir(dir_p)) != NULL){ // 폴더 안의 모든 파일들을 검색한다
		lstat(entry->d_name, &buf);
		if(S_ISDIR(buf.st_mode)) printf("%24s/", entry->d_name); // 디렉토리 파일
		else if(S_IEXEC & buf.st_mode) printf("%24s*", entry->d_name); // 실행 파일
		else printf("%25s", entry->d_name); // 그 외 파일
		cnt++;
		if(cnt%4 == 0) printf("\n");
	}
	if(cnt%4 != 0) printf("\n");
	closedir(dir_p);

}

// =========16진수->10진수 변환 함수=========
//	문자열 또는 문자를 받았을 때 10진수로 반환한다.
// ====================================
int hexTodec(char* str, char c, int key){
	int sum, h;

	if(key == 0){ // 문자열이 input일때
		int len = strlen(str);
		int i, a = 1;

		for(i=0;i<len-1;i++){ // 전체 자릿수 맞추기
			a *= 16;
		}

		sum = 0;
		for(i=0;i<len;i++){
			if(str[i] >= '0' && str[i] <= '9')
				h = str[i] - '0';
			else if(str[i] >= 'a' && str[i] <= 'z')
				h = str[i] - 'a' + 10;
			else if(str[i] >= 'A' && str[i] <= 'Z')
				h = str[i] - 'A' + 10;

			sum += h*a;
			a /= 16;
		}	
		return sum;
	}
	else{ // char이 input일때
		if(c >= '0' && c <= '9')
			h = c - '0';
		else if(c >= 'a' && c <= 'z')
			h = c - 'a' + 10;
		else if(c >= 'A' && c<= 'Z')
			h = c - 'A' + 10;
		return h;
	}

}

// =========메모리 reset=========
//	전제 메모리 배열을 0으로 초기화한다.
// ===========================
void mem_reset(){
	int i, j;

	for(i=0;i<65536;i++){
		for(j=0;j<16;j++){
			m[i][j] = 0;
		}
	}

}

// =========10진수->16진수=========
//	인자로 들어온 값을 16진수 문자로 반환한다.
// =============================
char decTohec(int d){
	if(d>=0 && d<=9){
		return d+'0';
	}
	else{
		return d-10+'A';
	}
}

// =========Dump 실행하는 함수=========
//	사용자가 지정한 범위의 메모리를 출력한다.
//  명령어의 종류에 따른 key값을 인자로 받고
//  결과를 출력한다.
// ===============================
void Dump(int key){
	int i, j, a, temp, t; // temp 및 index 변수
	int st_i, st_j, cnt, st_n; // st_i, st_j - 출력 시작할 메모리의 인덱스
						
	int show_addr[5] = { 0, }; // 주소 출력할 배열
	char t_str[4]; // temp 배열

	if(key == 1){ // du[mp] 명령어 처리
		st_i = 0;

		for(i=cur_i;i<10+cur_i;i++){

			temp = cur_add;
			a = 16*16*16*16;
			for(j=0;j<4;j++){ // 출력할 주소 앞의 4자리 계산
				show_addr[j] = temp/a;
				temp -= (temp/a)*a;
				a /= 16;
			}
			
			for(j=0;j<5;j++){ // 주소 출력
				printf("%c", decTohec(show_addr[j]));
			}
			printf("  ");

			for(j=0;j<16;j++){ // 메모리 출력. 저장된 정수값을 16진수로 변환한다.
				t = m[i][j]/16;	
				printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
			}

			printf(" ; ");
			for(j=0;j<16;j++){ // ASCII코드 출력
				if(m[i][j]>=32 && m[i][j]<=126) // 범위는 20 ~ 7E
					printf("%c", m[i][j]);
				else
					printf(".");
			}	
			printf("\n");

			cur_add += 16; // 다음에 출력할 첫번째 주소값
			
			if(i>=65535){ // FFFFF를 넘어갈 경우 초기화
				cur_i = -10;
				cur_add = 0;
			}
		}

		cur_i += 10; // m[i][] index 갱신
	
	}
	else if(key == 2){ // du[mp] start 명령어 처리
		cnt = 0;

		temp = hexTodec(start, '0', 0);
		st_n = temp;

		while(cnt != 160){ // 입력받은 start부터 160개 출력

			a = 16*16*16*16;
			for(j=0;j<4;j++){
				show_addr[j] = temp/a;
				t_str[j] = decTohec(temp/a);
				temp -= (temp/a)*a;
				a /= 16;
			}
			t_str[4] = '\0';
			st_i = hexTodec(t_str, '0', 0); // [st_i][]부터 출력 시작
			st_j = temp;	//[][st_j]부터 출력 시작

			for(j=0;j<5;j++){ // 주소 출력
				printf("%c", decTohec(show_addr[j]));
			}
			printf("  ");

			if(cnt == 0){ // 첫번째 줄
				i = st_i;
				for(j=0;j<st_j;j++){ // 출력되지 않는 부분은 공백처리
					printf("   ");
				}
				for(j=st_j;j<16;j++){
					t = m[i][j]/16;
					printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
					cnt++;
				}

				printf(" ; ");
				for(j=0;j<st_j;j++){ // 출력되지 않는 ASCII코드는 '.'처리
					printf(".");
				}
				for(j=st_j;j<16;j++){
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");

				}
			}
			else if(160-cnt < 16){ // 마지막 줄
				i = st_i;
				st_j = 160 - cnt;

				for(j=0;j<st_j;j++){
					t = m[i][j]/16;
					printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
					cnt++;
				}
				for(j=st_j;j<16;j++){ // 출력되지 않는 부분은 공백처리
					printf("   ");					
				}

				printf(" ; ");
				for(j=0;j<st_j;j++){ 
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");
				}
				for(j=st_j;j<16;j++){ // 출력되지 않는 ASCII코드는 '.'처리
					printf(".");
				}
				
			}
			else{ // 전체 출력되는 줄
				i = st_i;
				for(j=0;j<16;j++){
					t = m[i][j]/16;
					printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
					cnt++;
				}

				printf(" ; ");
				for(j=0;j<16;j++){
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");
				}
			
			}
			printf("\n");
			
			if(i == 65535) break; // 주소값 FFFFF까지 출력될 경우 멈춘다.
			temp = st_n + cnt; // 출력한 만큼 주소값 갱신
		
		}
	}
	else{ // du[mp] start, end 명령어 처리
		cnt = 0;

		temp = hexTodec(start, '0', 0);
		st_n = temp;

		int max = hexTodec(end, '0', 0) - temp+1; // 출력될 byte 숫자 계산
		while(cnt != max){
			a = 16*16*16*16;
			for(j=0;j<4;j++){
				show_addr[j] = temp/a;
				t_str[j] = decTohec(temp/a);
				temp -= (temp/a)*a;
				a /= 16;
			}
			t_str[4] = '\0';
			st_i = hexTodec(t_str, '0', 0); // [st_i][]부터 출력 시작
			st_j = temp;	//[][st_j]부터 출력 시작

			for(j=0;j<5;j++){ // 주소 출력
				printf("%c", decTohec(show_addr[j]));
			}
			printf("  ");

			if(max<=16){ // 출력할 byte 수가 한 줄 이하일 경우
				i = st_i;
				for(j=0;j<st_j;j++){
					printf("   ");
				}
				j = st_j;
				while(cnt != max){ // 메모리 출력
					t = m[i][j]/16;
					printf("%c%c ",decTohec(t), decTohec(m[i][j]-t*16));
					cnt++; j++;
				}
				for(;j<16;j++){
					printf("   ");
				}

				printf(" ; ");
				for(j=0;j<st_j;j++){
					printf(".");
				}
				cnt=0;
				while(cnt != max){ // ASCII코드 출력
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");
					j++; cnt++;
				}
				for(;j<16;j++){
					printf(".");
				}


			}
			else if(cnt == 0){ // 첫번째 줄
				i = st_i;
				for(j=0;j<st_j;j++){
					printf("   ");
				}
				for(j=st_j;j<16;j++){ // 메모리 출력
					t = m[i][j]/16;
					printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
					cnt++;
				}

				printf(" ; ");
				for(j=0;j<st_j;j++){
					printf(".");
				} 
				for(j=st_j;j<16;j++){ // ASCII코드 출력
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");

				}
			}
			else if(max-cnt < 16){ // 마지막 줄
				i = st_i;
				st_j = max - cnt;

				for(j=0;j<st_j;j++){ // 메모리 출력
					t = m[i][j]/16;
					printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
					cnt++;
				}
				for(j=st_j;j<16;j++){
					printf("   ");
				}

				printf(" ; ");
				for(j=0;j<st_j;j++){ // ASCII코드 출력
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");
				}
				for(j=st_j;j<16;j++){
					printf(".");
				}

			}
			else{ // 전체 출력되는 줄
				i = st_i;
				for(j=0;j<16;j++){ // 메모리 출력
					t = m[i][j]/16;
					printf("%c%c ", decTohec(t), decTohec(m[i][j]-t*16));
					cnt++;
				}

				printf(" ; ");
				for(j=0;j<16;j++){ // ASCII코드 출력
					if(m[i][j]>=32 && m[i][j]<=126)
						printf("%c", m[i][j]);
					else
						printf(".");
				}

			}
			printf("\n");
			temp = st_n + cnt; // 주소값 갱신
	
		}		
	}
}

// =========edit 실행하는 함수.=========
//	 입력받은 특정 주소의 값을 수정한다.
// ================================
void edit(){
	int i, j, a;
	int temp = hexTodec(address, '0', 0); // 입력받은 주소값 10진수로 저장
	char t_str[4];

	a = 16*16*16*16;
	for(j=0;j<4;j++){ 
		t_str[j] = decTohec(temp/a);
		temp -= (temp/a)*a;
		a /= 16;
	} 
	t_str[4] = '\0';	
	i = hexTodec(t_str,0,0); // 주소값 앞의 4자리를 10진수로 바꾼 값이 i
	j = temp; // 주소값 맨 끝자리를 10진수로 바꾼 값이 j

	m[i][j] = hexTodec(value, 0, 0); // 입력받은 value 저장
}

// =========fill 실행하는 함수=========
//	지정된 범위의 메모리 값을 수정한다
// ===============================
void fill(){
	int a, i, j, st_i, st_j, st_n; 
	int cnt, max, temp;
	char t_str[4];

	cnt = 0;
	temp = hexTodec(start, 0, 0);
	st_n = temp;
	max = hexTodec(end, 0, 0) - temp + 1; // 출력될 byte 숫자 계산

	while(cnt != max){
		a = 16*16*16*16;
		for(j=0;j<4;j++){
			t_str[j] = decTohec(temp/a);
			temp -= (temp/a)*a;
			a /= 16;
		}
		t_str[4] = '\0';

		st_i = hexTodec(t_str, 0, 0); // 주소값 앞의 4자리를 10진수로 바꾼 값이 i
		st_j = temp; // 주소값 맨 끝자리를 10진수로 바꾼 값이 j

		if(max<=16){ // 갱신할 메모리가 한 줄 이하일때
			i = st_i;
			j = st_j;

			while(cnt != max){
				m[i][j] = hexTodec(value,0,0);
				cnt++; j++;
			}
		}
		else if(cnt == 0){ // 첫번째 줄
			i = st_i;
			for(j=st_j;j<16;j++){
				m[i][j] = hexTodec(value, 0, 0);
				cnt++;
			}
		} // 마지막 줄
		else if(max-cnt < 16){
			i = st_i;
			st_j = max - cnt;
			for(j=0;j<st_j;j++){
				m[i][j] = hexTodec(value, 0, 0);
				cnt++;
			}
		}
		else{ // 전체 출력되는 줄
			i = st_i;
			for(j=0;j<16;j++){
				m[i][j] = hexTodec(value, 0, 0);
				cnt++;
			}
		}
		temp = st_n + cnt; // 주소값 갱신
	}
}

// =========입력받은 값의 범위를 검사하는 함수==========
//	start, end, addresss와 value의 범위 검사
// ========================================
int boundary_check(char* str, int key){ 
	int flag=0; 
	int t,i;
	int len = strlen(str);

	for(i=0;i<len;i++){ // 입력받은 인자는 모두 16진수이다.
		t = str[i];
		if(t>='0' && t<='9'){}
		else if(t>='a' && t<='f'){}
		else if(t>='A' && t<='F'){}
		else flag = 1;
		
		if(key==0){ // start, end, address의 경우
			if(len>5) flag = 1;
		}
		else if(key==1){ // value의 경우
			if(len>2) flag = 1;
		}	
	}

	if(flag == 1) return 1; // 한 번 이라도 플래그가 세워지면 비정상적인 input
	else return 0; // 정상적인 경우
}

// =========정상적인 명령어 history에 추가하는 함수=========
void add_history(char *str){
	his* temp = history;
	his* node = (his*)malloc(sizeof(his));
	
	node->next = NULL;
	strcpy(node->inst, str); // 노드에 명령어 저장
	
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = node;
}

// =========history 출력하는 함수==========
void print_history(){
	int cnt = 1;
	his* temp = history->next;
	
	while(temp!=NULL){
		printf("%10d %s\n", cnt++, temp->inst);
		temp = temp->next;
	}
}

// =========history list free하는 함수=========
void free_history(){
	his* temp = history;
	his* prev = NULL;
	while(temp->next != NULL){
		prev = temp;
		temp = temp->next;
		free(prev);
	}
	free(temp);
}

// =========opcode list free하는 함수=========
void free_opcode(){
	hash* temp;
	hash* prev = NULL;
	for(int i=0;i<20;i++){
		temp = h_table[i];
		while(temp->next != NULL){
			prev = temp;
			temp = temp->next;
			free(prev);
		}
		free(temp);
	}
}

// =========명령어가 정상적인지 결정하는 함수==========
//	어떤 명령어인지, 입력받은 값은 무엇인지, 그 값은 정상적인지
//  검사한 결과를 반환한다.
// ========================================
int instruction_check(char* inst){ 
	int flag = 0;
	int i, idx, len = 0;

	for(i=0;i<10;i++){ // 배열 초기화
		start[i] = '\0';
		end[i] = '\0';
		address[i] = '\0';
		value[i] = '\0';
		mne[i] = '\0';
	}

	if(strcmp(inst, "help") == 0 || strcmp(inst, "h") == 0){
		return 0;
	}
	else if(strcmp(inst, "dir") == 0 || strcmp(inst, "d") == 0){
		return 1;
	}
	else if(strcmp(inst, "quit") == 0 || strcmp(inst, "q") == 0){
		return 2;
	}
	else if(strcmp(inst, "history") == 0 || strcmp(inst, "hi") == 0){
		return 3;
	}
	else if(strncmp(inst, "dump", 4) == 0 || strncmp(inst, "du", 2) == 0){
		len = strlen(inst);
		idx=0; i=0; flag=0;

		if(strcmp(inst, "dump") == 0 || strcmp(inst, "du") == 0)
			return 41; // du[mp]만 입력될 경우

		for(i=0;i<=len;i++){	
			if(flag == 0 && inst[i] == ' '){ // ' '이 있으면 start는 있는 명령어
				flag = 1;
				i++;
			}
			else if(inst[i] == ','){ // ','가 있으면 start, end가 입력된 명령어
				flag = 2;
				idx = 0; i += 2;
			}

			if(flag == 1){ 
				start[idx++] = inst[i];
			}
			else if(flag == 2){
				end[idx++] = inst[i];
			}

			if(flag == 1 && i == len){ // start 받았는데 문장이 끝날 경우
				if(boundary_check(start, 0) == 1){ 
					printf(" input error!\n");
					return -1;
				}
				return 42; // start의 값 체크하고 검사 결과 return
			}
			else if(flag == 2 && i == len){ // start, end 받고 문장 끝날 경우
				if(boundary_check(start, 0) == 1 || boundary_check(end, 0) == 1){
					printf(" input error!\n");
					return -1;
				}
				if(hexTodec(start,0,0)>hexTodec(end,0,0)){
					printf(" start>end!\n");
					return -1;
				}

				return 43; // start, end와 범위 체크하고 검사 결과 return
			}
		}
	}
	else if(strncmp(inst, "edit ", 4) == 0 || strncmp(inst, "e ", 1) == 0){
		len = strlen(inst);
		i = 0; idx = 0; flag=0;

		for(i=0;i<len;i++){

			if(flag == 0 && inst[i] == ' '){ // 공백 이후 address 입력받기
				flag = 1;
				i++;
			}
			else if(inst[i] == ','){ // ',' 이후 value 입력받기
				flag = 2;
				idx = 0; i += 2;
			}

			if(flag == 1){
				address[idx++] = inst[i];
			}
			else if(flag == 2){
				value[idx++] = inst[i];
			}

		}
		
		if(boundary_check(address,0) == 1 || boundary_check(value, 1) == 1){
			printf(" input error!\n");
			return -1;
		} // address, value 값 체크 후 결과 return
		else if(value[0] == '\0'){
			printf(" input error!\n");
			return -1;
		} // value 값이 들어오지 않았을 경우

		return 5;

	}
	else if(strncmp(inst, "fill", 4) == 0 || strncmp(inst, "f", 1) == 0){

		len = strlen(inst);
		flag=0; i=0; idx=0;

		for(i=0;i<len;i++){	
			if(flag == 0 && inst[i] == ' '){ // 첫번째 공백 이후 start입력받음
				flag = 1;
				i++;
			}
			else if(flag == 1 && inst[i] == ','){ // 첫번째 ',' 이후 end 입력받음
				flag = 2;
				idx = 0; i += 2;
			}
			else if(flag == 2 && inst[i] == ','){ // 두번째 ',' 이후 value 입력받음
				flag = 3;
				idx = 0; i += 2;
			}

			if(flag == 1){
				start[idx++] = inst[i];
			}
			else if(flag == 2){
				end[idx++] = inst[i];
			}
			else if(flag == 3){
				value[idx++] = inst[i];
			}
		}
		
		if(boundary_check(start,0) == 1 || boundary_check(end, 0) == 1){
			printf(" input error!\n");
			return -1;
		}
		if(boundary_check(value, 1) == 1){
			printf(" input error!\n");
			return -1;
		}
		if(hexTodec(start,0,0)>hexTodec(end,0,0)){
			printf(" start>end!\n");
			return -1;
		}

		if(value[0] == '\0'){
			printf(" input error!\n");
			return -1;
		}
		return 6; // start, end, value 및 범위 검사 후 return
	}
	else if(strcmp(inst, "reset") == 0){
		return 7;
	}
	else if(strncmp(inst, "opcode ", 7) == 0){
		len = strlen(inst);
		flag = 0; idx = 0;

		for(i=0;i<len;i++){
			if(inst[i] == ' '){ // 공백 이후 명령어 받음
				flag = 1;
				i++;
			}
			if(flag == 1) mne[idx++] = inst[i];
		}
		return 8;
	}
	else if(strcmp(inst, "opcodelist") == 0){
		return 9;
	}

	return -1; // 위의 경우 모두 만족하지 못하면 유효하지 않은 명령어
}

