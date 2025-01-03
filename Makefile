all: client server

client: client.c
		gcc client.c -o Client

server: server.c
		gcc server.c -o Server

clean:
		rm Client
		rm Server