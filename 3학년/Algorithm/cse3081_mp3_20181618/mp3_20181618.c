#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct TreeNode{
	char symbor; //문자
	int frq; //문자의 빈도 수
	struct TreeNode *left; //트리노드의 left child
	struct TreeNode *right; //트리노드의 right child
}TreeNode;

typedef struct{
	TreeNode *TNode;
	int key; //frq의 연산결과 저장
}Node; //TreeNode를 성분으로 heap을 구성하는 노드

typedef struct{
	char symbor;
	char bits[100];
}ResultNode; //문자를 비트로 변환한 결과를 저장하는 구조체

Node heap[129]; //ASCII CODE는 128가지만 들어온다고 가정한다.

int hSize=0;//heap의 크기. 가변
int n=0;//파일에 입력된 문자의 가짓수. 불변

ResultNode list[128];//전체 아스키코드 표와 허프만 코드가 매칭된 구조체

char pCode[200]; //temp기능


//======== min-heap에 노드를 insert하는 function
void Insert(Node temp){ 
	int i = ++hSize; //heap의 크기가 1 커진다

	while(i != 1 && temp.key < heap[i/2].key){
		heap[i] = heap[i/2];
		i /= 2;
	}

	heap[i] = temp;
}

//======== min-heap의 root 노드를 delete하는 function
Node Delete(){
	int parent = 1, child = 2;
	Node delNode, temp;

	delNode = heap[1]; //root를 삭제한다
	temp = heap[hSize]; //맨 마지막 노드로 재배열한다
	hSize--;

	while(child <= hSize){
		if((child < hSize) && (heap[child].key) > (heap[child+1].key))
			child++; //자식노드 중 key값이 작은 쪽을 선택

		if(temp.key <= heap[child].key) break; //자식노드보다 더 작으면 멈춘다

		heap[parent] = heap[child];
		parent = child;
		child *= 2;
	}

	heap[parent] = temp;

	return delNode;	
}

//======== TreeNode를 동적할당하여 tree화 하는 function
TreeNode* makeNode(TreeNode *left, TreeNode *right){
	TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode));
	if(node == NULL){
		printf("Memory NULL!\n");
		exit(-1);
	}

	node->left = left; //파라미터로 자식노드가 될 노드의 
	node->right = right; //주소값을 받아 저장한다

	return node;
}

//======== 문자에 매칭되는 코드를 저장하는 function
void cAscii(char c, char* tCode){
	int i;
	for(i=0;i<128;i++){
		if(list[i].symbor==c) break;
	}
	strcpy(list[i].bits, tCode);
}

//======== 코드에 매칭되는 문자를 반환하는 function
int codeAscii(char* tCode){
	int i;

	for(i=0;i<128;i++){
		if(strcmp(tCode, list[i].bits)==0) break;
	}
	if(i == 128) return -1;
	return i;
}

//======== 완성된 트리로부터 부호화하여 매칭하는 function
void toBitcodes(TreeNode *temp, int i){

	if(temp){
		i++;
		pCode[i] = '0';
		toBitcodes(temp->left, i);
		pCode[i] = '1';
		toBitcodes(temp->right, i);
		pCode[i] = '\0';

		if(temp->left == NULL && temp->right == NULL){ //자식노드가 없으면 부호화 완료
			cAscii(temp->symbor, pCode);
		}
	}
}

//======== Huffman 알고리즘을 구현한 function
void Huffman(){
	TreeNode *temp;
	Node u,v,x;

	for(int i=0;i<n-1;i++){

		//min-heap에서 가장 작은 두개의 트리노드를 삭제하고, 가져온다
		u = Delete();
		v = Delete();

		//두개의 트리노드를 이어줄 부모 노드를 만들어 초기화한다
		//u, v의 frq 합을 저장하고
		//새롭게 만들어진 트리노드를 가리키는 heap 노드를 생성하여 다시 insert한다.
		temp = makeNode(u.TNode, v.TNode);
		x.key = u.key+v.key;
		temp->frq = x.key;
		x.TNode = temp;
		Insert(x);
	}

	x = Delete(); //완성된 tree의 root를 가져온다


	toBitcodes(x.TNode, -1);
}

