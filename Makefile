server:
	gcc server.c microsleep.c -pthread -Wall -o Server

client:
	gcc client.c -pthread -Wall -o Client

all : client server

clean:
	rm Client
	rm Server