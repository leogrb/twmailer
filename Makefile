all: client server

client:
	gcc -Wall myclient.c -o myclient

server:
	gcc -Wall myserver.c -o myserver

clean:
	rm -f server client