.PHONY: clean

RM = rm -f

server: server-interface2.c mysqldb.c mysqldb.h functions.c functions.h
	gcc -o server server-interface2.c mysqldb.c functions.c -I/usr/include/mysql -lmysqlclient -L/usr/lib -lcurses -lpthread

client: client-interface.c mysqldb.c mysqldb.h functions.c functions.h
	gcc -o client client-interface.c mysqldb.c functions.c -I/usr/include/mysql -lmysqlclient -L/usr/lib -lcurses -lpthread

##mysqldb.o: mysqldb.c mysqldb.h
##	gcc -o mysqldb.o mysqldb.c -I/usr/include/mysql -lmysqlclient -L/usr/lib -lcurses -lpthread

##functions.o: functions.c functions.h
##	gcc -o functions.o functions.c -I/usr/include/mysql -lmysqlclient -L/usr/lib -lcurses -lpthread

clean:
	$(RM) *.o