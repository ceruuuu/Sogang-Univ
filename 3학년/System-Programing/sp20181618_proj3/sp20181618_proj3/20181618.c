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
	bp* temp1;
	char inst[50];
	char* t_str;

	readfile(); // opcode.txt를 읽고 저장
	mem_reset(); // memory 초기화

	history = (his*)malloc(sizeof(his));
	history->next = NULL; // 명령어 history list의 head 초기화

	l_head = (line*)malloc(sizeof(line));
	l_head->next = NULL;
	l_head->loc = -1;

	bpList = (bp*)malloc(sizeof(bp));
	bpList->next = NULL;

	cur_add = 0; end_flag = 0; cur_i = 0; rPC = 0;

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
				printf("assemble filename\ntype filename\nsymbol\n");
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
						printf("-> [%s,%s,%d] ", temp->mnemonic, temp->op, temp->type);
						temp = temp->next;
					}
					printf("\n");
				}
				break;
			case 10://assemble filename
				symbol = (sym*)malloc(sizeof(sym));
				symbol->next = NULL;
				sym_flag=1;	
	
				if(assemble() == 0){
					printf("Successfully assemble %s.asm\n", filename);
				}
				break;
			case 11://type filename
				type();
				break;
			case 12://symbol
				if(sym_flag==1) sort_symbol();
				else printf("not exist symbol\n");
				break;
			case 13://progaddr
				tempaddr=curaddr; // curaddr는 pass1에서 수정되므로 저장해놓는다.
				break;
			case 14://loader
				l_table = (loader*)malloc(sizeof(loader));
				l_table->next = NULL; 
				tNode = l_table; // 링킹로더 심볼 정보 저장할 list head
				
				curaddr = tempaddr;

				linkEndFlag = 0;
				for(i=0;i<linkFlag;i++){
					linking_pass1(i);
				} // 파일 수 만큼 pass1 진행
					
				if(linkEndFlag==1) break; // 파일이 존재하지 않을 경우 종료한다.

				for(i=0;i<linkFlag;i++){
					linking_pass2(i);
				} // 파일 수 만큼 pass2 진행

				print_estab(); // estable 출력

				prevBP=-1; // 이후 run에 사용될 BP 플래그 초기화

				break;
			case 15: //run
				run();
				break;
			case 16: //bp
				temp1 = bpList;
				if(temp1->next == NULL){
					printf(" Not exist! \n");
				} // bp 리스트에 아무것도 없을 경우
				else{
					printf(" breakpoint \n");
					printf("============\n");
					temp1 = temp1->next;
					while(temp1 != NULL){
						printf(" %s\n", hex(temp1->addr, 4));
						temp1 = temp1->next;
					}
				}
				break;
			case 17: //bp clear
				bpList->next = NULL;
				printf("\t\t[ok] clear all breakpoints\n");			
				break;
			case 18: //bp [addr]
				break;
			default:
				break;
		}
	}

	//동적할당한 메모리들을 free해준다.
	free_history();
	free_opcode();
	free_bp();
	if(sym_flag==1){
		free_symbol();
		free_line();
	}
	return 0;
}

//==========BP list===========
//  입력받은 bp 주소를 리스트에 추가한다.
//==========================
void add_bp(int n){

	bp* node = (bp*)malloc(sizeof(bp));
	node->addr = n;
	node->next = NULL;
	bp* temp = bpList;

	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = node;

	printf("\t\t[ok] create breakpoint %s\n", hex(n, 4));

}

