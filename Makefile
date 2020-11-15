all:
	ctags -R .
	cat Makefile
run:
	ctags -R .
	g++ main.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	./a.out
	rm a.out
mrun:
	ctags -R .
	g++ main.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	vblank_mode=0 ./a.out
	rm a.out
debug:
	ctags -R .
	g++ -g -lm main.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	gdb ./a.out
	rm a.out
