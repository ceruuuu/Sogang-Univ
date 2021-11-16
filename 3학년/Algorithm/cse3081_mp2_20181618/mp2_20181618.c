#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

int* list;
int n; // 전체 리스트 크기
clock_t start_t, end_t;

void swap(int* a, int* b){ // swap 함수
	int t;

	t = *a;
	*a = *b;
	*b = t;
}

int three_mid(int start, int mid, int end){ // index를 파라미터로 받고, 셋 중 중간값의 index를 return한다.
	if((list[start]<=list[mid])&&(list[mid]<=list[end]) || (list[end]<=list[mid])&&(list[mid]<=list[start])) return mid;
	if((list[mid]<=list[start])&&(list[start]<=list[end]) || (list[end]<=list[start])&&(list[start]<=list[mid])) return start;
	if((list[start]<=list[end])&&(list[end]<=list[mid]) || (list[mid]<=list[end])&&(list[end]<=list[start])) return end;
}

void al1_insertion(){ // insertion sort
	
	int i, j; 
	int key;

	for(i=2; i<=n; i++){ // 첫 번째 인덱스는 이미 정렬된 것으로 간주한다.
		key = list[i]; // 현재 삽입 될 i번째 리스트의 정수를 key에 복사한다.
		for(j=i-1; j>0; j--){ // i-1번째부터 역순으로 조사한다.
			if(list[j]>key){
				list[j+1] = list[j]; // key보다 크면 오른쪽으로 값을 옮긴다.
			}
			else break; // 작으면 반복문을 종료시킨다.
		}
		list[j+1] = key; // 빈 자리에 삽입한다.
	}


}


int partition(int left, int right){ 
	int i;
	int pivot;

	// 배열의 맨 오른쪽이 최종 pivot으로 return된다.
	// 정렬할 리스트의 가장 왼쪽, 중간, 가장 오른쪽의 값 중 중간값을 pivot으로 정한다.
	swap(&list[right], &list[three_mid(left, (left+right)/2 , right)]);

	pivot = left; // pivot의 초기값은 리스트의 맨 왼쪽 인덱스이다
	for(i=left;i<right;i++){ 
		if(list[i]<list[right]){ // 리스트의 맨 오른쪽 값보다 작으면 
			swap(&list[i], &list[pivot]); // pivot 인덱스의 값과 swap하고
			pivot++; // pivot값을 증가시킨다
		}
	}
	swap(&list[right], &list[pivot]); // 맨 오른쪽 값을 기준으로 정렬되었기 때문에 
									  // 최종 pivot 인덱스와 바꾸어준다
	return pivot;
}

void al2_quick(int left, int right){
	int pivot;

	if(right - left > 0){
		
		pivot = partition(left, right); // partition()을 호출하여 pivot을 구한다

		al2_quick(left, pivot-1); // pivot기준 왼쪽 묶음 recursive
		al2_quick(pivot+1, right); // pivot기준 오른쪽 묶음 recursive
	}
}

void adjust(int root, int num){
	int child;
	int temp;

	temp = list[root]; 
	child = 2*root; // root의 왼쪽 자식
	
	while(child <= num){
		if((child<num) && (list[child]<list[child+1])) 
			child++; // 오른쪽 자식이 존재하고
			     	 // 왼쪽, 오른쪽 자식의 값을 비교해서 큰 값 결정
	
		if(temp>list[child]) break; // 부모가 자식보다 크면 반복문 종료
		
		list[child/2] = list[child]; // 작으면 자식이 부모가 되고
		child *= 2;	// 그 부모의 자식과 다시 비교한다.
	}
	list[child/2] = temp; // 빈 자리에 root값을 저장한다.

}

void heapify(int parent, int size, int start){
	int left, right, largest;
	left = (parent*2) - start;
	right = (parent*2)+1 - start;

	if((left<size)&&(list[left]>list[parent]))
		largest = left;
	else
		largest = parent;

	if((right<size)&&(list[right]>list[largest]))
		largest = right;

	if(largest != parent){
		swap(&list[parent], &list[largest]);
		heapify(largest, size, start);
	}
}

void al3_heap(int left, int right){

	int i,j;
	int num = right-left+1; // num은 정렬할 리스트의 크기이다

	for(i=num/2;i>=left;i--) adjust(i, num); // max heap tree를 생성한다.
						
	for(i=num-1;i>=left;i--){ // max heap을 오름차순으로 정리한다.
		swap(&list[left], &list[i+1]);
		adjust(left,i);
	}
	
}


void al4_intro(int left, int right, int depthlimit){

	if(depthlimit < 0){ // quick sort의 worst case O(n^2)를 막기 위해 depthlimit가 0이 되면 heap sort 진행.
		al3_heap(left, right);
	}

	int pivot;
	if(right - left > 0){ //일반적인 quick sort와 동일하다
		pivot = partition(left, right);

		al4_intro(left, pivot-1, depthlimit-1);
		al4_intro(pivot+1, right, depthlimit-1);
	}
		
}


void write_File(char in_filename[200], int index){
	
	//result_index_inputfilename.txt 파일 열기
	char filename[200];	
	sprintf(filename, "result_%d_%s", index, in_filename);
	FILE* file = fopen(filename, "w");

	// 함수 실행시간 계산
	double runnig_t = (double)(end_t - start_t)/CLOCKS_PER_SEC; 
	
	fprintf(file, "%s\n", in_filename); // inputfilename 출력
	fprintf(file, "%d\n", index); // 알고리즘 인덱스 출력
	fprintf(file, "%d\n", n); // 리스트 크기 출력
	fprintf(file, "%.6lf\n", runnig_t); // 함수 실행시간 출력

	for(int i=1;i<=n;i++){
		fprintf(file, "%d ",list[i]); // 정렬한 리스트 출력
	}
	
	fclose(file);
}

void read_File(char filename[200]){
	int i,j;
	FILE* file = fopen(filename, "r"); // 파일 오픈

	fscanf(file, "%d", &n); // 리스트 크기를 읽는다
	list = (int*)malloc(sizeof(int)*(n+1)); // n+1크기 배열 동적할당

	for(i=1;i<=n;i++){	
		fscanf(file, "%d", &list[i]); // 리스트 성분 읽기
	}

	fclose(file);
}

int main(int argc, char* argv[]){
	
	if(argc != 3){ 
		printf("usage: %s inputfilename.txt algo_index\n", argv[0]);
		return 0;
	}
	
	char filename[200];
	strcpy(filename, argv[1]); // inputfilename 저장
	int algo_index = atoi(argv[2]); // 실행 알고리즘 인덱스 저장

	read_File(filename); // 리스트 정보 읽어온다

	int depthlimit = log(n)*2;

	switch (algo_index){ // 선택된 알고리즘 실행
		case 1:
			start_t = clock();
			al1_insertion();
			end_t = clock();
			write_File(filename, 1);
			break;
		case 2:
			start_t = clock();
			al2_quick(1, n);
			end_t = clock();
			write_File(filename, 2);
			break;
		case 3:
			start_t = clock();
			al3_heap(1, n);
			end_t = clock();
			write_File(filename, 3);
			break;
		case 4:
			start_t = clock();
			if(n<20) al1_insertion(); // n이 작으면 insertion이 유리하다.
			else al4_intro(1, n, depthlimit);
			end_t = clock();
			write_File(filename, 4);
			break;
	}

	return 0;

}
