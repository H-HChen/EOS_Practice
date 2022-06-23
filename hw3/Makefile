all: server client
client: client.cpp
	g++ -std=c++2a -pthread -o client client.cpp sockop.c -I ./include/
server: socket_thread.cpp
	g++ -pthread -g socket_thread.cpp -o server sockop.c -I ./include/
clean:
	rm server client
