#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int n,m;
int** mat;
clock_t start_time, end_time;

int n6(){
	int sum, max = mat[0][0], maXsuM = mat[0][0];
	int i,j;
	int i_0,j_0,i_i,j_j;//i_0,j_0은 시작점, i_i,j_j는 끝점.

	
	//모든 element가 0이하일 경우 가장 큰 element를 return한다.
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			if(mat[i][j] > max) max = mat[i][j];
		}
	}
	if(max<=0) return max;

	
	for(i_0 = 0 ; i_0 < m ; i_0++){
		for(j_0 = 0 ; j_0 < n ; j_0++){ // 시작점 (i_0, j_0)를 (0,0) 부터 (m,n)까지 이동시킨다.
			for(i_i = i_0; i_i<m; i_i++){
				for(j_j = j_0; j_j<n;j_j++){ // 끝점 (i_i, j_j)를 (0,0) 부터 (m,n)까지 이동시킨다.


					sum = 0;
					for(i=i_0;i<=i_i;i++){	// 시작점부터 끝점까지 모든 성분을 sum에 더한다.
						for(j=j_0;j<=j_j;j++){
							sum += mat[i][j];
						}
					}
					if(sum>=maXsuM) maXsuM = sum; // sum이 최댓값이면 maXsuM에 저장한다.
						
				}
			}
		}
	}

	return maXsuM;
}

int n4(){
	int bef_sum, sum, max = mat[0][0], maXsuM = mat[0][0]; // bef_sum은 직전 반복문에서 계산된 sum값이 저장되는 변수이다.
	int i,j;
	int i_0,j_0,i_i,j_j;	

	
	//모든 element가 0이하일 경우
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			if(mat[i][j] > max) max = mat[i][j];
		}
	}
	if(max<=0) return max;


	//계산한 subretangle값을 저장하는 matrix. 
	int** save_mat;
	save_mat = (int**)malloc(sizeof(int*)*m);
	for(i=0;i<m;i++){
		save_mat[i] = (int*)malloc(sizeof(int)*n);
	}

	for(i_0=0;i_0<m;i_0++){
		for(j_0=0;j_0<n;j_0++){ // 시작점 (i_0, j_0)를 (0,0) 부터 (m,n)까지 이동시킨다.
			for(i_i = i_0; i_i<m; i_i++){
				for(j_j = j_0; j_j<n;j_j++){ //  끝점 (i_i, j_j)를 (0,0) 부터 (m,n)까지 이동시킨다.
					
					if(i_i == i_0 && j_j == j_0){ // subretangle이 1x1인 경우, 현재 인덱스의 성분만 sum에 더한다.
						sum = mat[i_i][j_j];
						save_mat[i_i][j_j] = sum;
						bef_sum = sum; 
					}
					else if(i_0 == i_i){ // subretangle이 1xl인 경우, 1x(l-1)의 bef_sum과 현재 인덱스의 성분을 더한다.
						sum = bef_sum + mat[i_i][j_j];
						save_mat[i_i][j_j] = sum;
						bef_sum = sum;
					}
					else if(j_0 == j_j){ // subretangle이 kx1인 경우, save_mat에 저장된 값과 현재 인덱스의 성분을 더한다.
						sum = save_mat[i_i-1][j_j] + mat[i_i][j_j];
						save_mat[i_i][j_j] = sum;
						bef_sum = sum;
					}
					else{ // kxl인 경우, (k-1)xl + kx(l-1) - (k-1)x(l-1) + 1x1 으로 구한다.
						  // (k-1)x(l-1)은 겹치는 영역이므로 한 번 빼준다.
						sum = bef_sum + save_mat[i_i-1][j_j] - save_mat[i_i-1][j_j-1] + mat[i_i][j_j]; 
						save_mat[i_i][j_j] = sum;
						bef_sum = sum;
					}
			
					if(sum>maXsuM) maXsuM = sum; // sum이 최댓값이면 maXsuM에 저장한다.

				}
			}
		}
	}

	return maXsuM;

}

int n3(){
	int sum, max = mat[0][0], maXsuM = mat[0][0];
	int i,j;
	int left = 0, right = 0;
	int aft;

	//모든 element가 0이하일 경우
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			if(mat[i][j] > max) max = mat[i][j];
		}
	}
	if(max<=0) return max;


	int* row_sum;
	row_sum = (int*)malloc(sizeof(int)*m); // left열부터 right열까지 동일한 행의 합을 저장하는 배열이다. 

	for(left=0;left<n;left++){ // left는 열 index이며, 처음부터 n까지 이동시킨다.	

		for(i=0;i<m;i++) row_sum[i] = 0;

		for(right=left;right<n;right++){ //left부터 n까지 이동시킨다.

			sum = 0;
			aft = 0; // 연속된 행의 합을 구하기 위한 행 index이다.
			for(i=0;i<m;i++){
				row_sum[i] += mat[i][right]; // left~right까지 동일한 행에 있는 성분들을 모두 더하고 저장한다. subretangle이 가로로 확장된다.

				sum += row_sum[aft]; // subretangle이 세로로 확장된다.
				
				if(sum>=maXsuM) maXsuM = sum; // sum이 최댓값이면 maXsuM에 저장한다.

				if(sum<0) sum = 0; // sum의 합이 음수가 될 경우, sum을 0으로 초기화하고 그 index를 건너뛴다.
				
				aft++;

				if(aft == m) break;
			}
	
		}
	}

	return maXsuM;

}


void write_File(char in_Filename[200], int index, int sum){ // 파일을 쓰는 함수이다.
	
	char filename[250] = "result_";
	strcat(filename, in_Filename);
	
	FILE* file = fopen(filename,"w");

	double running_time =  (double)(end_time - start_time)/CLOCKS_PER_SEC;
	running_time *= 1000;

	fprintf(file, "%s\n", in_Filename);
	fprintf(file, "%d\n", index);
	fprintf(file, "%d\n", m);
	fprintf(file, "%d\n", n);
	fprintf(file, "%d\n", sum);
	fprintf(file, "%.6f\n", running_time);

	fclose(file);
}

void read_File(char filename[200]){	//파일을 읽는 함수이다.
	
	int i,j;
	
	FILE* file = fopen(filename,"r");
	
	fscanf(file,"%d %d", &m, &n);
	mat = (int**)malloc(sizeof(int*)*m);
	for(i=0;i<m;i++){
		mat[i] = (int*)malloc(sizeof(int)*n);
	}

	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			fscanf(file, "%d", &mat[i][j]);
		}
	}

	fclose(file);
}


int main(int argc, char* argv[]){
	int sum;

	if(argc != 3){
		printf("usage: %s inputfilename.txt algo_index\n", argv[0]);
		return 0;
	}

	char filename[200];
	strcpy(filename, argv[1]); // input 파일 이름을 저장한다.
	int algo_index = atoi(argv[2]); // 실행시킬 알고리즘의 인덱스를 저장한다.

	read_File(filename);

	switch (algo_index){ // 입력된 인덱스에 따라 알고리즘을 실행시킨다.
		case 1:
			start_time = clock();
			sum = n6();
			end_time = clock();
			write_File(filename, 1, sum);
			break;
		case 2:
			start_time = clock();
			sum = n4();
			end_time = clock();
			write_File(filename, 2, sum);
			break;
		case 3:
			start_time = clock();
			sum = n3();
			end_time = clock();
			write_File(filename, 3, sum);
			break;
		default:
			printf("index가 이상한가봐요\n");
			break;
	}

	return 0;
	
}