//==========register print===========
//  현재 레지스터에 저장된 값 출력
//====================================
void r_print(){
	printf("A : %s  ", hex(rA, 6));
	printf("X : %s\n", hex(rX, 6));
	printf("L : %s ", hex(rL, 6));
	printf("PC : %s\n", hex(rPC, 6));
	printf("B : %s  ", hex(rB, 6));
	printf("S : %s\n", hex(rS, 6));
	printf("T : %s\n\n", hex(rT, 6));

}
//============RUN=============
//  메모리에 로드된 값을 이용해
//  프로그램을 실행시킨다.
//=============================
void run(){
	char temp[10], temp2[10]; // 범용 temp 문자열이다.
	int xbpe[10]; // object code의 xbpe값을 저장한다.
	int st_i, st_j; // 메모리에 접근할 인덱스가 저장된다.
	int tInt=0, key;
	int ta, x, y, a, bpFlag = 0; // 기타 범용 정수형 변수

	bp* bp_temp; // bp list 탐색 포인터

	if(rPC == 0){
		cc=rA=rX=rPC=rB=rS=rT=0;
		rL = 4215;
	} // 프로그램 시작시 레지스터를 초기화한다.

	cntFlag = 1;
	while(1){	
		if(strcmp(hex(rPC, 4), "1077")==0){
			r_print();
			printf("\t\t End Program\n");
			rL = 4215;
			rPC = 0;
			break;
		} // 프로그램이 끝일때 종료

		//첫 실행은 bp 체크 안함. 
		if(cntFlag == 0){
			bp_temp = bpList->next;
			while(bp_temp != NULL){
				if(rPC == bp_temp->addr){
					r_print();
					printf("\t\t Stop at checkpoint[%s]\n", hex(rPC,4));
					bpFlag = 1;
					break;
				} // bp에서 정지
				bp_temp = bp_temp->next;
			}

			if(bpFlag==1){
				bpFlag = 0;
				break;
			}
		}
		cntFlag=0;

		// 배열올라가는 시작 주소 인덱스 계산
		strcpy(temp , hex(rPC, 6));
		st_j = hexTodec("", temp[5], 1);
		temp[5] = '\0';
		st_i = hexTodec(temp, 0, 0); 

		// pc에 해당하는 메모리 접근
		temp[0] = '\0';
		strcat(temp, hex(m[st_i][st_j++],2));
		if(st_j==16){
			st_i +=1; st_j=0;
		}
		strcat(temp, hex(m[st_i][st_j],2));

		// 3 simple 2 indirect 1 immediate
		tInt = hexTodec("", temp[1], 1)%4; 

		// xbpe 값 계산 
		xbpe[3] = (hexTodec("", temp[2], 1))%2;
		xbpe[2] = (hexTodec("", temp[2], 1)/2)%2;
		xbpe[1] = (hexTodec("", temp[2], 1)/4)%2;
		xbpe[0] = (hexTodec("", temp[2], 1)/8)%2;
		xbpe[4] = '\0';

		// 명령어 key 값 계산
		temp2[0] = temp[0]; temp2[1] = temp[1];
		temp2[2] = '\0';
		key = hexTodec(temp2, 0, 0)-tInt;

		//tInt로 ni, xbpe[]로 xbpe 값들 넘겨준다.
		switch(key){
			case 0: // LDA
				ta = m_value(tInt, xbpe, st_i, st_j, 0, 0);
				rA = ta;
				break;
			case 104://LDB
				ta = m_value(tInt, xbpe, st_i, st_j, 0, 0);
				rB = ta;
				break;
			case 80://LDCH
				ta = m_value(tInt, xbpe, st_i, st_j, 2, 0);	

				// 접근할 메모리 인덱스 계산
				temp[0]='\0';
				strcpy(temp, hex(ta, 6));
				y = hexTodec("", temp[5], 1);
				temp[5] = '\0';
				x = hexTodec(temp, 0, 0); 

				// A 끝바이트에 값 저장
				temp2[0] = '\0';
				strcpy(temp2, hex(rA, 6));
				temp2[4] = hex(m[x][y],2)[0];
				temp2[5] = hex(m[x][y],2)[1];
				temp2[6] = '\0';
				rA = hexTodec(temp2, 0, 0);
				break;
			case 116://LDT
				ta = m_value(tInt, xbpe, st_i, st_j, 0, 0);
				rT = ta;
				break;
			case 12://STA 
				ta = m_value(tInt, xbpe, st_i, st_j, 1, rA);
				break;
			case 84://STCH
				ta = m_value(tInt, xbpe, st_i, st_j, 2, rA);

				// 메모리 접근할 인덱스 계산
				temp[0]='\0';
				strcpy(temp, hex(ta, 6));
				y = hexTodec("", temp[5], 1);
				temp[5] = '\0';
				x = hexTodec(temp, 0, 0); 

				// A의 끝 바이트의 값 추출 및 저장
				temp[0]='\0';
				strcat(temp, hex(rA, 6));
				temp[0] = temp[4];
				temp[1] = temp[5];
				temp[2] = '\0';

				m[x][y] = hexTodec(temp, 0, 0);
				break;
			case 20://STL
				ta = m_value(tInt, xbpe, st_i, st_j, 1, rL);
				break;
			case 16://STX
				ta = m_value(tInt, xbpe, st_i, st_j, 1, rX);
				break;
			case 40://COMP
				ta = m_value(tInt, xbpe, st_i, st_j, 0, 0);
				if(rA>ta) cc=1;
				else if(rA==ta) cc=0;
				else cc=-1;

				break;
			case 160://COMPR 2형식
				cc = 0;
				rPC += 2;
				break;
			case 48://JEQ
				if(cc==0) rPC = m_value(tInt, xbpe, st_i, st_j, 2, 0);
				else rPC += 3;
				break;
			case 60://J
				rPC = m_value(tInt, xbpe, st_i, st_j, 2, 0);
				break;
			case 56://JLT
				if(cc<0) rPC = m_value(tInt, xbpe, st_i, st_j, 2, 0);
				else rPC += 3;
				break;
			case 72://JSUB
				rL = rPC+4;
				rPC = m_value(tInt, xbpe, st_i, st_j, 2, 0);
				break;
			case 76://RSUB
				rPC = rL;
				break;
			case 180://CLEAR 2형식
				rPC += 2;
				a = hexTodec("", temp[2], 1);

				if(a==0) rA = 0;
				else if(a==1) rX = 0;
				else if(a==4) rS = 0;
				break;
			case 224://TD
				rPC += 3;
				cc = -1;
				break;
			case 216://RD
				rPC += 3;
				cc = 0;
				break;
			case 220://WD
				rPC += 3; 
				break;
			case 184://TIXR 2형식
				rPC += 2;
				rX += 1;
				if(rX>rT) cc=1;
				else if(rX==rT) cc=0;
				else cc=-1;
				break;
			default:
				break;
		}	
	}
}

//============메모리 접근===============
//  object 코드를 분석하여 TA의 값을 구해
//  반환하거나 접근하여 수정한다.
//=====================================
int m_value(int tInt, int xbpe[10], int x, int y, int type, int value){

	char temp[10], temp2[10];
	int ta;

	// dis 또는 addr를 3 half byte만큼 추출한다.
	temp[0] = hex(m[x][y],2)[1];
	temp[1] = '\0';
	y++;
	if(y==16){
		x++; y=0;
	}
	strcat(temp, hex(m[x][y], 2));

	if(xbpe[3]==0){ //3형식
		rPC += 3;

		ta = hexTodec(temp, 0, 0);

		if(temp[0] == 'F'){
			ta *= -1;
			ta = hexTodec(two_comp(ta, 3), 0, 0)*(-1);
		} // 보수화

		// x, b, p 설정
		if(xbpe[0]==1) ta += rX;
		if(xbpe[1]==1) ta += rB;
		else if(xbpe[2]==1) ta+= rPC;

		if(tInt==2){ //indirect

			// 메모리 인덱스 계산 및 계산
			temp[0]='\0';
			strcpy(temp, hex(ta, 6));
			y = hexTodec("", temp[5], 1);
			temp[5] = '\0';
			x = hexTodec(temp, 0, 0); 

			temp[0] = '\0';
			strcat(temp, hex(m[x][y], 2));
			y++;
			if(y==16){
				x++; y=0;
			}
			strcat(temp, hex(m[x][y], 2));
			y++;
			if(y==16){
				x++; y=0;
			}
			strcat(temp, hex(m[x][y], 2));

			ta = hexTodec(temp, 0, 0);
		}
	}
	else{ //4형식
		rPC += 4;

		y++;
		if(y==16){
			x++; y=0;
		}
		strcat(temp, hex(m[x][y], 2));
		ta = hexTodec(temp, 0, 0);

		if(xbpe[0]==1) ta += rX;
	}

	if(type == 1){ //메모리에 값 저장한다
		temp2[0] = '\0';
		strcpy(temp2, hex(value, 6));

		temp[0]='\0';
		strcat(temp, hex(ta, 6));
		y = hexTodec("", temp[5], 1);
		temp[5] = '\0';
		x = hexTodec(temp, 0, 0); 

		for(int i=0;i<3;i++){
			temp[0] = temp2[i*2];
			temp[1] = temp2[i*2+1];
			temp[2] = '\0';
			m[x][y] = hexTodec(temp, 0, 0);
			y++;
			if(y==16){
				x++; y=0;
			}
		} // 메모리 접근해서 값 저장
		return ta;
	}
	else if(type == 2) return ta; // 메모리의 주소를 반환한다.

	if(tInt == 1) return ta; // immdiate의 경우 TA의 값을 그대로 반환한다.

	temp[0]='\0';
	strcpy(temp, hex(ta, 6));
	y = hexTodec("", temp[5], 1);
	temp[5] = '\0';
	x = hexTodec(temp, 0, 0); 

	temp[0] = '\0';
	strcat(temp, hex(m[x][y], 2));
	y++;
	if(y==16){
		x++; y=0;
	}
	strcat(temp, hex(m[x][y], 2));
	y++;
	if(y==16){
		x++; y=0;
	}
	strcat(temp, hex(m[x][y], 2));

	return hexTodec(temp, 0, 0); //접근할 메모리에 있는 3바이트 값 반환

}

