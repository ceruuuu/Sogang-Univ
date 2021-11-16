#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

#define KEY_SPACE ' ' // not defined in ncurses.h
#define KEY_Enter 10
#define START_ROW 5
#define START_COL 5

int HEIGHT, WIDTH;

void saveGame(int **board, int row, int col, int turn, int players){
 
    FILE* save;
    char save_name[256];
    int i, j;

    mvprintw(row+5, col+5,"ENTER FILE NAME : ");
    scanw("%s", save_name);

    save = fopen(save_name, "w");
    fprintf(save,"%d %d %d %d %d %d\n", row, col, turn,HEIGHT,WIDTH , players);
    
    for(j=0; j<WIDTH;j++){
        for(i=0; i<HEIGHT; i++)
            fprintf(save, "%d ", board[i][j]);
        fprintf(save,"\n");
    }
    

    fclose(save);
}

int** readSavedGame(int **board, int *row, int *col, int *turn, char save_name[256], int *players){
    
    FILE* read;
    int s_row, s_col, s_turn;
    int i,k;

    read = fopen(save_name, "r");
    fscanf(read, "%d %d %d %d %d %d", &s_row, &s_col, &s_turn,&HEIGHT,&WIDTH, players);
	
    board = (int**)malloc(sizeof(int*)*HEIGHT);
	for(int i=0; i<HEIGHT; i++){
		board[i] = (int*)malloc(sizeof(int)*WIDTH);
	}
    *row = s_row;
    *col = s_col;
    *turn = s_turn;

    for(k=0;k<HEIGHT;k++){
        for(i=0;i<WIDTH;i++){
            fscanf(read,"%d", &board[i][k]);
        }
    }

    fclose(read);

    return board;

}

int** initBoard(int **board, int *row, int *col, int *turn, int load){
	board = (int**)malloc(sizeof(int*)*HEIGHT);
	for(int i=0; i<HEIGHT; i++){
		board[i] = (int*)malloc(sizeof(int)*WIDTH);
	}

	
	board[0][0] = ACS_ULCORNER;//'┌'
	for (int i=1; i < WIDTH-1; i++)
		board[0][i] = ACS_TTEE;//'┬'
	board[0][WIDTH-1] = ACS_URCORNER; //'┐'

	for (int i=1; i<HEIGHT-1; i++){
		board[i][0] = ACS_LTEE; // '├'
		for (int j=1; j < WIDTH-1; j++)
			board[i][j] = ACS_PLUS; //'┼'
		board[i][WIDTH-1] = ACS_RTEE; //'┤'
	}

	board[HEIGHT-1][0] = ACS_LLCORNER; //'└'
	for (int i=1; i < WIDTH-1; i++)
		board[HEIGHT-1][i] = ACS_BTEE; //'┴'
	board[HEIGHT-1][WIDTH-1] = ACS_LRCORNER; // '┘'

	return board;
}


void paintBoard(int **board, WINDOW *win, int row, int col){
	
    int i,j;

    for(i=0; i<HEIGHT; i++){
        for(j=0; j<WIDTH; j++){
        mvwaddch(win,i,j,board[i][j]);
        }
    }
    mvprintw(HEIGHT+13,8,"                    ");
    mvprintw(HEIGHT+12, 28, "  ");
    wrefresh(win);
    return;
    
}

int checkWin(int row, int col, int **board, int players){ 
	
    int i;
    int check1=0, check2=0, check3=0,check4=0, check5=0;
    int p = board[row][col];

    //왼쪽확인
    for(i=1;i<7-players;i++){
        if (col-i >= 0 && p==board[row][col-i])
            check1++;
        else
            break;
    }
    //오른쪽확인
    for(i=1;i<7-players;i++){
        if (col+i < WIDTH && p==board[row][col+i])
            check1++;
        else
            break;
    }
    //위쪽 확인
    for(i=1;i<7-players;i++){
        if(row-i >= 0 && p==board[row-i][col])
            check2++;
        else
            break;
    }
    //아래쪽 확인
    for(i=1;i<7-players;i++){
        if(row + i < HEIGHT && p==board[row+i][col])
            check2++;
        else
            break;
    }
    //오른쪽아래 대각선
    //위로
    for(i=1;i<7-players;i++){
        
        if(row-i >= 0 && col - i >= 0){
            
            
            if (p==board[row-i][col-i])
                check3++;
        }
        else
            break;
    }
    //아래로
    for(i=1;i<7-players;i++){
        
        if(row+i < HEIGHT && col + i < WIDTH){
            
            
            if (p==board[row+i][col+i])   
            check3++;
        }
        else
            break;
    }
    
    //왼쪽아래 대각선
    //위로
    for(i=1;i<7-players;i++){
        
        if(row-i>=0 && col+i < WIDTH){
            
            
            if (p==board[row-i][col+i])
            check4++;
        }
        else
            break;
    }
    //아래로
    for(i=1;i<7-players;i++){
        
        if(row+i < HEIGHT && col-i >=0){
            
            
            if (p==board[row+i][col-i])
            check4++;
        }
        else
            break;
    }
    
    
    if (check1 == 6-players) return 1;
    else if (check2 == 6-players) return 1;
    else if (check3 == 6-players) return 1;
    else if (check4 == 6-players) return 1;

    else return 0;

}

