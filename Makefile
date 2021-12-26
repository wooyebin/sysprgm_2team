all: server client

server.o: server.c
	cc -c server.c
client.o: client.c
	cc -c client.c
server: server.o
	cc -o server server.o -pthread
client: client.o
	cc -o client client.o -pthread -lncurses
clean:
	rm -r *.o