//============로드맵 출력===============
//  linking & loading이 완료된 로드맵을 출력한다.
//=====================================
void print_estab(){
	int total = 0;

	tNode = l_table->next;

	printf("control symbol address length\n");
	printf("section name  \n");
	printf("-----------------------------------\n");
	while(tNode != NULL){
		if(tNode->type != 0){
			printf("%s\t\t%s\t%s\n", tNode->name, hex(tNode->addr,4), hex(tNode->length, 4));
			total += tNode->length;
		}// H
		else{
			printf("\t%s\t%s\n", tNode->name, hex(tNode->addr, 4));
		}// D

		tNode = tNode->next;
	}
	printf("-----------------------------------\n");
	printf("\t\t total length %s\n", hex(total, 4));


}

//============pass 1===============
//  H, D record를 분석하여 estable에 추가한다.
//=====================================
void linking_pass1(int n){
	FILE* f = fopen(file_arr[n], "r");
	if(f == NULL){
		printf(" file dosen't exist!\n");
		linkEndFlag=1;
		return ;
	}

	char buf[500];
	char temp[10]; // 범용 문자열
	int i,j, idx; // 인덱싱 변수
	int tAddr = curaddr;
	while(!feof(f)){
		fgets(buf, sizeof(buf), f);

		if(buf[0] == 'H'){ // Header recode
			i=1; idx=0;
			loader* node = (loader*)malloc(sizeof(loader));
			// 이름 저장
			while(buf[i]!=' '){
				node->name[idx++] = buf[i++];
			}
			node->name[idx] = '\0';

			while(buf[i] == ' ') i++;
			// 주소 저장
			for(j=0;j<6;j++){
				temp[j] = buf[i++];
			}
			node->addr = hexTodec(temp, 0, 0) + curaddr;
			// 길이 저장
			for(j=0;j<6;j++){
				temp[j] = buf[i++];
			}
			node->length = hexTodec(temp, 0, 0);
			tAddr += node->length;

			node->type = n+1;

			tNode->next = node;
			tNode = tNode->next;
		}
		else if(buf[0] == 'D'){ // Define recode
			i=1; idx=0;
			while(buf[i] != '\n'){
				loader* node = (loader*)malloc(sizeof(loader));
				// 이름 저장
				while(buf[i] != ' '){
					node->name[idx++] = buf[i++];
				}
				node->name[idx] = '\0';

				while(buf[i] == ' ') i++;
				// 주소 저장
				for(j=0;j<6;j++){
					temp[j] = buf[i++];			
				}
				temp[6] = '\0';
				node->addr = hexTodec(temp, 0, 0) + curaddr;

				node->type = 0;
				idx=0;

				tNode->next = node;
				tNode = tNode->next;
			}

			curaddr = tAddr;
		}
		else continue;
	}
}