int Action(WINDOW *win, int **board, int keyin, int *row, int *col, int *turn, int players, char *exit){
    
    switch(keyin){
        case KEY_UP:
            if(*row >= 1){
                (*row)--;
            }
            break;

        case KEY_DOWN:
            if(*row <= HEIGHT-2){
                (*row)++;
            }
            break;

        case KEY_LEFT:
            if(*col >= 1){
                (*col)--;
            }
            break;

        case KEY_RIGHT:
            if(*col <= WIDTH-2){
                (*col)++;
            }
            break;

        case KEY_Enter:
            if(board[*row][*col] == 'O' || board[*row][*col] == 'X')
                break;
            if(board[*row][*col] == 'O' || board[*row][*col] == 'Y')
                break;
            if(board[*row][*col] == 'Y' || board[*row][*col] == 'X')
                break;

            if (players == 2){
                if((*turn)%2 == 0)
                    board[*row][*col] = 'O';

                else
                    board[*row][*col] = 'X';

                (*turn)++;
            }
            
            if (players == 3){
            
                if((*turn) % 3 == 0)
                    board[*row][*col] = 'O';

                else if((*turn)%3 == 1)
                    board[*row][*col] = 'X';
                
                else if((*turn)%3 == 2)
                    board[*row][*col] = 'Y';

            (*turn)++;
            
            }

            if ( checkWin (*row, *col, board, players) == 1)
                return 1;
            else
            return 0;
    }
    return 0;
} 
    


void gameStart(WINDOW *win, int load, int players,char save_name[256]){
	int **board;
	int row = 0;
	int col = 0;
	int keyin;
    int turn= 0;


    char exit = '0';
    char t;
    int a;
    int number;

    if(load == 'n')
	    board = initBoard(board, &row, &col, &turn, load);
    
    else if (load == 'y')
        board = readSavedGame(board, &row, &col, &turn, save_name, &players);
    
    

	while(1){
		
        if (turn % players== 0)
            t = 'O';
        else if(turn % players == 1)
            t = 'X';
        else
            t = 'Y';

        paintBoard(board, win, row, col);
        move(HEIGHT+10, 8);
        printw("Current Turn : %c", t);
        move(HEIGHT+11, 8);
        printw("1. Press 1 to save");
        move(HEIGHT+12, 8);
        printw("2. Exit without save");
        
        if (load == 'y'){
        move(1, 1);
        printw("%s", save_name);
        }
    
        wmove(win, row, col);
        refresh();
        wrefresh(win);
          
        int aaaa;
        aaaa = 0;
        keyin = getch();
        if(keyin == '\n') { 
            aaaa = Action(win, board, keyin, &row, &col, &turn, players, &exit);
        }
        else if(keyin == '1'){
            saveGame(board, row, col, turn, players);
            return;
        }
        else if(keyin == '2'){
            mvprintw(HEIGHT+13,8,"Really?? (y/n) ");
            scanw("%c", &exit);
            if (exit == 'y'){
                break;
            }
            else if(exit=='n')
                continue;
        }
        else {            
            Action(win, board, keyin, &row,&col,&turn, players, &exit);
        }

        if ( t == 'O')
            number = 1;
        else if ( t == 'X')
            number = 2;
        else
            number = 3;
        move(HEIGHT+9,8);
        if (aaaa) {

            paintBoard(board,win, row, col);
            mvprintw(HEIGHT+9,8, "PLAYER%d WIN !! Press any button to terminate the program", number);
        break;
        }
	}
	return;
}

int main(){
	char load, save_name[256];
	int players;	
    FILE* read;
    int i,j,k; 

    printf("Want to load the game?[y/n] : ");
    scanf("%c", &load);

    
    if (load == 'y'){
    
    printf("ENTER FILE NAME : ");
    scanf("%s", save_name);

    read = fopen(save_name, "r");
    fscanf(read, "%d %d %d %d %d %d", &i, &j, &k,&HEIGHT,&WIDTH, &players);
    fclose(read);
    
    initscr();
    
    WINDOW *win = newwin(HEIGHT, WIDTH, 5, 5);  
	
    refresh(); 
   
    move(1, 1);
    printw("%s", save_name);

    keypad(stdscr, true);
	gameStart(win, load, players,save_name);

    getch();
    endwin();

    }
    
    else if (load == 'n'){
    printf("Enter the HEIGHT of the board : ");
    scanf("%d", &HEIGHT);

    printf("Enter the WIDTH of the board : ");
    scanf("%d", &WIDTH);

    printf("Enter the number of players[2/3] : ");
    scanf("%d", &players);   

    initscr();
    WINDOW *win = newwin(HEIGHT, WIDTH, 5, 5);  
	
    refresh(); 
   
    keypad(stdscr, true);
	gameStart(win,load,players,save_name); 
    getch();
    endwin();
    }

    return 0;

    }   


