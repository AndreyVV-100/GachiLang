all: TreeBase/Tree.cpp TreeBase/Stack.cpp Frontend/Frontend.cpp Backend/Backend.cpp
	g++ Frontend/Frontend.cpp TreeBase/Tree.cpp TreeBase/Stack.cpp -Wall -o Frontend.out
	g++ Backend/Backend.cpp TreeBase/Tree.cpp TreeBase/Stack.cpp -Wall -o Backend.out
	g++ Compiler.cpp -Wall -o gcm
