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
#include <time.h>
#include <pthread.h>
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
}msg; 

/*struct msgbuf {
    long mtype;
    char mtext[MSG_SIZE];
}message1;*/

struct sigaction alrm;
int sql_database(char* db_name,char *who);
int loop_end=1;
char line[]  = "SENDING*************";
char line2[] = "WAITING FOR ACK*****";
char *who = "EMPLOYEE DATABASE MANAGEMENT SYSTEM USER";

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


void printMenu(int selectedOption, int numRows, int numCols) 
{
	char options[3][100] = {"ACCESS DATABASE","   SEND DATA   ","      EXIT     "};
	printf("\e[?25l");
    int menuRow = numRows / 2 - NUM_OPTIONS / 2;
    int menuCol = numCols / 2 - 10; 
    int arr[3] = {41,45,42};

    moveCursorTo(menuRow-5, menuCol-10);
    printf("\x1b[2;30;42mEMPLOYEE DATABASE MANAGEMENT SYSTEM USER\x1b[0m\n");
    moveCursorTo(menuRow, menuCol);
    printf("Select an option:\n");
    for (int i = 0; i < NUM_OPTIONS; ++i) {
        moveCursorTo(menuRow + i + 1, menuCol);
        if (i == selectedOption) {
            printf("\x1b[2;30;%dm  %s  \x1b[0m\n",arr[i], options[i]);
        } else {
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
			alarm(2);
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
	const char *validUsername = "user";
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
	printw("Welcome to the User Login Application\n");
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
	sql_database("Dark",who);	
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
	char buffer[50];
	char data[100];
	int rcv[6];
	int val; 
	int pid; 
	int choice = 0;
    	int totalOptions = 3;
    	
    	struct winsize w;
    	
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char *server = "localhost";
	char *user = "root";
	char *password = "1234";
	char *database = "Dark";
	char input;
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
		
	while (1) {
make:
        system("clear");
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        printMenu(choice, w.ws_row, w.ws_col);
        if (read(STDIN_FILENO, &input, 1) != 1) {
            perror("read");
            disableRawMode();
            exit(1);
        }

        if (input == '\033') 
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
        else if (input == '\n') 
        {
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

			do
			{
			try:
				system("clear");
				disableRawMode();
				printf("\033[0;32mENTER THE EMPLOYEE ID:\n\033[0m");   
				scanf("%[^\n]%*c",buffer); 
				sprintf(data,"SELECT * FROM employees WHERE empid = %s",buffer);
				enableRawMode();
				if (mysql_query(conn, data)) 
				{
					fprintf(stderr, "%s\n", mysql_error(conn));
					disableRawMode();
					exit(1);
				}

				res = mysql_store_result(conn);
				row = mysql_fetch_row(res);
				 
				if(row == NULL)
				{
					printf("WRONG EMPID\n");
					sleep(2);
					goto try;
				}

				if(!strcmp(buffer,row[0]))
				{		
					strcpy(msg.empid,row[0]);
					strcpy(msg.name,row[1]);
					strcpy(msg.phone,row[2]);
					strcpy(msg.email,row[3]);
					strcpy(msg.blood,row[4]);
				}
				else
				{
					disableRawMode();
					exit(1);
				}
				msg.msg_type=MSG_TYPE_SENDER;
				if(msgsnd(msgid,(void *)&msg,sizeof(msg),0)==-1)   
				{  
					printf("MESSAGE NOT SENT\n");  
				}

				if(strncmp(buffer,"end",3)==0)  
				{  
					disableRawMode();
					exit(0);  
				}
				
				pid = fork();
				
				if(pid==0)
				{
					running_font(line,0);
				}
				else
				{
					if(msgrcv(msgid2,(void*)&rcv,sizeof(rcv),0,0)==-1)
					{
						perror("msgrcv");
        					//exit(EXIT_FAILURE);
					}	
					//memcpy(rcv, message1.mtext, 6 * sizeof(int));	
					kill(pid, SIGTERM);
					system("clear");
					printf("\033[0;33mSENT\n\033[0m");
					running_font(line2,1);
					usleep(1000000);
					system("clear");
					
				}
	
				if(rcv[0]==1)
				{
					printf("\033[0;33mACK RECEIVED 0\n\033[0m");
					printf("-----------------------------\n");
					printf("\033[0;32mDATA COMPLETELY MATCHED\n\033[0m");
					printf("\033[0;32mEMPID:%s\n\033[0m",msg.empid);
					printf("\033[0;32mName:%s\n\033[0m",msg.name);
					printf("\033[0;32mPHONE:%s\n\033[0m",msg.phone);
					printf("\033[0;32mEMAIL:%s\n\033[0m",msg.email);
					printf("\033[0;32mBLOOD:%s\n\033[0m",msg.blood);
					printf("-----------------------------\n");
					sleep(1);
				} 
				else if(rcv[0]==2)
				{
					printf("\033[0;33mACK RECEIVED 1\n\033[0m");
					printf("-----------------------------\n");
					printf("\033[0;31mDATA MISMATCHED(mismatched data are shown in red)\n\033[0m");
					rcv[1]!=0?(printf("\033[0;31mEMPID:%s\n\033[0m",msg.empid)):(printf("\033[0;32mEMPID:%s\n\033[0m",msg.empid));
					
					rcv[2]!=0?(printf("\033[0;31mNAME:%s\n\033[0m",msg.name)):(printf("\033[0;32mName:%s\n\033[0m",msg.name));
					
					rcv[3]!=0?(printf("\033[0;31mPHONE:%s\n\033[0m",msg.phone)):(printf("\033[0;32mPHONE:%s\n\033[0m",msg.phone));
					
					rcv[4]!=0?(printf("\033[0;31mEMAIL:%s\n\033[0m",msg.email)):(printf("\033[0;32mEMAIL:%s\n\033[0m",msg.email));
					
					rcv[5]!=0?(printf("\033[0;31mBLOOD:%s\n\033[0m",msg.blood)):(printf("\033[0;32mBLOOD:%s\n\033[0m",msg.blood));
					printf("-----------------------------\n");
					disableRawMode();
					exit(0);	
				}
				else if(rcv[0]==3)
				{
					system("clear");
					printf("-----------------------------\n");
					printf("\033[0;31mENTERED USERID IS INVALID...\n\033[0m");
					printf("-----------------------------\n");
					sleep(2);
					goto try;
				}
				enableRawMode();
				printf("PRESS \x1b[2;30;41mENTER\x1b[0m TO CONTINUE \x1b[2;30;41mESC\x1b[0m TO BACK\n");
				if (read(STDIN_FILENO, &input, 1) != 1) 
        			{
					perror("read");
					exit(1);
        			}
				
			}while(input == '\n' && input != '\033');
			//strcpy(msg.str,"exit");
			//msg.msg_type=MSG_TYPE_SENDER;
			//msgsnd(msgid,(void *)&msg,sizeof(msg),0);
			//msgctl(msgid,IPC_RMID,0);   
			//msgctl(msgid2,IPC_RMID,0);       
			//mysql_free_result(res);
			//mysql_close(conn);
			if(input == '\033')
			{
				goto make;
			}

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





   

 