//============pass 2===============
//  T record 값을 메모리에 올린 뒤
//  R record의 심볼 정보를 바탕으로
//  M record의 요구사항을 수행한다.
//=====================================
void linking_pass2(int n){
	FILE* f = fopen(file_arr[n], "r");
	if(f == NULL){
		printf(" file dosen't exist!\n");
		return ;
	}

	char buf[500];
	char temp[10];
	int ref[10] = {0,}; // reference 변수의 주소 값 저장됨

	tNode = l_table;
	while(tNode!=NULL){
		if(tNode->type == n+1){
			ref[1] = tNode->addr;
			break;
		}
		tNode = tNode->next;
	}

	int i, idx, ref_idx;



	int rFlag=0; // 공백 구분 플래그
	while(!feof(f)){
		fgets(buf, sizeof(buf), f);

		if(buf[0] == 'R'){ //Reference record
			i=1; idx=0;
			while(rFlag == 0){
				// 참조 변수의 번호 저장
				temp[0] = buf[i++];
				temp[1] = buf[i++];
				temp[2] = '\0';
				ref_idx = hexTodec(temp, 0, 0);

				idx=0;
				while(buf[i] != ' '){	
					temp[idx++] = buf[i++];
					if(buf[i] == '\n'){
						rFlag=1;
						break;
					}
				}
				temp[idx] = '\0';

				while(buf[i] == ' ') i++;

				// 참조 변수에 해당하는 주소값을 저장한다.
				tNode = l_table->next;
				while(tNode != NULL){
					if(strcmp(tNode->name, temp) == 0){
						ref[ref_idx] = tNode->addr;
					}
					tNode = tNode->next;
				}
			}
		}
		else if(buf[0] == 'T'){ // Text record
			int t_addr, len;
			int st_i, st_j;

			for(i=0;i<6;i++){
				temp[i] = buf[i+1];
			}
			temp[i] = '\0';	
			t_addr = hexTodec(temp, 0, 0) + ref[1]; 

			for(i=0;i<4;i++){
				temp[i] = hex(t_addr, 5)[i];
			}
			temp[i] = '\0';

			// 접근할 메모리의 인덱스 계산
			st_i = hexTodec(temp, 0, 0);
			st_j = hexTodec("", hex(t_addr, 5)[4], 1);

			temp[0] = buf[7];
			temp[1] = buf[8];
			temp[2] = '\0';

			len = hexTodec(temp, 0, 0); //한번에 올라갈 길이

			// 메모리에 값 저장
			for(i=9;i<len*2+9;i++){
				temp[0] = buf[i++];
				temp[1] = buf[i];
				temp[2] = '\0';

				m[st_i][st_j] = hexTodec(temp, 0, 0);

				st_j++;
				if(st_j == 16){
					st_j = 0;
					st_i++;
				}
			}

		}
		else if(buf[0] == 'M'){ // Modification record
			int t_addr;
			int st_i, st_j;
			int m_i, m_j; // 메모리 인덱스 변수

			for(i=0;i<6;i++){
				temp[i] = buf[i+1];
			}
			temp[i] = '\0';	
			t_addr = hexTodec(temp, 0, 0) + ref[1];

			for(i=0;i<4;i++){
				temp[i] = hex(t_addr, 5)[i];
			}
			temp[i] = '\0';
			// 접근할 메모리의 인덱스 계산
			m_i = st_i = hexTodec(temp, 0, 0);
			m_j = st_j = hexTodec("", hex(t_addr, 5)[4], 1);

			idx=0; temp[0] = '\0';
			for(i=0;i<3;i++){	
				strcat(temp, hex(m[m_i][m_j], 2));

				m_j++;
				if(m_j == 16){
					m_j = 0;
					m_i++;
				}
			}

			t_addr = hexTodec(temp, 0, 0);

			if(strncmp(temp, "FF", 2)==0){
				t_addr = hexTodec(two_comp((-1)*hexTodec(temp,0,0), 6), 0, 0);
				t_addr *= -1;
			} // 보수화

			// 접근할 변수 인덱스 추출
			temp[0] = buf[10];
			temp[1] = buf[11];
			temp[2] = '\0';

			// modification 값 조정
			if(buf[9] == '+'){
				t_addr += ref[hexTodec(temp,0,0)];
			}
			else if(buf[9] == '-'){
				t_addr -= ref[hexTodec(temp,0,0)];
			}

			if(t_addr<0){
				t_addr = hexTodec(two_comp(t_addr, 6), 0, 0);
			} // 값이 음수일경우 보수화해서 저장해야한다.

			idx=0; 
			for(i=0;i<3;i++){	
				temp[0] = hex(t_addr, 6)[i*2];
				temp[1] = hex(t_addr, 6)[i*2+1];
				temp[2] = '\0';

				m[st_i][st_j] = hexTodec(temp,0,0);

				st_j++;
				if(st_j == 16){
					st_j = 0;
					st_i++;
				}
			} // 바꾼 값 갱신
		}
		else continue;
	}
}


//==========PASS1===========
//  pass1을 구현한 함수이다.
//  라인별로 loc를 계산하여 저장한다.
//==========================
int pass1(){
	FILE* f = fopen(filename, "r");
	if(f == NULL){
		printf(" file dosen't exist!\n");
		return -1;
	}

	char buf[50], str1[10], str2[10], str3[10], str4[10];
	char* ptr;
	int cnt = 1;
	int result = 0;
	int debugCnt=0, start_loc;

	error_flag=0;
	cur_loc = 0;
	fLoc = 0;
	base = 0;
	fCnt = 0;

	line* temp = l_head;
	while(!feof(f)){	
		fgets(buf, sizeof(buf), f);
		debugCnt++;

		if(buf[0] == '\n') continue; // 빈 줄

		line* node = (line*)malloc(sizeof(line));
		node->l_num = cnt++;
		node->modi = 0;
		node->next = NULL; // 해당 줄에 대한 노드 생성


		if(buf[0] == '.'){
			node->loc = -2;
			temp->next = node;
			temp = temp->next;
			continue;
		}

		if(buf[0] == ' '){ // symbol 없는 줄
			////////////////////////////////////line 토큰 나누기
			buf[strlen(buf)-1] = '\0';
			str1[0] = '\0';

			ptr = strtok(buf, " ");
			strcpy(str2, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL){
				strcpy(str3, ptr);
			}

			if(str3[strlen(str3)-1] == ','){
				str3[strlen(str3)-1] = '\0';
				ptr = strtok(NULL, " ");
				strcpy(str4, ptr);
			}

			////////////////////////////////////line 정보 저장
			if(op_check(str2)==5 || op_check(str2)==6) 
				node->loc = -1; // BASE, END의 경우 loc x
			else
				node->loc = cur_loc;

			result = loc_func(str2, str3); 
			if(result>=0){
				cur_loc += result; // loc 누적 갱신
			}

			if(op_check(str2)==1 || op_check(str2)==2){
				node->fmat = 0; // BYTE, WORD는 포맷 없음
			}
			else if(op_check(str2)>=3){
				if(op_check(str2)==6) node->modi=-1;
				node->fmat = -1; 
				// BASE, START, END, RESB, RESW는 object code 없음
			}
			else{
				node->fmat = result;
			}

			temp->next = node;
			temp = temp->next;
		}
		else{
			////////////////////////////////////line 토큰 나누기
			buf[strlen(buf)-1] = '\0';
			ptr = strtok(buf, " ");
			strcpy(str1, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL){
				strcpy(str2, ptr);
			}

			ptr = strtok(NULL, " ");
			if(ptr!=NULL){
				strcpy(str3, ptr);
			}

			if(str3[strlen(str3)-1] == ','){
				str3[strlen(str3)-1] = '\0';
				ptr = strtok(NULL, " ");
				strcpy(str4, ptr);
			}

			if(op_check(str2)==7){ // 시작주소 초기화
				cur_loc = hexTodec(str3, 0, 0);
				start_loc = cur_loc;
			} 

			if(op_check(str2)==5 || op_check(str2)==6)
				node->loc = -1; // BYTE, WORD는 포맷 없음
			else
				node->loc = cur_loc;

			result = loc_func(str2, str3);
			if(result>=0){
				cur_loc += result; // loc 누적 갱신
				if(add_symbol(str1, cur_loc-result)==-1){
					error_flag = 1; // symbol 추가 및 중복 체크
				}
			}

			if(op_check(str2)==1 || op_check(str2)==2){
				node->fmat = 0;// BYTE, WORD는 포맷 없음
			}
			else if(op_check(str2)>=3){
				node->fmat = -1;	
				// BASE, START, END, RESB, RESW는 object code 없음
			}
			else{
				node->fmat = result;
			}

			temp->next = node;
			temp = temp->next;
		}

		if(error_flag==1){
			printf("[pass1] error in line %d\n", (debugCnt)*5);
			break;
		} // error 메세지 출력
	}

	fLoc = cur_loc-start_loc; // 파일의 총 길이
	fCnt = cnt-1; // 파일의 끝 줄

	fclose(f);
	if(error_flag==1) return-1; 
	return 0;

}

