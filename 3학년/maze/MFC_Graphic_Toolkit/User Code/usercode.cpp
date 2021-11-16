#include <stdlib.h>
#include "..\ExternDoc.h"
#include "..\UI\Zoom.h"
#include "..\UI\MsgView.h"
#include "..\Graphics\DrawFunc.h"
#include "..\Example\Example.h"

#define ERROR_NUMBER -1

//function prototype

static void drawDirect(CDC *pDC);
static void drawBuffered();

//Start of user code
#include <float.h>

struct me{
	int chk;
	bool right,under;
};
me **a;

struct ghost{
	int x, y;
};
char **map;
int n,m;//map 크기
int n2,m2;//자료구조 크기

int stack_cnt;
bool dfs_chk,bfs_chk;
ghost *stack;
ghost *q;
/*****************************************************************
* function	: bool readFile(const char* filename)
* argument	: cons char* filename - filename to be opened
* return	: true if success, otherwise flase
* remark	: After read data file, phisycal view must be set;
*			  otherwise drawing will not be executed correctly.
*			  The window will be invalidated after readFile()
*			  returns true.
******************************************************************/

bool readFile(const char* filename){
	char c,t=0;
	int cnt=0,cnt2=0,i,j;
	FILE *fp=fopen(filename,"r");
	if(fp==NULL) return false;
	//start of the user code
	while(1){
		if(fscanf(fp,"%c",&c)==EOF){
			if(t!='\n') cnt2++;
			break;
		}
		if(c=='\n'){m=cnt+1; cnt2++; cnt=-1;} t=c;
		cnt++;
	}
	n=cnt2;
	fclose(fp); fp=fopen(filename,"r");
	map=(char**)malloc(sizeof(char*)*n);
	for(i=0;i<n;i++){
		map[i]=(char*)malloc(sizeof(char)*m);
	}
	n2=n/2; m2=(m-1)/2;
	a=(me**)malloc(sizeof(me*)*n2);
	for(i=0;i<n2;i++){
		a[i]=(me*)malloc(sizeof(me)*m2);
	}

	for(i=0;i<n;i++){
		for(j=0;j<m;j++){
			fscanf(fp,"%c",&map[i][j]);
			if(i==n-1 && j==m-2) break;
		}
	}
	 cnt=0; m--;
	for(i=1;i<n-1;i+=2){
		cnt2=0;
		for(j=1;j<m-1;j+=2){
			a[cnt][cnt2].chk=a[cnt][cnt2].under=a[cnt][cnt2].right=0;
			if(i+2<n-1 && map[i+1][j]==' ')		a[cnt][cnt2].under=1;
			if(j+2<m-1 && map[i][j+1]==' ')	a[cnt][cnt2].right=1;
			cnt2++; 
		}
		cnt++;
	}

	stack=(ghost*)malloc(sizeof(ghost)*n2*m2+10);
	q=(ghost*)malloc(sizeof(ghost)*n2*m2+10);
	dfs_chk=0; bfs_chk=0;

	setWindow(0,0,m*2+1,n*2+1,1);

	return true; //edit after finish this function
	//end of usercode
}

/******************************************************************
* function	: bool FreeMemory()
*
* remark	: Save user data to a file
*******************************************************************/
void freeMemory(){
	//start of the user code
	//end of usercode
	int i;
	for(i=0;i<n;i++){
		free(map[i]);
	}
	free(map);
	for(i=0;i<n2;i++){
		free(a[i]);
	}
	free(a);

	free(stack);
}

/**************************************************************
* function	: bool writeFile(const char* filename)
*
* argument	: const char* filename - filename to be written
* return	: true if success, otherwise false
* remark	: Save user data to a file
****************************************************************/
bool writeFile(const char* filename){
	//start of the user code
	bool flag;
	flag = 0;

	return flag;
	//end of usercode
}

/************************************************************************
* fucntion	: void drawMain(CDC* pDC)
*
* argument	: CDC* pDC - device context object pointer
* remark	: Main drawing function. Called by CMFC_MainView::OnDraw()
*************************************************************************/
void drawMain(CDC *pDC){
	//if direct drawing is defined
#if defined(GRAPHICS_DIRECT)
	drawDirect(pDC);
	//if buffered drawing is defined
#elif defined(GRAPHICS_BUFFERED)
	drawBuffered();
#endif
}

/************************************************************************
* function	: static void drawDirect(CDC *pDC
*
* argument	: CDC* pDC - device context object pointer
* remark	: Direct drawing routines here.
*************************************************************************/
static void drawDirect(CDC *pDC){
	//begin of user code
	//Nothing to write currently.
	//end of user code
}

