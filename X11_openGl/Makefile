all:
	ctags -R .
	cat Makefile
run:
	ctags -R .
	g++ a.one.cpp -lX11 -lGL -lGLU -std=c++17
	./a.out
	rm a.out
mrun:
	ctags -R .
	g++ a.one.cpp -lX11 -lGL -lGLU -std=c++17
	vblank_mode=0 ./a.out
	rm a.out
debug:
	ctags -R .
	g++ -g -lm a.one.cpp -lX11 -lGL -lGLU -std=c++17
	gdb ./a.out
	rm a.out