//==============PASS2===============
//  pass2을 구현한 함수이다.
//  라인별로 object code를 생성하여 저장한다.
//===================================
void pass2(){
	FILE* f = fopen(filename, "r");

	char buf[50], temp[20], str1[10], str2[10], str3[10], str4[10];
	char* ptr;
	char op[3], nixbpe[7], disp[4], addr[6];
	int pc = 0;
	int i, len, flag, b_flag, offset, debugCnt=0, t = fCnt;

	line* cur = l_head; // 현재 라인 정보
	line* ne = NULL; // 다음 라인 정보
	while(!feof(f)){
		fgets(buf, sizeof(buf), f);
		debugCnt++;

		if(buf[0] == '\n') continue; // 빈 줄
		if(buf[0] == '.'){
			fCnt--;
			cur = cur->next;
			continue; // 주석
		}
		cur = cur->next;

		fCnt--;
		if(fCnt>1){
			ne = cur->next;
			while(ne->loc<0){
				ne = ne->next;
			}
			pc = ne->loc;
		} // 현재 줄 기준 program counter 지정
		else{
			pc = 0;
		} // 마지막 줄의 program counter는 0

		if(fCnt==0){
			cur->loc = -1;
			strcpy(cur->obj,"");
			continue;
		}
		////////////////////////////////////line 토큰 나누기
		for(int i=0;i<10;i++){
			str1[i]='\0'; str2[i]='\0';
			str3[i]='\0'; str4[i]='\0';
		}

		if(buf[0] == ' '){ // symbol 없는 줄
			buf[strlen(buf)-1] = '\0';
			str1[0] = '\0';

			ptr = strtok(buf, " ");
			strcpy(str2, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL) strcpy(str3, ptr);	

			if(str3[strlen(str3)-1] == ','){
				str3[strlen(str3)-1] = '\0';
				ptr = strtok(NULL, " ");
				strcpy(str4, ptr);
			}	
		}
		else{ // symbol 있는 줄
			buf[strlen(buf)-1] = '\0';
			ptr = strtok(buf, " ");
			strcpy(str1, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL) strcpy(str2, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL) strcpy(str3, ptr);

			if(str3[strlen(str3)-1] == ','){
				str3[strlen(str3)-1] = '\0';
				ptr = strtok(NULL, " ");
				strcpy(str4, ptr);
			}
		}
		////////////////////////////////////obj 코드 생성
		obj_reset(op, nixbpe, disp, addr);
		flag = 0; 
		b_flag = 0; // base 플래그

		if(strcmp(str2, "BASE")==0){
			base = search_symbol(str3);	
			continue;
		} // BASE의 기준 심볼 loc값 저장
		else if(cur->loc<0) continue; // object code 정보 x

		if(cur->fmat == 0){ // BYTE, WORD
			len = strlen(str3);
			if(strcmp(str2, "BYTE")==0){
				if(str3[0] == 'C'){ // 캐릭터 문자
					i=2;
					while(i<len-1){
						str3[i-2] = str3[i];
						i++;
					}	
					str3[i-2] = '\0';

					strcpy(cur->obj, ascii_str(str3)); 
					// 해당 문자의 16진수 아스키코드 값이 그대로 저장한다.
				}
				else{ // 16진수
					i=2;
					while(i<len-1){
						cur->obj[i-2] = str3[i];
						i++;
					} // 16진수 그대로 저장한다.
				}
			}
			else{ // WORD
				strcpy(cur->obj, hex(atoi(str3), 6));
			} // 10진수를 16진수로 바꾸어 저장한다.
		}
		else if(cur->fmat == 1){
			strcpy(cur->obj, hash_function(str2));
		} // 포맷1. opcode를 저장한다.
		else if(cur->fmat == 2){
			strcpy(cur->obj, hash_function(str2));

			cur->obj[2] = decTohec(regi(str3));
			cur->obj[3] = decTohec(regi(str4));
		} // 포맷2. opcode와 register를 저장한다.
		else if(cur->fmat>=3){
			strcpy(temp, "");

			ptr = strtok(str2, "+");
			strcpy(str2, ptr); // 포맷4의 '+' 제거

			strcpy(cur->obj, hash_function(str2));
			if(cur->obj[1]=='0'){
				op[0]='0'; op[1]='0';
			}
			else if(cur->obj[1]=='4'){
				op[1]='1';
			}
			else if(cur->obj[1]=='8'){
				op[0]='1';
			}
			else if(cur->obj[1]=='C'){
				op[0]='1'; op[1]='1';
			}
			op[2] = '\0';
			cur->obj[1]='\0';
			// opcode(6bit) 정보 저장

			if(str3[0] == '#'){ // immediate addressing
				nixbpe[1] = '1'; // i=1

				ptr = strtok(str3, "#");
				strcpy(str3, ptr);

				if(str3[0]>='0' && str3[0]<='9'){
					flag = 1;
				} // [#숫자]와 같이 숫자가 그대로 저장
			}
			else if(str3[0]== '@'){ // indirect addressing
				nixbpe[0] = '1'; // n=1

				ptr = strtok(str3, "@");
				strcpy(str3, ptr);
			}
			else{ // simple addressing
				nixbpe[0] = '1';
				nixbpe[1] = '1';
			}

			if(strcmp(str4, "X")==0) nixbpe[2] = '1'; // x=1
			if(cur->fmat == 4) nixbpe[5] = '1'; // e=1

			if(cur->fmat == 3){ // displace 및 address 
				if(flag==1){		
					strcpy(disp, hex(atoi(str3), 3));
				} // [#10진수숫자]는 16진수로 변환하여 그대로 저장
				else{
					offset = search_symbol(str3) - pc;	
					// 현재 operand의 loc와 pc의 차이값

					if(offset>=-2048 && offset<=2047){ // pc
						if(offset>=0){ // 양수
							strcpy(disp, hex(offset, 3));
						}
						else{ // 음수. 보수화 단계를 거친다.
							strcpy(disp, two_comp(offset, 3));
						}
					}
					else{ // base
						b_flag=1;
						offset  = search_symbol(str3) - base;
						// 현재 operand의 loc와 base loc의 차이값

						if(strcmp(str3, "")==0) {
							strcpy(disp, "000");
							flag=1;
						} // operand가 없을 경우
						else strcpy(disp, hex(offset, 3));
					}
				}		
			}
			else if(cur->fmat == 4){ // 포맷4의 경우 operand의 loc값을 바로 저장
				if(flag==1){ // [#숫자]
					strcpy(addr, hex(atoi(str3), 5));
				}
				else{
					strcpy(addr, hex(search_symbol(str3),5));
					if(strcmp(str3, "")==0) strcpy(addr, "00000");
				}
			}

			if(flag != 1 && cur->fmat == 4){
				cur->modi = 1;
			} // modification의 경우

			if(flag == 1 || cur->fmat == 4){
				nixbpe[3] = '0'; // b=0
				nixbpe[4] = '0'; // p=0
			}
			else if(b_flag==0){ 
				nixbpe[4] = '1'; // p=1
			}
			else{ 
				nixbpe[3] = '1'; // b=1
			}

			strcpy(temp, op);
			strcat(temp, nixbpe);
			strcat(cur->obj, biTohex(temp, 2));
			// [op + nixbpe] 12bit 저장

			if(cur->fmat==3){
				strcat(cur->obj, disp);
			}
			else{
				strcat(cur->obj, addr);
			}
			// [displace] 12bit or [address] 20bit 저장 
		}
		else{
			strcpy(cur->obj, "");
		} // object code 없는 경우

		if(error_flag  == 1){
			printf("[pass2] error in line %d\n", (debugCnt)*5);
			break;
		}
	}

	fCnt=t;
	fclose(f);
}
//=======Assemble 함수=========
//  pass1, pass2를 통해 얻은 정보로
//  .lst , .obj 파일 생성
//===========================
int assemble(){
	FILE* fr;
	FILE* fw;
	char t_filename[50];
	char code_name[10];
	char* ptr;

	if(pass1()==0) pass2();

	if(error_flag==1) return -1;

	fr = fopen(filename, "r");

	ptr = strtok(filename, ".");
	strcpy(filename, ptr);

	//************************************lst파일 생성
	sprintf(t_filename, "%s.lst", filename);
	fw = fopen(t_filename, "w");

	char buf[50], str1[10], str2[10], str3[10];

	line* cur = l_head;	
	while(!feof(fr)){
		fgets(buf, sizeof(buf), fr);

		if(buf[0] == '\n') continue; // 빈 줄
		if(buf[0] == '.'){
			fprintf(fw, "%-5d", (cur->l_num)*5);
			fprintf(fw, "%s", buf);
			cur = cur->next;
			continue; // 주석
		}
		cur = cur->next;

		////////////////////////////////////line 토큰 나누기
		for(int i=0;i<10;i++){
			str1[i]='\0'; str2[i]='\0';
			str3[i]='\0'; 
		}

		if(buf[0] == ' '){ // symbol 없는 줄
			buf[strlen(buf)-1] = '\0';
			str1[0] = '\0';

			ptr = strtok(buf, " ");
			strcpy(str2, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL) strcpy(str3, ptr);	

			if(str3[strlen(str3)-1] == ','){
				str3[strlen(str3)] = ' ';
				str3[strlen(str3)+1] = '\0';
				ptr = strtok(NULL, " ");
				strcat(str3, ptr);
			}
		}
		else{ // symbol 있는 줄
			buf[strlen(buf)-1] = '\0';
			ptr = strtok(buf, " ");
			strcpy(str1, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL) strcpy(str2, ptr);

			ptr = strtok(NULL, " ");
			if(ptr!=NULL) strcpy(str3, ptr);	

			if(str3[strlen(str3)-1] == ','){
				str3[strlen(str3)] = ' ';
				str3[strlen(str3)+1] = '\0';
				ptr = strtok(NULL, " ");
				strcat(str3, ptr);
			}

			if(op_check(str2)==7) strcpy(code_name, str1);
			// code 이름 저장
		}

		////////////////////////////////////출력
		fprintf(fw, "%-5d", (cur->l_num)*5);
		fprintf(fw, "%s  ", hex(cur->loc, 4));
		fprintf(fw, "%-10s", str1);
		fprintf(fw, "%-10s", str2);
		fprintf(fw, "%-15s", str3);
		fprintf(fw, "%-15s\n", cur->obj);
	}
	fclose(fw);
	fclose(fr);

	//******************************************obj 파일 생성
	sprintf(t_filename, "%s.obj", filename);
	fw = fopen(t_filename, "w");

	int first=0, len;
	char obj_buf[200];
	cur = l_head->next;

	fprintf(fw, "H%-6s%s%s\n", code_name, hex(cur->loc,6), hex(fLoc, 6));

	cur=cur->next;
	while(cur->next != NULL){
		strcpy(obj_buf, "");
		len=0;

		if(cur->modi == -1) break; // END의 경우 종료한다
		else if(strcmp(cur->obj, "") == 0){
			cur = cur->next;
			continue;
		} // 주석

		fprintf(fw, "T%-6s", hex(cur->loc, 6));
		first = cur->loc;

		while(1){
			if(cur->modi==-1) break; // END

			if(cur->loc<0){
				cur = cur->next;
				continue;
			} // 주석 및 loc가 배정되지 않은 줄
			if(strcmp(cur->obj, "") == 0){
				break;
			}

			len = cur->loc-first;
			strcat(obj_buf, cur->obj);
			cur = cur->next;
			if(len>25) break; // 한 줄에 25 ~ 30byte 출력
		}
		len = cur->loc - first;
		if(cur->modi==-1){
			len = fLoc - first;
		}

		fprintf(fw, "%s%s\n", hex(len, 2), obj_buf);
	}

	cur = l_head->next;
	while(cur!=NULL){
		if(cur->modi == 1){
			fprintf(fw, "M%s05\n", hex(cur->loc+1, 6));
		}
		cur = cur->next;
	} // modification 출력

	fprintf(fw, "E%s\n",hex((l_head->next)->loc, 6)); 
	fclose(fw);
	return 0;
}