/***********************************************************************
* function	: static void drawBuffered()
*
* argument	: CDC* pDC -0 device object pointer
* remark	: Buffered drawing routines here.
************************************************************************/
static void drawBuffered(){
	//start of the user code
	//end of the user code
	int i,j;
	ghost t1,t2,t,tt;

	for(i=0;i<n;i++){
		for(j=0;j<m;j++){
			if(i%2==0){
				if(map[i][j]=='+'){
					DrawSolidBox_I((j/2)*4,(i/2)*4,(j/2)*4+1,(i/2)*4+1,1,RGB(51,51,153),RGB(51,51,153));
				}
				else if(map[i][j]=='-'){
					DrawSolidBox_I((j/2)*4+1,(i/2)*4,(j/2)*4+4,(i/2)*4+1,1,RGB(51,51,153),RGB(51,51,153));
				}
			}
			else{
				if(map[i][j]=='|'){
					DrawSolidBox_I((j/2)*4,(i/2)*4+1,(j/2)*4+1,(i/2)*4+4,1,RGB(51,51,153),RGB(51,51,153));
				}
			}
		}
	}
	
	DrawSolidBox_I(2,2,3,3,1,RGB(255,0,0),RGB(255,0,0));
	DrawSolidBox_I((m/2)*4-2,(n/2)*4-2,(m/2)*4-1,(n/2)*4-1,1,RGB(0,255,151),RGB(0,255,51));

	if(dfs_chk==1){
		for(i=0;i<n2;i++){
			for(j=0;j<m2;j++){
				if(a[i][j].chk==1){
					DrawSolidBox_I(j*4+2,i*4+2,j*4+3,i*4+3,1,RGB(204,204,204),RGB(204,204,204));
					if(i-1>=0 && a[i-1][j].chk==1 && a[i-1][j].under==1){
						DrawSolidBox_I(j*4+2,(i-1)*4+3,j*4+3,i*4+2,1,RGB(204,204,204),RGB(204,204,204));
					}
					if(j-1>=0 && a[i][j-1].chk==1 && a[i][j-1].right==1){
						DrawSolidBox_I((j-1)*4+3,i*4+2,j*4+2,i*4+3,1,RGB(204,204,204),RGB(204,204,204));
					}
				}
			}
		}
		for(i=stack_cnt-1;i>=0;i--){
			t=stack[i];
			DrawSolidBox_I(t.y*4+2,t.x*4+2,t.y*4+3,t.x*4+3,1,RGB(0,204,204),RGB(0,204,204));
			if(i!=0){
				if(stack[i-1].x==t.x){
					t1=stack[i-1]; t2=t;
					if(t1.y>t2.y){ t1=t; t2=stack[i-1];}
					DrawSolidBox_I(t1.y*4+3,t1.x*4+2,t2.y*4+2,t2.x*4+3,1,RGB(0,204,204),RGB(0,204,204));
				}
				else{
					t1=stack[i-1]; t2=t;
					if(t1.x>t2.x){ t1=t; t2=stack[i-1];}
					DrawSolidBox_I(t1.y*4+2,t1.x*4+3,t2.y*4+3,t2.x*4+2,1,RGB(0,204,204),RGB(0,204,204));
				}
			}
		}
	

	}
	if(bfs_chk==1){
		for(i=0;i<n2;i++){
			for(j=0;j<m2;j++){
				if(a[i][j].chk!=0){
					DrawSolidBox_I(j*4+2,i*4+2,j*4+3,i*4+3,1,RGB(204,204,204),RGB(204,204,204));
					if(i-1>=0 && a[i-1][j].chk!=0 && a[i-1][j].under==1){
						DrawSolidBox_I(j*4+2,(i-1)*4+3,j*4+3,i*4+2,1,RGB(204,204,204),RGB(204,204,204));
					}
					if(j-1>=0 && a[i][j-1].chk!=0 && a[i][j-1].right==1){
						DrawSolidBox_I((j-1)*4+3,i*4+2,j*4+2,i*4+3,1,RGB(204,204,204),RGB(204,204,204));
					}
				}
			}
		}

		t.x=n2-1; t.y=m2-1;
		while(1){
			DrawSolidBox_I(t.y*4+2,t.x*4+2,t.y*4+3,t.x*4+3,1,RGB(255,153,255),RGB(255,153,255));
			if(t.x==0 && t.y==0) break;
			tt=t;
			if(a[tt.x][tt.y].chk==1){//위에서 온 경우
				t.x=tt.x-1; t.y= tt.y;
			}
			else if(a[tt.x][tt.y].chk==2){//아래에서 온 경우
				t.x=tt.x+1; t.y=tt.y;
			}
			else if(a[tt.x][tt.y].chk==3){//왼쪽에서 온 경우
				t.x=tt.x; t.y=tt.y-1;
			}
			else{//오른쪽에서 온 경우
				t.x=tt.x; t.y=tt.y+1;
			}
			if(t.x==tt.x){
				t1=t; t2=tt;
				if(t1.y>t2.y){ t1=tt; t2=t;}
				DrawSolidBox_I(t1.y*4+3,t1.x*4+2,t2.y*4+2,t2.x*4+3,1,RGB(255,153,255),RGB(255,153,255));
			}
			else{
				t1=t; t2=tt;
				if(t1.x>t2.x){ t1=tt; t2=t;}
				DrawSolidBox_I(t1.y*4+2,t1.x*4+3,t2.y*4+3,t2.x*4+2,1,RGB(255,153,255),RGB(255,153,255));
			}
		}
	}

	DrawSolidBox_I(2,2,3,3,1,RGB(255,0,0),RGB(255,0,0));
	DrawSolidBox_I((m/2)*4-2,(n/2)*4-2,(m/2)*4-1,(n/2)*4-1,1,RGB(0,255,151),RGB(0,255,51));
}

