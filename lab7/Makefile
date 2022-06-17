all: game guess 
game: game.cpp
	g++ -c game.cpp -I ./include/
	g++ -pthread -L /lib game.o -o game -lrt
	rm game.o
guess: guess.cpp
	g++ -c guess.cpp -I ./include/
	g++ -pthread -L /lib guess.o -o guess -lrt
	rm guess.o
clean:
	rm game guess 
