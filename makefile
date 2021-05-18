all: TreeBase/Tree.cpp TreeBase/Stack.cpp Frontend Backend Compiler

Frontend: Frontend/Frontend.cpp
	g++ Frontend/Frontend.cpp TreeBase/Tree.cpp TreeBase/Stack.cpp -Wall -o Frontend.out

Backend: Backend/Backend.cpp
	g++ Backend/Backend.cpp TreeBase/Tree.cpp TreeBase/Stack.cpp -Wall -o Backend.out

Compiler: Compiler.cpp
	g++ Compiler.cpp -Wall -o Compiler.out
