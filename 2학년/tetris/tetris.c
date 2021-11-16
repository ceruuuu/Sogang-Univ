#include "tetris.h"

static struct sigaction act, oact;
int rotateCases[7] = {2, 4, 4, 4, 1, 2, 2};
int maxscore = 0;
long long int space = 0;
double treetime = 0;
int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	
	createRankList();
	srand((unsigned int)time(NULL));

	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_REC_PLAY: recommendedPlay(); break;
		case MENU_EXIT: exit=1; break;
		case '5': ExtremeRec(); break;
		default: break;
		}
	}
	
	writeRankFile();
	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	nextBlock[0]=rand()%7;
	nextBlock[1]=rand()%7;
	nextBlock[2]=rand()%7;
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;
		
	recRoot = (RecNode*)malloc(sizeof(RecNode));
	maxscore = 0;
	recRoot->lv = 0;
	recRoot->score = 0;
	for(i = 0; i < HEIGHT; i++)
	{
		for(j = 0; j < WIDTH; j++) recRoot->recF[i][j] = field[i][j];
	}
	
	modified_recommend(recRoot);

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);

	move(9, WIDTH+10);
	DrawBox(10,WIDTH+10,4,8);
	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(16,WIDTH+10);
	printw("SCORE");
	DrawBox(17,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(18,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}

	for( i = 0; i < 4; i++ ){
		move(11+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[2]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}

}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			free(recRoot);
			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	free(recRoot);
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int i, j;
	int flag = 1;	
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(block[currentBlock][blockRotate][i][j] == 1)
			{
				// Range Check
				if(blockY + i >= HEIGHT) flag = 0;
				if(blockX + j < 0 || blockX + j >= WIDTH) flag = 0;
				if(f[blockY + i][blockX + j] == 1) flag = 0;
			}
		}
	}
	// printf("flag: %d\n", flag);
	return flag;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	// Reverse
	int tempY = blockY, tempX = blockX, tempR = blockRotate;
	int i, j;
	
	switch(command)
	{
		case KEY_DOWN:
			// go up
			tempY--;
			break;
		case KEY_LEFT:
			// go right
			tempX++;
			break;
		case KEY_RIGHT:
			// go left
			tempX--;
			break;
		case KEY_UP:
			// rotate (reverse)
			tempR = (tempR + 3) % 4;
			break;
	}

	// erase previous block
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(block[currentBlock][tempR][i][j] == 1)
			{
				if(tempY + i + 1 == 0) continue;
				move(tempY + i + 1, tempX + j + 1);
				printw(".");
			}
		}
	}

	// erase previous shadow
	
	while(CheckToMove(field, currentBlock, tempR, tempY, tempX)) tempY++;
	tempY--;

	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(block[currentBlock][tempR][i][j] == 1)
			{
				move(tempY + i + 1, tempX + j + 1);
				printw(".");
			}
		}
	}


	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
}



void BlockDown(int sig){
	// user code
	int i, j;

	int flag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX);
	timed_out = 0;

	if(flag == 1)
	{
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
		
		return;
	}

	if(blockY == -1) gameOver = true;
	
	// Add Current Block, Delete Field, Add Score and Change to next
	score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
	score += DeleteLine(field);

	// Initialize	
	nextBlock[0] = nextBlock[1];
	nextBlock[1] = nextBlock[2];
	nextBlock[2] = rand() % 7;


	// Recommendation
	
	recRoot->lv = 0;
	recRoot->score = 0;
	maxscore = 0;
	for(i = 0; i < HEIGHT; i++)
	{
		for(j = 0; j < WIDTH; j++) recRoot->recF[i][j] = field[i][j];
	}
	modified_recommend(recRoot);

	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;


	// Draw UI
	DrawNextBlock(nextBlock);
	PrintScore(score);
	DrawField();
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int i, j;
	int addScore = 0;
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(block[currentBlock][blockRotate][i][j] == 1)
			{
				if(blockY + i == 21) addScore += 10;
				else if(f[blockY + i + 1][blockX + j] == 1) addScore += 10;
				f[blockY + i][blockX + j] = 1;
			}
		}
	}

	return addScore;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code
	int i, j, k;
	int temp;
	int deleted = 0;
	for(i = HEIGHT - 1; i >= 0; i--)
	{
		temp = 0;
		for(j = 0; j < WIDTH; j++)
		{
			if(f[i][j] == 1) temp++;
		}

		if(temp == WIDTH)
		{
			// Overwrite Line
			for(k = i; k > 0; k--)
			{
				for(j = 0; j < WIDTH; j++)
					f[k][j] = f[k - 1][j];
			}
			deleted++;
			i++;
		}
	}

	return deleted * deleted * 100;
}

