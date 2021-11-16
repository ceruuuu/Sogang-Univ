#ifndef _20181618
#define _20181618

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// --------- opcode 표 정리하기 위한 구조체 ---------
typedef struct hash{
	char mnemonic[10]; 
	char op[10];
	int type;
	struct hash *next;
}hash;
// --------- 명령어 history를 구성하기 위한 구조체 ---------
typedef struct his{
	char inst[100];
	struct his *next;
}his;
//----------- symbol의 정보가 저장되는 구조체 ----------
typedef struct sym{
	char name[15]; // symbol
	int loc;
	struct sym *next;
}sym;
// ---------- line별 정보가 저장되는 구조체 -------------
typedef struct line{
	int l_num, loc, fmat, modi;
	//fmat은 포멧번호, 0이면 byte word, -1이면 기재x, -2면 주석
	char obj[10]; // object code
	struct line *next;
}line;

//----------- 링킹로더 결과 심볼 정보가 저장되는 구조체----------------
typedef struct loader{
	char name[10];
	int addr;
	int length;
	int type;
	struct loader *next;
}loader;

//------------ bp 리스트 구조체-----------------
typedef struct bp{
	int addr;
	struct bp *next;
}bp;

//============로드맵 출력===============
//  linking & loading이 완료된 로드맵을 출력한다.
//=====================================
void print_estab();

//============pass 1===============
//  H, D record를 분석하여 estable에 추가한다.
//=====================================
void linking_pass1(int idx);

//============pass 2===============
//  T record 값을 메모리에 올린 뒤
//  R record의 심볼 정보를 바탕으로
//  M record의 요구사항을 수행한다.
//=====================================
void linking_pass2(int n);

//==========BP list===========
//  입력받은 bp 주소를 리스트에 추가한다.
//==========================
void add_bp(int n);

//============RUN=============
//  메모리에 로드된 값을 이용해
//  프로그램을 실행시킨다.
//=============================
void run();

//============메모리 접근===============
//  object 코드를 분석하여 TA의 값을 구해
//  반환하거나 접근하여 수정한다.
//=====================================
int m_value(int tInt, int xbpe[], int x, int y, int type, int value);

//==========register print===========
//  현재 레지스터에 저장된 값 출력
//====================================
void r_print();

int prevBP; // 직전에 멈춘 BP값이 저장된다.
int curaddr; // progaddr 값 저장
int tempaddr; // 임시로 저장되는 progaddr
int linkFlag; // loader 명령어에 입력된 파일의 개수
int linkEndFlag; // 링킹시에 파일 이름 잘못되었을 때 1이 된다.
int cc; // 부등호 결과 저장
int rA, rX, rL, rPC, rB, rS, rT; // 레지스터
int cntFlag; 

loader *l_table; // 심볼 리스트 head
loader *tNode; // 심볼 임시 노드
bp *bpList; // bp 리스트 head


//-------------

//==========PASS1===========
//  pass1을 구현한 함수이다.
//  라인별로 loc를 계산하여 저장한다.
//==========================
int pass1();

//==============PASS2===============
//  pass2을 구현한 함수이다.
//  라인별로 object code를 생성하여 저장한다.
//===================================
void pass2();

//=======Assemble 함수=========
//  pass1, pass2를 통해 얻은 정보로
//  .lst , .obj 파일 생성
//===========================
int assemble();

//=========type 함수=========
// 파일이 존재하는지 확인한 후, 출력
//=========================
void type();

//=========symbol 추가 함수=========
//	  리스트에 symbol을 추가한다.
//	  symbol이 중복될경우 -1을 반환한다.
//===============================
int add_symbol(char* str, int num);

//============symbol->loc===========
//	symbol과 대응되는 주소를 리스트에서 찾아 반환한다
//  존재하지 않을 경우 -1을 반환한다.
//==================================
int search_symbol(char* str);

//======symbol 정렬 뒤 출력======
//  symbol 테이블을 내림차순으로 출력.
//===========================
void sort_symbol();

//=========2's compliment========
//  음수인 10진수 offset과 길이를 받아서
//  보수화한 뒤 16진수 문자열로 반환한다.
//===============================
char* two_comp(int n, int b);

