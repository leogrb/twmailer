all: client server

client:
	gcc -Wall myclient.c src/ClientInputHelper.c -o myclient

server:
	gcc -Wall -pthread src/message.c src/ServerInputHelper.c src/LDAPHandler.c myserver.c -llber -lldap -o myserver

clean:
	rm -f myserver myclient
