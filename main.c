
#include<ncurses.h>						//ncurses lib
#include<stdio.h>
#include<stdlib.h>
//====================================================================
typedef struct position{		//this struct store stone's position
	int x;
	int y;
}pos;

typedef struct _stack{			//stack structure
	int x;
	int y;
	struct _stack *next;
}stack;
typedef stack *stackPtr;

//====================================================================
int arr[20][20];			//global array can be used in all functions
stackPtr stack_undo = NULL;		//global stack
stackPtr stack_redo = NULL;
//====================================================================
//function prototypes
void menu(WINDOW *, int *);
void start(WINDOW *);
void option(WINDOW *);
void put_stone(WINDOW *, int, int, char *, int);
int push(stackPtr *, pos);
int undo(WINDOW *);
int redo(WINDOW *, int);
pos pop(stackPtr *);
//====================================================================
int main(void){							
	int x,y,max_x,max_y;
	int highlight = 0;
	int i,j;

	start_color();				//start color
	initscr();				//initialize screen
	noecho();				//no printing character to screen when hit
	curs_set(FALSE);			//remove cursor
	getmaxyx(stdscr,max_y,max_x);		//get the max pos of screen

	WINDOW *win = newwin(20,40,10,30);	//create a win with heigth = 20, width = 40
	refresh();				
	menu(win, &highlight);			//call function menu to print menu and get choice
	
	switch(highlight){			//switch choice(highlight)
		case 0:
		   wclear(win);
		   start(win);
		   wrefresh(win);
		   getch();
		   break;
		case 1:
		   wclear(win);
		   mvwprintw(win,1,1,"Option");
		   wrefresh(win);
		   break;
		default:
		   break;
	        }
	   
	endwin();
}
//====================================================================
void menu(WINDOW *win,int *highlight){
	int x, y, max_x, max_y;
	char str[3][32] = {"1. Start","2. Exit"};
	int i,j,choice;
	
	getmaxyx(win,max_y,max_x);
	keypad(win, true);
	mvwprintw(win,max_y/2-7,max_x/2-2,"GomokU",max_x,max_y);
    mvwprintw(win,max_y/2-5,max_x/2-14,"Contact: adluong11@korea.ac.kr",max_x,max_y);
	while(1){
           for(i = 0; i<2; i++){
            if(i == *highlight)
                wattron(win, A_REVERSE);
            mvwprintw(win,max_y/2-3+i,max_x/2-3,"%s",str[i]);
            wattroff(win, A_REVERSE);
           }//end for
	   wrefresh(win);

           choice = wgetch(win);

           switch(choice){
                case KEY_UP:
                  *highlight = *highlight - 1;
                  if(*highlight < 0) *highlight = 0;
                  break;
                case KEY_DOWN:
                  *highlight = *highlight + 1;
                  if(*highlight > 2) *highlight = 2;
                  break;
                default:
                  break;
           }
           if(choice == 10) break;
          }//end while
}//end menu
//====================================================================
void start(WINDOW *win){
	pos get_move(WINDOW *win);
	int check_vic(int player, int x, int y);
	int x, y, max_x, max_y, i, j, turn = 0;
	pos input;
	
	start_color();
	init_pair(3,COLOR_BLACK,COLOR_WHITE);
	getmaxyx(win,max_y,max_x);
	wattron(win,COLOR_PAIR(3));
	for(i = 0; i <= max_y; i++){
		for(j = 0; j<max_x; j+=2)
			  mvwprintw(win,i,j,". ");
        }
	wattroff(win,COLOR_PAIR(3));
	wmove(win,max_y/2-1,max_x/2);				//move cursur to pos 0,0 of game win
	refresh();
	wrefresh(win);
	while(1){
	  if(turn == 0){			//player 1
                mvprintw(8,76,"Player 2");	//highlight player 1 text
		attron(A_REVERSE);
		mvprintw(8,16,"Player 1");
		attroff(A_REVERSE);
		refresh();
		wrefresh(win);
		
		//get the position of stone
		input = get_move(win);
		if(input.x != -1 && input.x != -2){
			stack_redo = NULL;		//every time putting new stone, clear redo stack
        	        push(&stack_undo,input);

			//check victory condition
			int stt = check_vic(1,input.y,input.x/2);
			if(stt == -1)
                                mvprintw(0,0,"Invalid position. Re-input please!");
			else{
			//put the stone to the map
                        	if(input.x == max_x-1)
					put_stone(win, input.x, input.y, "X", 1);
                       		else
					put_stone(win, input.x, input.y, "X ", 1);
				turn = 1;
			}
			if(stt == 1){
				mvprintw(1,0,"Player 1 is victory!!");
				refresh();
				wrefresh(win);
				break;
			}
			refresh();
			wrefresh(win);
		}//end if

		//case undo:
		else if(input.x == -1){
			int undo_stt = undo(win);
			if(undo_stt == 0)
				turn = 0;
			else
				turn = 1;
		}//end case undo

		//case redo:
		else{
			int redo_stt = redo(win,1);
			if(redo_stt == 0)
                                turn = 0;
                        else
                                turn = 1;

		}//end else
	  }
	  else{
		mvprintw(8,16,"Player 1");
		attron(A_REVERSE);
                mvprintw(8,76,"Player 2");
		attroff(A_REVERSE);
                refresh();
		wrefresh(win);
		
		//get position of stone
		input = get_move(win);
		
		if(input.x != -1 && input.x != -2){
			stack_redo = NULL;			//clear redo stack
			push(&stack_undo,input);		//push input to undo stack
	
               		int stt = check_vic(2,input.y,input.x/2);//check victory condition
                	if(stt == -1)				//if a stone existed in input pos
                      		mvprintw(0,9,"Invalid position. Re-input please!");
			else{
				//Put stone to the map
                        	if(input.x == max_x-1)          //move the stone to pos input.x input.y
					put_stone(win, input.x, input.y, "O", 2);
                        	else
					put_stone(win, input.x, input.y, "O ", 2);
                       		turn = 0;
			}//end else
			if(stt == 2){                           //if player 2 win
                                mvprintw(1,0,"Player 2 is victory!!");
                                break;
                        }//end if
			refresh();
                	wrefresh(win);
		}//end if
		//case undo:
                else if(input.x == -1){
                        int undo_stt = undo(win);
                        if(undo_stt == 0)
                                turn = 1;
                        else
                                turn = 0;
                }//end case undo

                //case redo:
                else{
                        int redo_stt = redo(win,2);
			if(redo_stt == 0)
                                turn = 1;
                        else
                                turn = 0;

                }//end else
	  }//end else
	}//end while
}//end function
//====================================================================
int check_vic(int player, int x, int y){
	if(arr[x][y] != 0)
		return -1;
	else{
	int a,b,c,d;
	int s1,s2,s3,s4,s5,s6,s7,s8;
	s1 = s2 = s3 = s4 = s5 = s6 = s7 = s8 = 1;
	a = b = c = d = 0;
	int step;
	
	arr[x][y] = player;
	for(step = 1; step <= 4; step++){
		(arr[x][y-step] == player && s1 == 1)? a++ : (s1 = 0);
		(arr[x-step][y-step] == player && s2 == 1) ? b++ : (s2 = 0);
		(arr[x-step][y] == player && s3 == 1) ? c++ : (s3 = 0);
		(arr[x+step][y-step] == player && s4 == 1) ? d++ : (s4 = 0);
		(arr[x][y+step] == player && s5 == 1) ? a++ : (s5 = 0);
		(arr[x+step][y+step] == player && s6 == 1) ? b++ : (s6 = 0);
		(arr[x+step][y] == player && s7 == 1) ? c++ : (s7 = 0);
		(arr[x-step][y+step] == player && s8 == 1) ? d++ : (s8 = 0);
	}
	return (a >= 4 || b >= 4 || c >= 4 || d >= 4) ? player : 0;
	}//end else
}//end function
//====================================================================
pos get_move(WINDOW *win){
	pos input;
	int choice, stt = 0;
	int x,y,max_x,max_y;
	curs_set(true);
	//enable keyboard	
	keypad(win,true);
	getyx(win,y,x);
	getmaxyx(win,max_y,max_x);

	while(1){
	choice = wgetch(win);
	
	switch(choice){
	case 'w':
	case KEY_UP:
		y--;
		if(y < 0)
		  y = 0;
		break;
	case 's':
	case KEY_DOWN:
		y++;
		if(y > max_y-1)
		  y = max_y-1;
		break;
	case 'a':
	case KEY_LEFT:
		x-=2;
		if(x < 0)
		  x = 0;
		break;
	case 'd':
	case KEY_RIGHT:
		x+=2;
		if(x > max_x-1)
		  x = max_x-1;
		break;
	default: break;
	}//end switch
	input.x = x;
	input.y = y;
	wmove(win,y,x);
	wrefresh(win);
	if(choice == 10 || choice == 32)
		return input;
	//if input u, undo
	if(choice == 'u' || choice == 'U'){
		input.x = input.y = -1;
		return input;
	}
	//if input r, redo
	if(choice == 'r' || choice == 'R'){
		input.x = input.y = -2;
		return input;
	}
	}//end while
}//end function
//====================================================================
int push(stackPtr *top, pos input){
	stackPtr new_block = (stack*)malloc(sizeof(stack));
	if(new_block != NULL){
		new_block->x = input.x;
		new_block->y = input.y;
		new_block->next = *top;
		*top = new_block;
	return 1;
	}//end if malloc
	else return 0;
}//end case empty
//====================================================================
pos pop(stackPtr *top){
	pos item;
	if(*top == NULL)
		item.x = item.y = -1;
	else{
		stackPtr tmp = *top;
		item.x = (*top)->x;
		item.y = (*top)->y;
		*top = (*top)->next;
		free(tmp);
	}//end else
	return item;
}//end pop
//====================================================================
int undo(WINDOW *win){
	pos tmp = pop(&stack_undo);
	if(tmp.x == -1)
		return 0;
	else{	
		int max_x, max_y;
		getmaxyx(win,max_y,max_x);
		push(&stack_redo,tmp);
		
		//remove stone
		if(tmp.x == max_x-1)
			put_stone(win, tmp.x, tmp.y, ".", 2);
		else
			put_stone(win, tmp.x, tmp.y,". ", 2);
		wrefresh(win);
		//remove input in array
		arr[tmp.y][tmp.x/2] = 0;
		return 1;
	}//end else
}//end undo
//====================================================================
int redo(WINDOW *win, int player){
	pos tmp = pop(&stack_redo);
	mvprintw(3,3,"in function redo stackredo = %x",stack_redo);
	if(tmp.x == -1)
		return 0;
	else{
		int max_x, max_y;
		getmaxyx(win,max_y,max_x);
		push(&stack_undo,tmp);
		//re-print stone
		if(tmp.x == max_x-1){
		  if(player == 1)
			put_stone(win,tmp.x,tmp.y,"X", 1);
		  else
			put_stone(win,tmp.x,tmp.y,"O", 2);
		}
		else{
		  if(player == 1)	
			put_stone(win,tmp.x,tmp.y,"X ", 1);
		  else
			put_stone(win,tmp.x,tmp.y,"O ", 2);
		}
		wrefresh(win);
		//re-add player number to array
		arr[tmp.y][tmp.x/2] = player;
		return 2;
	}//end else
}
//=======================================================================
void put_stone(WINDOW *win, int x, int y, char *c, int stt){
	init_pair(1,COLOR_RED,COLOR_WHITE);
	init_pair(2,COLOR_BLUE,COLOR_WHITE);
	if(stt == 1){
		wattron(win,COLOR_PAIR(1));
		mvwprintw(win,y,x,"%s",c);
		wattroff(win,COLOR_PAIR(1));
	}//end if
	else{
		wattron(win,COLOR_PAIR(2));
                mvwprintw(win,y,x,"%s",c);
		wattroff(win,COLOR_PAIR(2));
	}//end else

}//end put stone
