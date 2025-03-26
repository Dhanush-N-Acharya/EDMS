#include<stdio.h>
#include<mysql/mysql.h>
#include<string.h>
#include<termios.h>
#include<unistd.h> 
#include<sys/ioctl.h> 

#define NUM_OPTIONS1 4

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void moveCursorTo(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

void printMenu1(int selectedOption, int numRows, int numCols,char *who) {
    char options1[NUM_OPTIONS1][100] = {"INSERT DATA","DELETE DATA"," PRINT DATA","  GO BACK  "};
    int menuRow = numRows / 2 - NUM_OPTIONS1 / 2;
    int maxOptionLength = 0;
    int arr[4] = {41,45,46,42};
    // Find the length of the longest option
    for (int i = 0; i < NUM_OPTIONS1; ++i) {
        int optionLength = strlen(options1[i]);
        if (optionLength > maxOptionLength) {
            maxOptionLength = optionLength;
        }
    }

    int menuCol = numCols / 2 - (maxOptionLength + 4) / 2; // Adjust to center based on the width of the options
	moveCursorTo(menuRow-5, menuCol-14);
    printf("\x1b[2;30;42m%s\x1b[0m\n",who);
    moveCursorTo(menuRow, menuCol);
    printf("Select an option:\n"); 
    for (int i = 0; i < NUM_OPTIONS1; ++i) {
        moveCursorTo(menuRow + i + 1, menuCol);
        if (i == selectedOption) {
            printf("\x1b[2;30;%dm  %s  \x1b[0m\n",arr[i], options1[i]);
        } else {
            printf("  %s  \n", options1[i]);
        }
    }
}

int sql_database(char* db_name,char* who) 
{
	//enableRawMode();
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char *server = "localhost";
	char *user = "root";
	char *password = "1234";
	char database[20];
	int id;
	int emp_id;
	int choice = 0;
    	int totalOptions = NUM_OPTIONS1;
	
    	struct winsize w;
    	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    	
	strcpy(database,db_name);

	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) 
	{
		printf("Error connecting to database: %s\n", mysql_error(conn));
	}
	
	int empid;
	char name[50];
	char phone[20];
	char email[20];
	char blood[5];	
	char sql_query[200];
	while (1) {
        system("clear");
        printMenu1(choice, w.ws_row, w.ws_col,who);

        // Read user input
        char input;
        if (read(STDIN_FILENO, &input, 1) != 1) {
            perror("read");
            exit(1);
        }
        // Process user input
        if (input == '\033') { // Check if arrow key is pressed
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;
		//disableRawMode();
            if (seq[0] == '[') {
                if (seq[1] == 'A' && choice > 0) { // Up arrow
                    choice--;
                } else if (seq[1] == 'B' && choice < totalOptions - 1) { // Down arrow
                    choice++;
                }
            }
        } else if (input == '\n') { // Enter key
            // Perform action based on selected option
            switch (choice) {
                case 0:
                	disableRawMode();
			//scanf("%*c%*c%*c");	
			printf("ENTER EMPID:\n");
			scanf("%d", &empid);
			conn = mysql_init(NULL);

			if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) 
			{
				printf("Error connecting to database: %s\n", mysql_error(conn));
			}
			sprintf(sql_query, "SELECT * FROM employees WHERE empid = %d",empid);
			mysql_query(conn, sql_query);
			res = mysql_use_result(conn);
			if((row = mysql_fetch_row(res)) == NULL)
			{
				printf("ENTER NAME:\n");
				scanf("%50s",name);

				printf("ENTER PHONE NUMBER:\n");
				scanf("%13s",phone);

				printf("ENTER EMAIL ID:\n");
				scanf("%s",email);

				printf("ENTER BLOOD GROUP:\n");
				scanf("%s",blood);

				sprintf(sql_query, "INSERT INTO employees (empid, name, phone, email, blood) VALUES ('%d', '%s','%s','%s','%s')", empid, name, phone, email, blood);

				mysql_query(conn, sql_query);

				printf("DATA INSERTED.\n");
				scanf("%*c");
				enableRawMode();
				sleep(2);
				system("clear");
			}
			else
			{
				enableRawMode();
				printf("DATA EXISTS\n");
				sleep(2);
				system("clear");
			}
			break;
                case 1:
                	disableRawMode();
			printf("ENTER THE EMPID TO BE DELETED:\n");
			scanf("%d", &emp_id);
			sprintf(sql_query, "DELETE FROM employees WHERE empid='%d'", emp_id);
			mysql_query(conn, sql_query);	
			printf("ROW DELETED.\n");
			enableRawMode();
			sleep(2);
			system("clear");
                    	break;
                case 2:
                	system("clear");
                	moveCursorTo(0,0);
                	sprintf(sql_query, "SELECT * FROM employees");
			mysql_query(conn, sql_query);
			res = mysql_use_result(conn);
			printf("--------------------------------------------------------\n");
			while ((row = mysql_fetch_row(res)) != NULL) 
			{
				printf("%-5s%-8s%-15s%-20s%s\n", row[0], row[1], row[2], row[3], row[4]);
			}
			printf("--------------------------------------------------------\n");
			printf("PRESS \x1b[2;30;41mESC\x1b[0m TO BACK\n");
			read(STDIN_FILENO, &input, 1); 
			if(input == '\033')
			{
				break;
			}
                    	break;
                case 3:
                	system("clear");
			return 0;
            }
        }
    }
	//disableRawMode();
	return 0;
	
}