//=========type 함수=========
// 파일이 존재하는지 확인한 후, 출력
//=========================
void type(){
	FILE* f = NULL;
	char buf;
	f = fopen(filename, "r");
	if(f == NULL) printf(" file doesn't exist\n"); //파일이 존재하지 않을경우 error
	else{
		while(fscanf(f, "%c", &buf) != EOF) printf("%c",buf);
		printf("\n");
	}
}

//=========symbol 추가 함수=========
//	  리스트에 symbol을 추가한다.
//	  symbol이 중복될경우 -1을 반환한다.
//===============================
int add_symbol(char* str, int num){
	sym* temp = symbol;
	sym* node = (sym*)malloc(sizeof(sym));
	strcpy(node->name, str);
	node->loc = num;
	node->next = NULL;

	while(temp->next != NULL){
		temp = temp->next;
		if(strcmp(temp->name, node->name)==0) return -1; // 중복되는 경우
	}
	temp->next = node;
	return 0; 
}

//======symbol 정렬 뒤 출력======
//  symbol 테이블을 내림차순으로 출력.
//===========================
void sort_symbol(){
	char** temp;
	int* tLoc;
	int* idx;
	sym* cur = symbol;
	int i,j,t, cnt=0;

	while(cur->next!=NULL){
		cur=cur->next;
		cnt++;
	}// 전체 심볼 수 체크
	temp = (char**)malloc(sizeof(char*)*cnt);
	tLoc = (int*)malloc(sizeof(char*)*cnt);
	idx = (int*)malloc(sizeof(int)*cnt);
	for(i=0;i<cnt;i++){
		temp[i] = (char*)malloc(sizeof(char)*20);
		idx[i]=i;
	} // 동적할당

	cur = symbol;
	i=0;
	while(cur->next!=NULL){
		cur=cur->next;	
		strcpy(temp[i], cur->name);
		tLoc[i] = cur->loc;	
		i++;
	}// 심볼 문자열 카피

	for(i=0;i<cnt;i++){
		for(j=i+1;j<cnt;j++){
			if(strcmp(temp[idx[i]], temp[idx[j]])<=0){
				t=idx[i]; idx[i]=idx[j]; idx[j]=t;
			}
		}
	} // 문자열과 인덱스 배열을 맵핑하여 문자열의 크기를 비교한다.

	for(i=0;i<cnt;i++){
		printf("\t%s\t%s\n", temp[idx[i]], hex(tLoc[idx[i]], 4));
	}
	free(temp);
	free(idx);
	free(tLoc); //동적할당한 배열 free
}

