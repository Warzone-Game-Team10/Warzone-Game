
output: MainDriver.o MapDriver.o Map.o CardsDriver.o Cards.o Player.o Orders.o
	g++ MainDriver.o MapDriver.o Map.o CardsDriver.o Cards.o Player.o Orders.o -o warzone.exe

MainDriver.o: src/MainDriver.cpp
	g++ -std=c++20 -Iinclude -c src/MainDriver.cpp -o MainDriver.o

MapDriver.o: src/MapDriver.cpp include/Map.h
	g++ -std=c++20 -Iinclude -c src/MapDriver.cpp -o MapDriver.o

Map.o: src/Map.cpp include/Map.h
	g++ -std=c++20 -Iinclude -c src/Map.cpp -o Map.o

CardsDriver.o: src/CardsDriver.cpp include/Cards.h
	g++ -std=c++20 -Iinclude -c src/CardsDriver.cpp -o CardsDriver.o

Cards.o: src/Cards.cpp include/Cards.h
	g++ -std=c++20 -Iinclude -c src/Cards.cpp -o Cards.o

Player.o: src/Player.cpp include/Player.h
	g++ -std=c++20 -Iinclude -c src/Player.cpp -o Player.o

Orders.o: src/Orders.cpp include/Orders.h
	g++ -std=c++20 -Iinclude -c src/Orders.cpp -o Orders.o

run: output
	./warzone.exe all

clean:
	del /Q *.o warzone.exe 2>NUL

.PHONY: run