void DrawShadow(int y, int x, int blockID,int blockRotate){
	// user code
	int i, j;
	int maxY = y;

	while(CheckToMove(field, blockID, blockRotate, maxY, x)) maxY++;
	maxY--;
	DrawBlock(maxY, x, blockID, blockRotate, '/');
	// erase previous block
}


void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate)
{
	DrawShadow(y, x, blockID, blockRotate);
	DrawBlock(y, x, blockID, blockRotate, ' ');
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
}

void createRankList(){
	FILE *file;
	int score;
	char name[NAMELEN];
	nptr last = head;
	nptr newNode;

	file = fopen("rank.txt", "r");

	if(file == NULL) 
	{
		printw("FILE OPEN ERROR: NO RANK.TXT\n");
		return;
	}

	fscanf(file, "%d", &rankSize);

	while(fscanf(file, "%s %d", name, &score) != EOF)
	{
		newNode = (nptr)malloc(sizeof(Node));
		newNode->score = score;
		strcpy(newNode->name, name);
		newNode->link = NULL;

		if(head == NULL)
		{
			head = newNode;
			last = head;
		}
		else
		{
			last->link = newNode;
			last = newNode;
		}
	}

	fclose(file);
}

void PrintXtoY()
{
	nptr node = head;
	int x = 1, y = rankSize, count = 1;
	int printflag = 0;
	
	// keyboard input visible
	echo();

	printw("X: ");
	scanw("%d", &x); // printw("X : %d\n", x);
	printw("Y: ");
	scanw("%d", &y); //printw("Y : %d\n", y);
	
	// invisible
	noecho();
	printw("\n       name       |   score   \n");
	printw("------------------------------\n");

	while(node != NULL)
	{
		if(x < 1) break;

		if(count >= x && count <= y) 
		{
			printw(" %-16s | %d\n", node->name, node->score);
			printflag = 1;
		}
		else if(count > y) break;
		count++;
		node = node->link;
	}

	if(printflag == 0) printw("\nsearch failure: no rank in the list\n");
}



void PrintWithName()
{
	char str[NAMELEN];
	nptr node = head;
	int printflag = 0;

	echo();

	printw("Input the name: ");
	getstr(str);
	// invisible
	noecho();
	
	printw("\n       name       |   score   \n");
	printw("------------------------------\n");
	
	while(node != NULL)
	{
		if(strcmp(node->name, str) == 0) 
		{
			printw(" %-16s | %d\n", node->name, node->score);
			printflag = 1;
		}
		node = node->link;
	}

	if(printflag == 0) printw("\nsearch failure: no rank in the list\n");
}

void DeleteRank()
{
	int count = 1;
	int rank;
	nptr node = head;
	nptr prev;

	echo();
	printw("input the rank: ");
	scanw("%d", &rank);
	noecho();

	if(rank < 1 || head == NULL)
	{
		printw("\nsearch failure: the rank not in the list\n");
		return;
	}

	if(rank == 1)
	{
		head = node->link;
		rankSize--;
		free(node);
		return;
	}

	while(node != NULL)
	{
		if(count == rank) break;
		prev = node;
		node = node->link;
		count++;
	}

	if(count - 1 == rankSize && count <= rank)
	{
		printw("\nsearch failure: the rank not in the list\n");
		return;
	}
	rankSize--;
	prev->link = node->link;
	free(node);

	printw("\nresult: the rank deleted\n");
}

void rank(){
	// user code
	int select = 0;
	int exit = 0;
	char str[10];
	clear();
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	
	while(!exit)
	{
		select = wgetch(stdscr);
		switch(select)
		{
			case '1': PrintXtoY(); exit = 1; break;
			case '2': PrintWithName(); exit = 1; break;
			case '3': DeleteRank(); exit = 1; break;
		}
	}

	wgetch(stdscr);
}