//============symbol->loc===========
//	symbol과 대응되는 주소를 리스트에서 찾아 반환한다
//  존재하지 않을 경우 -1을 반환한다.
//==================================
int search_symbol(char* str){
	sym* temp = symbol->next;

	if(strcmp(str, "")==0) return 0;
	while(temp != NULL){
		if(strcmp(temp->name, str)==0) return temp->loc;
		temp = temp->next;
	}
	error_flag=1;
	return -1;
}

//=========2's compliment========
//  음수인 10진수 offset과 길이를 받아서
//  보수화한 뒤 16진수 문자열로 반환한다.
//===============================
char* two_comp(int n, int b){
	char str[4];
	char buf[13];
	int i, idx,temp = 0;
	n *= -1;

	strcpy(str,hex(n, b));

	idx = 0;
	for(i=0;i<b;i++){
		temp = hexTodec("", str[i], 1);
		buf[idx++] = ((temp/8)+1)%2 +'0';
		temp -= (temp/8)*8;
		buf[idx++] = ((temp/4)+1)%2 +'0';
		temp -= (temp/4)*4;
		buf[idx++] = ((temp/2)+1)%2 +'0';
		temp -= (temp/2)*2;
		buf[idx++] = (temp+1)%2+'0';
	} // 2진수

	buf[b*4-1] += 1;
	for(i=b*4-1;i>=0;i--){
		if(buf[i]>'1'){
			buf[i] -= 2;
			buf[i-1] += 1;
		}
	} // 보수화
	return biTohex(buf, b);
}

//=====2진수 문자열->16진수 문자열=======
//  2진수 문자열을 16진수 문자열 길이를 받아
//  16진수 문자열로 반환한다. 
//===============================
char* biTohex(char* buf, int b){
	char* str = malloc(sizeof(char)*4);
	int i, idx=0;
	int n;

	for(i=0;i<b;i++){
		n = (buf[idx++]-'0')*8;
		n += (buf[idx++]-'0')*4;
		n += (buf[idx++]-'0')*2;
		n += (buf[idx++]-'0');
		str[i] = decTohec(n);
	}
	str[i] = '\0';
	return str;
}

//=========아스키코드 문자열=========
//  문자열을 받아와 문자 하나하나에 대응하는
//  아스키코드 값을 다시 문자열로 반환한다.
//=============================
char* ascii_str(char* str){
	int i;
	int temp;
	int len = strlen(str);
	char s[3];
	char* a_str = malloc(sizeof(char)*(len*2+1));
	a_str[0] = '\0';
	for(i=0;i<len;i++){
		temp = str[i];
		strcpy(s, hex(temp, 2));
		strcat(a_str, s);
	}
	a_str[len*2] = '\0';

	return a_str;
}

//===========loc 값 계산 =============
//  해당 라인에 할당된 byte수를 게산하여 반환
//=================================
int loc_func(char* str2, char* str3){
	int i=0;
	int len = strlen(str3);
	int n = op_check(str2);
	char* ptr;
	char tstr[10];

	if(n==1){ // BYTE
		i=2;
		while(i<len-1){ 
			tstr[i-2] = str3[i];
			i++;
		}
		if(str3[0] == 'C'){ // 문자 하나당 1byte
			return strlen(tstr);	
		}
		else if(str3[0] == 'X'){ // 16진수
			return (strlen(tstr)+1)/2;
		}
	}
	else if(n==2){ // WORD
		return 3;
	}
	else if(n==3){ // RESB 
		return atoi(str3);
	}
	else if(n==4){ // RESW
		return atoi(str3)*3;
	}
	else if(n==5 || n==6){ //END, BASE
		return -1; 
	}
	else if(n==7){ // START
		return 0;
	}
	else{
		//str2에 +면 4형식
		//나머진 해쉬검색해서 나오는 값 type만큼 더함
		if(str2[0] == '+'){
			ptr = strtok(str2, "+");
			strcpy(str2, ptr);
			if(strcmp(hash_function(str2), "\0") != 0) return 4;
		}

		int key = (str2[0]-'A')%21;
		hash* temp = h_table[key];

		while(temp != NULL){
			if(strcmp(str2, temp->mnemonic) == 0){
				return temp->type;
			}
			temp = temp->next;
		}
		error_flag=1;
		return -2; // 아무것도 반환되지않으면 없는 instruction이므로 error
	}

	return -2;
}

