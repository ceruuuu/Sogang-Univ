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

// =========opcode list free하는 함수=========
void free_opcode();

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
int cur_i; // 현재 출력할 address의 index

hash* h_table[20]; // hash table
his* history; // history를 저장하는 링크드리스트의 head

#endif
