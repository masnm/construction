all:
	ctags -R .
	cat Makefile
run:
	ctags -R .
	g++ main.cpp -lX11 -lGL -lpthread -lGLU -std=c++17
	./a.out
	rm a.out
debug:
	ctags -R .
	g++ -g -lm main.cpp -lX11 -lGL -lpthread -lGLU -std=c++17
	gdb ./a.out
	rm a.out
brun:
	ctags -R .
	g++ all.cpp -lX11 -lGL -lpthread -lGLU -std=c++17
	./a.out
	rm a.out
