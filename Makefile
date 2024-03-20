#ODE compiler Makefile

CXX = g++
CXXFLAGS = -c -Wall -std=c++17 -O2

OBJS = main.o 

Opdr: $(OBJS)
	$(CXX) $(OBJS) -o compiler

main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) src/main.cpp

clean: 
	rm -f *.o compiler