server:
	gcc server.c microsleep.c -pthread -o Server

client:
	gcc client.c -pthread -o Client

all : client server

clean:
	rm Client
	rm Server