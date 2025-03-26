#include<stdlib.h>
#include<stdio.h>  
#include<string.h>  
#include<unistd.h>  
#include<sys/types.h>  
#include<sys/ipc.h>  
#include<sys/msg.h>  
#include<mysql/mysql.h>
#include<signal.h>
#include<termios.h>
#include<errno.h>
#include<sys/ioctl.h> 
#include <pthread.h>
#include <stdbool.h>
#include <ncurses.h>

//#define MSG_SIZE 6
#define NUM_OPTIONS 3
#define MSG_TYPE_SENDER 1
//#define MSG_TYPE 2

struct my_msg
{  
	long int msg_type; 
	char empid[5];
	char name[20];
	char phone[15];
	char email[50];
	char blood[5];	
	char str[5]; 
}some_data; 

/*struct msgbuf {
    long mtype;
    char mtext[MSG_SIZE];
}message;*/

struct sigaction alrm;
int sql_database(char* db_name,char *who);
int loop_end=1;
char line[]  = "RECEIVING***********";
char line2[] = "SENDING ACK*********";
char *who = "EMPLOYEE DATABASE MANAGEMENT SYSTEM SERVER";

void enableRawMode() 
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() 
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void moveCursorTo(int row, int col) 
{
    printf("\033[%d;%dH", row, col);
}

void getCursorPosition(int *row, int *col) 
{
    struct termios orig_attr, new_attr;
    tcgetattr(STDOUT_FILENO, &orig_attr);

    new_attr = orig_attr;
    new_attr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &new_attr);

    printf("\033[6n");
    fflush(stdout); 

    scanf("\033[%d;%dR", row, col);

    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &orig_attr);
}

void printMenu(int selectedOption, int numRows, int numCols) 
{
	char options[3][100] = {"ACCESS DATABASE","  RECEIVE DATA ","      EXIT     "};
	printf("\e[?25l");
	int menuRow = numRows / 2 - NUM_OPTIONS / 2;
	int menuCol = numCols / 2 - 10; 
	int arr[3] = {41,45,42};
	moveCursorTo(menuRow-5, menuCol-12);
    	printf("\x1b[2;30;42mEMPLOYEE DATABASE MANAGEMENT SYSTEM SERVER\x1b[0m\n");
   	moveCursorTo(menuRow, menuCol);
	printf("Select an option:\n");
	for (int i = 0; i < NUM_OPTIONS; ++i) 
	{
		moveCursorTo(menuRow + i + 1, menuCol);
		if (i == selectedOption) 
		{
			printf("\x1b[2;30;%dm  %s  \x1b[0m\n",arr[i], options[i]);
		} 
		else 
		{
		 	printf("  %s  \n", options[i]);
		}
	}
	//moveCursorTo(menuRow+16, menuCol-4);
    	//printf("\x1b[2;30;mCopyright (c) 2024 Dhanush N\x1b[0m\n");
	printf("\e[?25h");
}

void handle_alarm(int sig)
{
     loop_end=0;
}

void running_font(char* str, int ctrl)
{
	int len = strlen(str);
	while(1)
	{
		for (int i = 0; i < len; i++) 
		{
			int w=i+160;
			printf("\033[38;5;%dm%c\033[0m", w,str[i]);
			fflush(stdout);  
			usleep(100000);  
		}

		for (int i = len-1; i >= 0; i--) 
		{
			printf("\b \b");  
			fflush(stdout);   
			usleep(100000);   
		}
		
		if(ctrl==1)
		{
			alarm(1);
		}
		
		if(loop_end==0)
		{
			loop_end==1;
			break;
		}
	}
	
}

bool validateLogin(const char *username, const char *password) 
{
	const char *validUsername = "admin";
	const char *validPassword = "1234";

	return (strcmp(username, validUsername) == 0 && strcmp(password, validPassword) == 0);
}

void moveToCenter(int row, int col, const char *message) 
{
	move(row / 2, (col - strlen(message)) / 2);
}

