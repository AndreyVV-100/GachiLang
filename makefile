all: Tree/Stack.cpp Tree/Tree.cpp Backend/Backend.cpp
	g++ Compiler.cpp Tree/Stack.cpp Tree/Tree.cpp Backend/Backend.cpp -Wall
