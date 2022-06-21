all: server client
client: client.cpp
	g++ -pthread -o client client.cpp sockop.c -I ./include/
server: socket_thread.cpp
	g++ -pthread -o server socket_thread.cpp sockop.c -I ./include/
clean:
	rm server client