//=====2진수 문자열->16진수 문자열=======
//  2진수 문자열을 16진수 문자열 길이를 받아
//  16진수 문자열로 반환한다. 
//===============================
char* biTohex(char* buf, int b);

//=========아스키코드 문자열=========
//  문자열을 받아와 문자 하나하나에 대응하는
//  아스키코드 값을 다시 문자열로 반환한다.
//=============================
char* ascii_str(char* str);

//===========loc 값 계산 =============
//  해당 라인에 할당된 byte수를 게산하여 반환
//=================================
int loc_func(char* str1, char* str2);

//=========10진수->16진수===========
//  10진수와 길이를 받아 16진수 문자열 반환
//==============================
char* hex(int n, int b);

//========레지스터 검색========
//  레지스터 테이블에서 번호 반환
//=========================
int regi(char* str);

//=====opcode table에 없는 명령어 예외처리=======
int op_check(char* str);

//========문자열 초기화==========
void obj_reset(char* str1, char* str2, char* str3, char* str4);

// =========opcode.txt 파일 열고 읽는 함수=========
//		파일의 정보를 읽고 리스트에 저장한다. 
//		반환값은 없다.
// =========================================
void readfile();

// =========hash 함수=========
//	key는 명령의 첫번째 알파벳
//	(key-'A')%21의 값으로 인덱싱하여
//	value를 반환한다.
// ==========================
char* hash_function(char* str);

// =========현재 디렉토리의 파일 출력=========
//		현재 디렉토리의 경로를 추출하여
//		그 안의 모든 파일을 출력한다.
// ==================================
void show_directory();

// =========16진수->10진수 변환 함수=========
//	문자열 또는 문자를 받았을 때 10진수로 반환한다.
// ====================================
int hexTodec(char* str, char c, int key);

// =========메모리 reset=========
//	전제 메모리 배열을 0으로 초기화한다.
// ===========================
void mem_reset();

// =========10진수->16진수=========
//	인자로 들어온 값을 16진수 문자로 반환한다.
// =============================
char decTohec(int d);

// =========Dump 실행하는 함수=========
//	사용자가 지정한 범위의 메모리를 출력한다.
//  명령어의 종류에 따른 key값을 인자로 받고
//  결과를 출력한다.
// ===============================
void Dump(int key);

// =========edit 실행하는 함수.=========
//	 입력받은 특정 주소의 값을 수정한다.
// ================================
void edit();

// =========fill 실행하는 함수=========
//	지정된 범위의 메모리 값을 수정한다
// ===============================
void fill();

// =========입력받은 값의 범위를 검사하는 함수==========
//	start, end, addresss와 value의 범위 검사
// ========================================
int boundary_check(char* str, int key);

// =========정상적인 명령어 history에 추가하는 함수=========
void add_history(char *str);

// =========history 출력하는 함수==========
void print_history();

// =========history list free하는 함수=========
void free_history();

// ========= bp list free하는 함수 =============
void free_bp();

// =========opcode list free하는 함수=========
void free_opcode();

// =========symbol list free하는 함수=========
void free_symbol();

// =========line list free하는 함수=========
void free_line();

// =========명령어가 정상적인지 결정하는 함수==========
//	어떤 명령어인지, 입력받은 값은 무엇인지, 그 값은 정상적인지
//  검사한 결과를 반환한다.
// ========================================
int instruction_check(char* inst);

int m[65536][16]; // 1MByte 메모리
char start[10], end[10], address[10], value[10]; // 입력받는 주소 및 value
char mne[10]; // opcode 명령어
int cur_add; // 현재 출력할 address
int end_flag; // 프로그램의 종료를 기록하는 플래그
int error_flag=0;
int cur_i; // 현재 출력할 address의 index

int sym_flag=0; // symbol 성공적으로 저장되면 1
int cur_loc = 0; // 현재 loc값 저장
int fLoc = 0; // 최종 loc값
int base = 0; // base로 지정된 loc
char filename[50];
char file_arr[3][50];
int fCnt = 0; // 파일 끝 번호

char* reg[] = {"A", "X", "L", "B", "S", "T", "F", "PC", "SW"};

hash* h_table[20]; // hash table
his* history; // history를 저장하는 링크드리스트의 head
sym* symbol; // symbol list head
line* l_head; // line list head

#endif
