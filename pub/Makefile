all:remove

create:
	gcc -c -o sql_database.o ../database/sql_database.c -lmysqlclient

fun_file:create
	gcc -shared -fPIC -o libsql_database.so sql_database.o

main:fun_file
	gcc -o pub publisher.c -lpthread -lncurses -lsql_database -lmysqlclient -L $(shell pwd)

execute:main
	./pub
        
remove:execute
	rm -rf sql_database.o
	rm -rf sql_database.so
	rm -rf libsql_database.so
	rm -rf pub

man_rem:
	rm -rf sql_database.o
	rm -rf sql_database.so
	rm -rf libsql_database.so
	rm -rf pub
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
	
	
	