void writeRankFile(){
	// user code
	
	FILE* file = fopen("rank.txt", "w");
	nptr prev = head;

	fprintf(file, "%d\n", rankSize);
	while(head != NULL)
	{
		fprintf(file, "%s %d\n", head->name, head->score);
		prev = head;
		head = head->link;
		free(prev);
	}

	free(head);

	fclose(file);
}

void newRank(int score){
	// user code
	char str[NAMELEN];
	nptr prev;
	nptr curr = head;
	nptr node;
	
	rankSize++;

	clear();
	
	printw("your name: ");
	
	// keyboard input visible
	echo();
	getstr(str);
	// invisible
	noecho();

	node = (nptr)malloc(sizeof(Node));

	strcpy(node->name, str);
	node->score = score;
	node->link = NULL;

	if (head == NULL) head = node;

	else
	{
		while(curr != NULL)
		{
			if(curr -> score < score)
			{
				if(curr == head)
				{
					node->link = head;
					head = node;
					return;
				}

				else
				{
					node->link = curr;
					prev->link = node;
					return;
				}
			}

			prev = curr;
			curr = curr->link;
		}

		prev->link = node;
	}
}

void DrawRecommend(int y, int x, int blockID,int blockRotate){
	DrawBlock(y, x, blockID, blockRotate, 'R');
}



int recommend(RecNode *root){
	int max=root->score; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int i, j, k, x;
	int maxX = WIDTH / 2 - 2, maxY = -1, minX;
	int index = 0;	// Total root->c[index] number
	RecNode* temp = NULL;
	RecNode* child;

	struct timeval start, stop;


	if(root->lv == 0)
	{
		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 4; j++)
			{
				if(block[nextBlock[0]][blockRotate][i][j] == 1)
				{
					if(field[maxY + i][maxX + j] == 1)
					{
						gameOver = 1;
						return 0;
					}
				}
			}
		}

	}

	if(root->lv == 0) gettimeofday(&start, NULL);

	// rotation
	for(i = 0; i < rotateCases[nextBlock[root->lv]]; i++)
	{
		maxX = WIDTH / 2 - 2;
		// position	
		while(CheckToMove(root->recF, nextBlock[root->lv], i, 0, maxX)) maxX--;
		minX = maxX + 1;
		x = minX;
		while(CheckToMove(root->recF, nextBlock[root->lv], i, 0, x)) {x++; maxX++;}
			
		if(root->lv == 0 && x <= minX)
		{
			gameOver = 1;
		}

		for(x = minX; x <= maxX; x++)
		{
			maxY = -1;

			while(CheckToMove(root->recF, nextBlock[root->lv], i, maxY + 1, x)) maxY++;
			// Init root->c[index]
			
			child = root->c[index++];

			child = (RecNode*)malloc(sizeof(RecNode));
			space += sizeof(RecNode);
			child->lv = root->lv + 1;
			child->score = root->score;
			child->parent = root;
			child->recX = x;
			child->recY = maxY;
			child->recR = i;
			child->curBlockID = nextBlock[root->lv];

			for(k = 0; k < HEIGHT; k++)
			{
				for(j = 0; j < WIDTH; j++)
				{
				child->recF[k][j] = root->recF[k][j];
				}
			}

			child->score += AddBlockToField(child->recF, nextBlock[root->lv], i, maxY, x);
			child->score += DeleteLine(child->recF);
			


			if(child->lv < VISIBLE_BLOCKS)
			{
				recommend(child);
			}

			else
			{
				if(maxscore < child->score)
				{
					maxscore = child->score;
					temp = child;
						
					while(temp->parent != recRoot) temp = temp->parent;
					recommendY = temp->recY;
					recommendX = temp->recX;
					recommendR = temp->recR;

				}
			}
		}
	}



	// Free Everything
	for(i = 0; i < index; i++)
	{
		free(root->c[index]);
		root->c[index] = NULL;
	}


	if(root->lv == 0)
	{
		gettimeofday(&stop, NULL);
		treetime += (double)((stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / 1000000.0);
	}
	
	return max;
}

