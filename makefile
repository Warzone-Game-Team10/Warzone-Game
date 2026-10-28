output: MainDriver.o MapDriver.o Map.o CardsDriver.o Cards.o Player.o PlayerDriver.o Orders.o OrdersDriver.o GameEngineDriver.o GameEngine.o CommandProcessing.o CommandProcessingDriver.o
	g++ MainDriver.o MapDriver.o Map.o CardsDriver.o Cards.o Player.o PlayerDriver.o Orders.o OrdersDriver.o GameEngineDriver.o GameEngine.o CommandProcessing.o CommandProcessingDriver.o -o warzone-tests.exe

MainDriver.o: src/MainDriver.cpp include/MapDriver.h include/CardsDriver.h include/PlayerDriver.h include/OrdersDriver.h include/GameEngineDriver.h include/CommandProcessingDriver.h
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

PlayerDriver.o: src/PlayerDriver.cpp include/Player.h
	g++ -std=c++20 -Iinclude -c src/PlayerDriver.cpp -o PlayerDriver.o

Orders.o: src/Orders.cpp include/Orders.h
	g++ -std=c++20 -Iinclude -c src/Orders.cpp -o Orders.o

OrdersDriver.o: src/OrdersDriver.cpp include/OrdersDriver.h include/Orders.h
	g++ -std=c++20 -Iinclude -c src/OrdersDriver.cpp -o OrdersDriver.o

GameEngineDriver.o: src/GameEngineDriver.cpp include/GameEngineDriver.h include/GameEngine.h include/CommandProcessing.h
	g++ -std=c++20 -Iinclude -c src/GameEngineDriver.cpp -o GameEngineDriver.o

GameEngine.o: src/GameEngine.cpp include/GameEngine.h include/CommandProcessing.h
	g++ -std=c++20 -Iinclude -c src/GameEngine.cpp -o GameEngine.o

CommandProcessing.o: src/CommandProcessing.cpp include/CommandProcessing.h
	g++ -std=c++20 -Iinclude -c src/CommandProcessing.cpp -o CommandProcessing.o

CommandProcessingDriver.o: src/CommandProcessingDriver.cpp include/CommandProcessingDriver.h include/CommandProcessing.h include/GameEngine.h include/GameEngineDriver.h
	g++ -std=c++20 -Iinclude -c src/CommandProcessingDriver.cpp -o CommandProcessingDriver.o

run: output
	./warzone-tests.exe all

run-engine: output
	./warzone-tests.exe gameengine

clean:
	del /Q *.o warzone-tests.exe 2>NUL

.PHONY: run run-engine clean