//=========10진수->16진수===========
//  10진수와 길이를 받아 16진수 문자열 반환
//==============================
char* hex(int n, int b){
	int i, temp, a;
	char* str = malloc(sizeof(char)*(b+1));

	if(n<0){
		for(i=0;i<b;i++){
			str[i] = ' ';
		}
		str[i] = '\0';
		return str;
	}

	a=1;
	for(i=0;i<b-1;i++){
		a *= 16;
	}
	temp = n;
	for(i=0;i<b-1;i++){
		str[i] = decTohec(temp/a);
		temp -= (temp/a)*a;
		a /= 16;
	}
	str[i] = decTohec(temp);
	str[i+1] = '\0';

	return str;
}

//========레지스터 검색========
//  레지스터 테이블에서 번호 반환
//=========================
int regi(char* str){
	int cnt = 0;
	int idx = 0;

	while(cnt<10){
		if(strcmp(reg[idx], str)==0) return cnt;
		if(cnt==6) cnt += 2;
		else cnt += 1;
		idx++;
	}
	if(strcmp(str, "")==0) return 0;

	error_flag = 1;
	return 0; // 저장된 레지스터가 아닐경우
}

//=====opcode table에 없는 명령어 예외처리=======
int op_check(char* str){
	if(strcmp(str, "BYTE")==0) return 1;
	else if(strcmp(str, "WORD")==0) return 2;
	else if(strcmp(str, "RESB")==0) return 3;
	else if(strcmp(str, "RESW")==0) return 4;
	else if(strcmp(str, "BASE")==0) return 5;
	else if(strcmp(str, "END")==0) return 6;
	else if(strcmp(str, "START")==0) return 7;
	else return 0;
}

//========문자열 초기화==========
void obj_reset(char* op, char* nixbpe, char* disp, char* addr){
	int i;
	for(i=0;i<3;i++){
		op[i] = '0';
	}
	op[6] = '\0';
	for(i=0;i<6;i++){
		nixbpe[i] = '0';
	}
	nixbpe[6] = '\0';
	for(i=0;i<3;i++){
		disp[i] = '0';
	}
	disp[3] = '\0';
	for(i=0;i<5;i++){
		addr[i] = '0';
	}
	addr[5] = '\0';
	return ;
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
			if(flag == 1){
				if(buf[i] >= '1' && buf[i] <= '3'){
					node->type = buf[i] - '0';
					break;
				}
			}		
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
// ========symbol list free하는 함수=========
void free_symbol(){	
	sym* temp = symbol;
	sym* prev = NULL;
	while(temp->next != NULL){
		prev = temp;
		temp = temp->next;
		free(prev);
	}
	free(temp);
}

void free_bp(){	

	bp* temp = bpList;
	bp* prev = NULL;
	while(temp->next != NULL){
		prev = temp;
		temp = temp->next;
		free(prev);
	}
	free(temp);

}



// =========line list free하는 함수=========
void free_line(){
	line* temp = l_head;
	line* prev = NULL;

	loader* temp1 = l_table;
	loader* prev1 = NULL;

	while(temp->next != NULL){
		prev = temp;
		temp = temp->next;
		free(prev);

		prev1 = temp1;
		temp1 = temp1->next;
		free(prev1);
	}
	free(temp);
	free(temp1);

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
	for(i=0;i<50;i++) filename[i] = '\0';

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
	else if(strncmp(inst, "assemble ",9) == 0){
		len = strlen(inst);
		idx=0;
		for(i=9;i<len;i++){
			filename[idx++] = inst[i];
		}
		return 10;
	}
	else if(strncmp(inst, "type ", 5) == 0){
		len = strlen(inst);
		idx=0;
		for(i=5;i<len;i++){
			filename[idx++] = inst[i];
		}
		return 11;
	}
	else if(strcmp(inst, "symbol") == 0){
		return 12;
	}
	else if(strncmp(inst, "progaddr ", 9) == 0){
		len = strlen(inst);
		idx=0;
		for(i=9;i<len;i++){
			address[idx++] = inst[i];
		}
		curaddr = hexTodec(address, 0, 0);  // 입력받은 주소 10진수 값으로

		if(curaddr>65535){
			printf(" 0 <= address <= FFFF \n");
			return -1;
		}
		return 13;
	}
	else if(strncmp(inst, "loader ", 7) == 0){
		char* ptr;
		FILE* ff;
		len = strlen(inst);
		linkFlag = 0;

		inst[len] = '\0';
		ptr = strtok(inst, " ");

		// 입력받은 파일의 개수만큼 token화
		ptr = strtok(NULL, " ");
		if(ptr!=NULL){
			strcpy(file_arr[0], ptr);
			linkFlag = 1;
			ff= fopen(file_arr[0], "r");
			if(ff==NULL) return -1;
			fclose(ff);
		}

		ptr = strtok(NULL, " ");
		if(ptr!=NULL){
			strcpy(file_arr[1], ptr);
			linkFlag = 2;
			ff= fopen(file_arr[1], "r");
			if(ff==NULL) return -1;
			fclose(ff);	
		}

		ptr = strtok(NULL, " ");
		if(ptr!=NULL){
			strcpy(file_arr[2], ptr);
			linkFlag = 3;
			ff= fopen(file_arr[2], "r");
			if(ff==NULL) return -1;
			fclose(ff);	
		}

		return 14;
	}
	else if(strcmp(inst, "run") == 0){
		return 15;
	}
	else if(strcmp(inst, "bp") == 0){
		return 16;
	}
	else if(strcmp(inst, "bp clear")==0){
		return 17;	
	}
	else if(strncmp(inst, "bp ", 3)==0){
		len = strlen(inst)-3;

		for(i=0;i<len;i++){
			address[i] = inst[i+3];
		}
		address[i] = '\0';

		add_bp(hexTodec(address, 0, 0)); // 입력받은 주소값 10진수로

		return 18;
	}

	return -1; // 위의 경우 모두 만족하지 못하면 유효하지 않은 명령어
}