void *login(void *arg)
{
	char username[50];
	char password[50];
	int row, col;
	int *val = malloc(sizeof(int));
	*val = 1;
	getmaxyx(stdscr, row, col);
	moveToCenter(row-1, col-1, "Welcome to the Admin Login Application");
	printw("Welcome to the Admin Login Application\n");
	refresh();

	do 
	{
		moveToCenter(row+1, col+1, "Enter username: ");
		printw("Enter username: ");
		refresh();
		echo();
		getstr(username);
		noecho();
		moveToCenter(row+2, col+1, "Enter password: ");
		printw("Enter password: ");
		refresh();
		getstr(password);

		if (validateLogin(username, password)) 
		{
			pthread_exit(val);
		} 
		else 
		{
			clear();
			moveToCenter(row-1, col-1, "Invalid username or password. Please try again.");
			printw("Invalid username or password. Please try again.\n\n");
			refresh();
		}
	} while (true);
	return 0;
}

void *access_database(void *arg)
{
	system("clear");
	sql_database("Side",who);	
}

int main()  
{   
	int rc;
    	pthread_t tid;
    	pthread_t threads[NUM_OPTIONS];
    	//int val = 1;
    	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	void *val;
	rc = pthread_create(&threads[1], NULL, login, NULL);
	if (rc) 
	{
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(EXIT_FAILURE);
	}
	pthread_join(threads[1], &val);
	endwin();
	int *result = (int*)val;
	//int *result = &val;
	if(*result == 1)
	{
	enableRawMode();
	int msgid,msgid2;    
	char data[100];
	int flag[6]={0,0,0,0,0,0};
    	int len = strlen(line);
    	int pid;
    	int choice = 0;
    	int totalOptions = 3;
    	int row1,col1;
    	int err;
    	struct winsize w;
    	
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char *server = "localhost";
	char *user = "root";
	char *password = "1234";
	char *database = "Side";
	char input1;
	char buffer[6*sizeof(int)];
	system("clear");
	msgid=msgget((key_t)1234,0666|IPC_CREAT); 
	msgid2=msgget((key_t)1500,0666|IPC_CREAT);
	
	if (msgid == -1)
	{  
		printf("ERROR CREATING THE QUEUE\n"); 
		disableRawMode(); 
		exit(0);  
	} 
		
	if (msgid2 == -1)
	{  
		printf("ERROR CREATING THE QUEUE\n"); 
		disableRawMode(); 
		exit(0);  
	} 
		
	while (1) 
	{
make:
        system("clear");
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        printMenu(choice, w.ws_row, w.ws_col);
        if (read(STDIN_FILENO, &input1, 1) != 1) 
        {
		perror("read");
		disableRawMode();
		exit(1);
        }

        if (input1 == '\033') 
        { 
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1)!= 1) continue;
		if (read(STDIN_FILENO, &seq[1], 1)!= 1) continue;

		if (seq[0] == '[') 
		{
			if (seq[1] == 'A' && choice > 0) 
			{ 
				choice--;
			} 
			else if (seq[1] == 'B' && choice < totalOptions - 1) 
			{ 
				choice++;
			}
		}
        } 
        else if (input1 == '\n') 
        {
        	system("clear");
		switch (choice) 
		{
			case 0:
				printf("\e[?25l");
				rc = pthread_create(&threads[0], NULL, access_database, NULL);
				if (rc) 
				{
					printf("ERROR; return code from pthread_create() is %d\n", rc);
					exit(EXIT_FAILURE);
				}
				pthread_join(threads[0], NULL);
				printf("\e[?25h");
				goto make;
				break;
			case 1:
				alrm.sa_handler=handle_alarm;
				alrm.sa_flags=SA_RESTART;
				sigaction(SIGALRM,&alrm,NULL);
				
				conn = mysql_init(NULL);

				if (!mysql_real_connect(conn, server, user, password, database, 0,NULL, 0)) 
				{
					fprintf(stderr, "%s\n", mysql_error(conn));
					disableRawMode();
					exit(1);
				}
				system("clear");
				
				start:
				if(msgrcv(msgid,&some_data,sizeof(some_data),MSG_TYPE_SENDER,0)==-1)
				{
					printf("hello %d\n",errno);
					getchar();
					if(errno == EINTR)
					{
						printf("error\n");
						getchar();
						//goto make;
						goto start;
					}
				} 
				
				pid = fork();

				if(pid==0)
				{
					system("clear");
					running_font(line,0);
				}
				else
				{
					sleep(3);
					kill(pid, SIGTERM);		
				}

				sprintf(data,"SELECT * FROM employees WHERE empid = %s",some_data.empid);
				mysql_query(conn, data);
				res = mysql_store_result(conn);
				row = mysql_fetch_row(res);

				if(row == NULL)
				{
					system("clear");
					printf("-----------------------------\n");
					printf("\033[0;34mDATABASE IS EMPTY...\n\033[0m");
					printf("-----------------------------\n");
					flag[0]=3;
					//memcpy(buffer, flag, 6 * sizeof(int));
					//message.mtype = MSG_TYPE;
					//memcpy(message.mtext, buffer, 6 * sizeof(int));
					msgsnd(msgid2,(void*)&flag,sizeof(flag),0);
					goto start;
				}

				flag[1]=strcmp(some_data.empid,row[0]);
				flag[2]=strcmp(some_data.name,row[1]);
				flag[3]=strcmp(some_data.phone,row[2]);
				flag[4]=strcmp(some_data.email,row[3]);
				flag[5]=strcmp(some_data.blood,row[4]);

				if(!(flag[1] || flag[2] || flag[3] || flag[4] || flag[5]))
				{
					system("clear");

					printf("\033[0;34m\nDATA VERIFICATION...\n\033[0m");
					printf("-----------------------------\n");
					printf("DATA      : %-15s - %s\n","RECEIVER DATA","SENDER DATA");
					printf("ID CHECK  : %-15s - %s\n",row[0],some_data.empid);
					printf("NAME CHECK: %-15s - %s\n",row[1],some_data.name);
					printf("P.NO CHECK: %-15s - %s\n",row[2],some_data.phone);
					printf("MAIL CHECK: %-15s - %s\n",row[3],some_data.email);
					printf("B.G CHECK : %-15s - %s\n",row[4],some_data.blood);
					printf("-----------------------------\n");
					printf("\033[0;32mDATA MATCHED...\033[0m\n");
					flag[0]=1;
					//memcpy(buffer, flag, 6 * sizeof(int));
					//message.mtype = MSG_TYPE;
					//memcpy(message.mtext, buffer, 6 * sizeof(int));
					if(msgsnd(msgid2,(void*)&flag,sizeof(flag),0)==-1)
					{
						printf("FAILED\n");
					}
					printf("-----------------------------\n");
					running_font(line2,1);
					printf("\033[0;32m\nACKNOWLEDGEMENT SENT\n\033[0m");    
					/*enableRawMode();
					printf("PRESS \x1b[2;30;41mENTER\x1b[0m TO CONTINUE \x1b[2;30;41mESC\x1b[0m TO BACK\n");
					if (read(STDIN_FILENO, &input1, 1) != 1) 
        				{
						perror("read");
						exit(1);
        				}
        				if(input1 == '\n')
        				{
						goto start;
					}
					else if(input1 == '\033')
					{
						goto make;
					}
					break;*/
				}
				else
				{
					system("clear");
					printf("\033[0;34m\nDATA VERIFICATION...\n\033[0m");
					printf("-----------------------------\n");
					printf("DATA      : %-15s - %s\n","RECEIVER DATA","SENDER DATA");
					printf("ID CHECK  : %-15s - %s\n",row[0],some_data.empid);
					printf("NAME CHECK: %-15s - %s\n",row[1],some_data.name);
					printf("P.NO CHECK: %-15s - %s\n",row[2],some_data.phone);
					printf("MAIL CHECK: %-15s - %s\n",row[3],some_data.email);
					printf("B.G CHECK : %-15s - %s\n",row[4],some_data.blood);
					printf("-----------------------------\n");
					printf("\033[0;31mDATA MISMATCHED...\n\033[0m");
					flag[0]=2;
					//memcpy(buffer, flag, 6 * sizeof(int));
					//message.mtype = MSG_TYPE;
					//memcpy(message.mtext, buffer, 6 * sizeof(int));
					if(msgsnd(msgid2,(void*)&flag,sizeof(flag),0)==-1)
					{
						printf("FAILED\n");
					}
					printf("-----------------------------\n");
					running_font(line2,1);
					printf("\033[1F\033[0;31m\nNACK SENT\n\033[0m");
					disableRawMode();
					printf("EXIT\n");
					exit(0);  
				}

				//msgctl(msgid,IPC_RMID,0); 
				//msgctl(msgid2,IPC_RMID,0); 
				//mysql_free_result(res);
				//mysql_close(conn);
				break;
			case 2:
				msgctl(msgid,IPC_RMID,0);   
				msgctl(msgid2,IPC_RMID,0);
				disableRawMode();       
				exit(0);
			    	break;
		} 
	}
    }
	disableRawMode();
	}
	return 0;		
}  