int modified_recommend(RecNode *root){
	int max=root->score; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int i, j, k, x;
	int maxX = WIDTH / 2 - 2, maxY = -1, minX;
	int index = 0;	// Total root->c[index] number
	RecNode* temp = NULL;
	RecNode* child;
	
	struct timeval start, stop;

	if(root->lv == 0)
	{
		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 4; j++)
			{
				if(block[nextBlock[0]][blockRotate][i][j] == 1)
				{
					if(field[maxY + i][maxX + j] == 1)
					{
						gameOver = 1;
						return 0;
					}
				}
			}
		}

	}


	if(root->lv == 0) gettimeofday(&start, NULL);



	// rotation
	for(i = 0; i < rotateCases[nextBlock[root->lv]]; i++)
	{
		maxX = WIDTH / 2 - 2;
		// position	
		while(CheckToMove(root->recF, nextBlock[root->lv], i, 0, maxX)) maxX--;
		minX = maxX + 1;
		x = minX;
		while(CheckToMove(root->recF, nextBlock[root->lv], i, 0, x)) {x++; maxX++;}
			
		if(root->lv == 0 && x <= minX)
		{
			gameOver = 1;
		}

		for(x = minX; x <= maxX; x++)
		{
			maxY = -1;

			while(CheckToMove(root->recF, nextBlock[root->lv], i, maxY + 1, x)) maxY++;
			// Init root->c[index]
			
			child = root->c[index++];

			child = (RecNode*)malloc(sizeof(RecNode));
			child->lv = root->lv + 1;
			child->score = root->score;
			child->parent = root;
			child->recX = x;
			child->recY = maxY;
			child->recR = i;
			child->curBlockID = nextBlock[root->lv];

			for(k = 0; k < HEIGHT; k++)
			{
				for(j = 0; j < WIDTH; j++)
				{
				child->recF[k][j] = root->recF[k][j];
				}
			}

			child->score += AddBlockToField(child->recF, nextBlock[root->lv], i, maxY, x);
			child->score += DeleteLine(child->recF);
			
			// Height Penalty
			child->score += maxY * 20;

			// Blank Penalty
			for(k = 0; k < 4; k++)
			{
				for(j = 0; j < 4; j++)
				{
					if(block[nextBlock[root->lv]][i][k][j] == 1)
					{
						if(maxY + k == 21) continue;
						else if(child->recF[maxY + k + 1][x + j] == 0) child->score -= 20;
					}
				}
			}

			if(child->lv < VISIBLE_BLOCKS)
			{
				modified_recommend(child);
			}

			else
			{
				if(maxscore < child->score)
				{
					maxscore = child->score;
					temp = child;
						
					while(temp->parent != recRoot) temp = temp->parent;
					recommendY = temp->recY;
					recommendX = temp->recX;
					recommendR = temp->recR;

				}
			}
		}
	}



	// Free Everything
	for(i = 0; i < index; i++)
	{
		free(root->c[index]);
		root->c[index] = NULL;
	}


	if(root->lv == 0)
	{
		gettimeofday(&stop, NULL);
		treetime += (double)((stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / 1000000.0);
	}



	return max;
}


void PrintComplexity(double t)
{
	clear();
	printw("Elapsed Time: %lf\n", t);
	printw("Score: %d\n", score);
	printw("Time(t): %lf\n", treetime);
	printw("Space(t): %lld\n", space);
}


void recommendedPlay(){
	// user code
	int command;
	int i, j;
	
	struct timeval start, stop;

	gettimeofday(&start, NULL);

	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	space = 0;
	treetime = 0;
	do{
		if(timed_out==0){
			
			blockX = recommendX;
			blockY = recommendY;
			blockRotate = recommendR;
			alarm(1);
			timed_out = 1;
		}

		command = GetCommand();
		if(command==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			free(recRoot);

			gettimeofday(&stop, NULL);
			PrintComplexity((double)((stop.tv_sec - start.tv_sec) + 
				(stop.tv_usec - start.tv_usec) / 1000000.0));

			getch();
			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	free(recRoot);
	gettimeofday(&stop, NULL);
	PrintComplexity((double)((stop.tv_sec - start.tv_sec) +
			(stop.tv_usec - start.tv_usec) / 1000000.0));

	getch();
}


void ExtremeRec()
{
	// user code
	int command;
	int i, j;
	struct itimerval timer;
	clear();
	InitTetris();
	act.sa_handler = BlockDown;
	sigaction(SIGVTALRM,&act,&oact);

	do
	{
		blockX = recommendX;
		blockY = recommendY;
		blockRotate = recommendR;
		BlockDown(0);

		command = GetCommand();
		if(command==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();
			free(recRoot);
			return;
		}
		
	} while(!gameOver);


	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	free(recRoot);

	ExtremeRec();
}