void DFS(){
	int i,j;
	bool tu;
	ghost temp,t;

	bfs_chk=0;//////////////////////
	for(i=0;i<n2;i++){
		for(j=0;j<m2;j++) a[i][j].chk=0;
	}

	temp.x=0; temp.y=0; stack_cnt=0;
	stack[stack_cnt++]=temp; a[0][0].chk=1;
	while(stack_cnt!=0){
		temp=stack[stack_cnt-1];
		if(temp.x==n2-1 && temp.y==m2-1) break;
		tu=0;
		if(temp.x-1>=0 && a[temp.x-1][temp.y].under==1 && a[temp.x-1][temp.y].chk==0){
			tu=1; a[temp.x-1][temp.y].chk=1; t.x=temp.x-1; t.y=temp.y;
			stack[stack_cnt++]=t; continue;
		}
		if(temp.x+1<n2 && a[temp.x][temp.y].under==1 && a[temp.x+1][temp.y].chk==0){
			tu=1; a[temp.x+1][temp.y].chk=1; t.x=temp.x+1; t.y=temp.y;
			stack[stack_cnt++]=t; continue;
		}
		if(temp.y-1>=0 && a[temp.x][temp.y-1].right==1 && a[temp.x][temp.y-1].chk==0){
			tu=1; a[temp.x][temp.y-1].chk=1; t.x=temp.x; t.y=temp.y-1;
			stack[stack_cnt++]=t; continue;
		}
		if(temp.y+1<m2 && a[temp.x][temp.y].right==1 && a[temp.x][temp.y+1].chk==0){
			tu=1; a[temp.x][temp.y+1].chk=1; t.x=temp.x; t.y=temp.y+1;
			stack[stack_cnt++]=t; continue;
		}
		if(!tu){
			stack_cnt--;
		}
	}
	dfs_chk=1;
	drawBuffered();
}

void BFS(){
	int f,r,i,j;
	ghost t,temp;
	
	dfs_chk=0; //////////////////
	for(i=0;i<n2;i++){
		for(j=0;j<m2;j++) a[i][j].chk=0;
	}
	f=0; r=0;
	temp.x=0; temp.y=0; q[r++]=temp; a[0][0].chk=1;
	while(f!=r){
		temp=q[f++];
		if(temp.x==n2-1 && temp.y==m2-1) break;

		if(temp.x-1>=0 && a[temp.x-1][temp.y].under==1 && a[temp.x-1][temp.y].chk==0){
			a[temp.x-1][temp.y].chk=2; t.x=temp.x-1; t.y=temp.y;
			q[r++]=t;
		}
		if(temp.x+1<n2 && a[temp.x][temp.y].under==1 && a[temp.x+1][temp.y].chk==0){
			a[temp.x+1][temp.y].chk=1; t.x=temp.x+1; t.y=temp.y;
			q[r++]=t;
		}
		if(temp.y-1>=0 && a[temp.x][temp.y-1].right==1 && a[temp.x][temp.y-1].chk==0){
			a[temp.x][temp.y-1].chk=4; t.x=temp.x; t.y=temp.y-1;
			q[r++]=t;
		}
		if(temp.y+1<m2 && a[temp.x][temp.y].right==1 && a[temp.x][temp.y+1].chk==0){
			a[temp.x][temp.y+1].chk=3; t.x=temp.x; t.y=temp.y+1;
			q[r++]=t;
		}
	}
	bfs_chk=1;
	drawBuffered();
}