themake: c2.cpp 
	g++ -o out -std=c++11 c2.cpp -lmpfr -lgmp -lpthread -pthread -lcln -lginac; ./out graphs.txt 
