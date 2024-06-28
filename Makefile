CC = g++

CompileParms = -c -Wall -std=c++17 -O2

OBJS = main.o odeSystem.o digitalSimulator.o FPAAParser.o expression.o

Opdr: $(OBJS)
	$(CC) $(OBJS) -o compiler

clean:
	rm -f *.o compiler

expression.o: src/expression.cpp src/include/expression.h
	$(CC) $(CompileParms) src/expression.cpp 

odeSystem.o: src/odeSystem.cpp src/include/odeSystem.h
	$(CC) $(CompileParms) src/odeSystem.cpp

digitalSimulator.o: src/digitalSimulator.cpp src/include/odeSystem.h
	$(CC) $(CompileParms) src/digitalSimulator.cpp

FPAAParser.o: src/FPAAParser.cpp src/include/odeSystem.h
	$(CC) $(CompileParms) src/FPAAParser.cpp

main.o: src/main.cpp src/include/odeSystem.h
	$(CC) $(CompileParms) src/main.cpp