//======== 바이너리 문자를 8비트 변환 후, 파일에 char문자로 쓰는 function
void unpack(FILE* file,FILE* wfile,int add_bit){
	int i,j,k,l;
	int flag;
	char bit8[9];
	char temp[100];
	unsigned char c, d;
	fgetc(file);
	c = fgetc(file);
	k=0;
	j=0;

	while(!feof(file)){
		
		//8비트로 변환
		for(i=7;i>=0;i--){
			bit8[i] = c%2 + '0';
			c>>=1;
		}
		d = fgetc(file);
		
		if(feof(file)){
			for(i=0;i<add_bit;i++){
				bit8[8-add_bit+i] = 0;
			}				
		}

		k=0;	
		while(k<8){
			temp[j] = bit8[k]; //비트를 하나씩 옮긴다
			flag = codeAscii(temp); //일치하는 허프만 코드를 찾는다
			
			if(flag == -1){ //없으면 그 다음 비트를 옮긴다
				j++;k++;	
			}
			else{ //일치하는 문자를 찾으면 출력한다.
				fputc(flag, wfile); 
				for(l=0;l<100;l++){
					temp[l] = 0;
				}//옮겨적을 배열 초기화
				k++;
				j = 0;
			}	
		
			if(bit8[k] == 0) break;
		}
		c = d;

	}
	
	for(i=0;i<add_bit;i++){
		bit8[8-add_bit+i] = 0;
	}
	


}

//======== 파일을 읽는 function
void readFile(char filename[200], int op){
	FILE* file = fopen(filename, "r");
	char sfilename[200];
	sprintf(sfilename, "%s.yy",filename);
	char c;
	int i,j;

	if(op == 1){ // compression
		TreeNode *temp;
		Node x;
		for(i=0;i<129;i++){
			heap[i].key = 0;
		}//heap의 key 초기화

		int flag=0;
		while(1){
			c = fgetc(file); //파일에서 한글자 입력받는다
			if(c == EOF) break;
		
			flag = 0;
			for(j=1;j<=n;j++){
				if(heap[j].TNode->symbor == c){
					flag = 1; // 이미 한 번 나온 symbor이면 frq를 증가시킨다
					heap[j].TNode->frq++;
					heap[j].key++;
					break;
				}
			}

			if(flag == 0){ //heap에 존재하지 않는 문자라면 노드를 만들어준다.
				temp = makeNode(NULL, NULL);
				temp->symbor = c;
				temp->frq = 1;
				x.TNode = temp;
				x.key = 1;

				Insert(x);
				n++;	
			}
		}
	}
	else{ // decompression
		FILE* wfile = fopen(sfilename, "w");
		int cur;
		int add_bit;
		char buf[129];
		int flag = 0;

		fscanf(file,"%d %d",&add_bit,&n); //끝에 추가비트수와 전체 문자 갯수 저장
		c = fgetc(file);

		// 문자와 매칭되는 비트 정보를 저장한다
		for(i=0;i<n;i++){
			c = fgetc(file);//c에는 현재 문자에 대한 정보
			cur = c;//비트를 하나씩 받을 커서
			j=0;

			while(1){
				cur = fgetc(file);
				if(j != 0 && cur == ' ') break;
				else if((i == n-1 && j != 0) && cur == '\n') break; //문자 \n 구분하는 예외처리
				list[c].bits[j] = cur;
				j++;

			}
			list[c].bits[j] = 0;
		}

		unpack(file,wfile,add_bit); // .zz.yy파일 작성

		fclose(wfile);

	}
	fclose(file);
}

//======== 01문자열을 8비트씩 나누어 바이너리문자로 반환
unsigned char pack(char *buf){
	int i;
	unsigned char x = 0;
	for(i=7;i>=0;i--){
		if(*buf == '1'){
			x = x|(1<<i);
		}
		buf++;
	}
	return x;
}

//======== pack()함수를 buf길이만큼 반복시킨다
void byteTobit(char buf[], FILE* file){
	int i;
	int len = strlen(buf);
	char *ptr;

	ptr = buf;
	for(i=0;i<len/8;i++){
		unsigned char bit = pack(ptr);
		fputc(bit,file);
		ptr += 8;
	}
	

}

