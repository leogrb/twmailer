all: client server

client:
	gcc -Wall myclient.c -o myclient

server:
	gcc -Wall -pthread myserver.c -o myserver

clean:
	rm -f myserver myclient