//======== 압축한 파일 출력하는 function
void writeFile(char filename[200]){
	char wFilename[200];
	sprintf(wFilename, "%s.zz", filename);
	FILE* wfile = fopen(wFilename, "w");
	FILE* rfile = fopen(filename, "r");

	//------------문자 갯수 출력, 추가 비트수 자리 확보
	fprintf(wfile, "m %d\n",n);

	//------------문자와 매칭되는 코드 출력
	int bitLen;
	char* cbuf;

	for(int i=0;i<128;i++){
		if(list[i].bits[0] != '\0'){
			bitLen = strlen(list[i].bits);
			cbuf = (char*)malloc(sizeof(char)*(bitLen+3));

			if(i==0){
				fputc('\0', wfile);
				fputs(list[0].bits, wfile);
				fputc(' ', wfile);
				continue;
			}


			cbuf[0] = list[i].symbor;
			cbuf[1] = '\0';

			strcat(cbuf, list[i].bits);

			cbuf[bitLen+1] = ' ';
			cbuf[bitLen+2] = '\0';
			fputs(cbuf, wfile);
			free(cbuf);
		}
	}

	fputc('\n', wfile);	
	//---------------------------

	//-----------허프만부호화를 한 결과 출력
	int i,j,x,y,p,q;
	char rCur;

	fclose(rfile);

	rfile = fopen(filename,"r");
	char buf[129] = {0};
	buf[0] = 0;

	while(1){ //비트를 128개 까지만 받아서 바이너리 문자로 변환해 파일에 쓴다
		rCur = fgetc(rfile);
		if(rCur == EOF) break;

		i = rCur;

		x = strlen(list[i].bits); // 현재 문자의 코드
		y = strlen(buf); // 현재까지 채워진 buf 크기

		if(y+x>128){

			p = x+y-128; // 다음 버퍼에 적을 개수
			q = x-p; // 현재 버퍼 끝자리에 적을 개수

			for(j=0;j<q;j++){
				buf[y+j] = list[i].bits[j];
			}
			buf[128] = 0;

			byteTobit(buf, wfile);

			for(int k=0;k<128;k++){
				buf[k] = 0;
			}

			for(j=0;j<p;j++){
				buf[j] = list[i].bits[q+j];
			}
		}
		else{// buf가 128개까지 다 차지 않을경우 계속 비트를 복사한다
			strcat(buf, list[i].bits);
		}
	}
	y = strlen(buf);

	if(y%8 != 0){//8비트 단위로 끊어지게 마지막에 0을 추가한다
		x = y%8;
		for(i=0;i<8-x;i++){
			buf[y+i]='0';
		}
		buf[y+i] = 0;
	}
	byteTobit(buf, wfile);

	fseek(wfile,0,SEEK_SET);//추가한 0비트 수를 파일에 입력

	if(y%8==0) fprintf(wfile, "%d", 0);
	else fprintf(wfile, "%d", 8-x);

	//----------------------------------

	fclose(wfile);
	fclose(rfile);
}

int main(int argc, char* argv[]){

	char inFilename[200];
	int option;// 압축은 1, 풀기는 0

	//-------------- option check & error message
	if(argc!=3){
		printf("usage: %s [option] [inputfilename]\n", argv[0]);
		return 0;
	}
	else if(strcmp("-c",argv[1]) == 0){
		option = 1;
	}
	else if(strcmp("-d",argv[1]) == 0){
		option = 0;
	}
	else if((strcmp("-c",argv[1]) != 0) || (strcmp("-d",argv[1]) != 0)){
		printf("option error!\n");
		return 0;
	}

	strcpy(inFilename, argv[2]);

	FILE* f = fopen(inFilename, "r");
	if(f==NULL){
		printf("File does not exist!\n");
		return 0;
	}
	fclose(f);
	//--------------------------

	for(int i=0;i<128;i++){
		list[i].symbor = i;
	}//ascii code 갯수만큼 초기화한다

	switch(option){
		case 0: // 압축해제
			readFile(inFilename, 0); //	readFile()에 unpack()포함됨	
			break;
		case 1: // 압축
			readFile(inFilename,1);
			Huffman();
			writeFile(inFilename);
			break;
		default:
			break;
	}

	return 0;
}
