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
	g++ main.cpp -o app -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	vblank_mode=0 ./app
	rm app
debug:
	ctags -R .
	g++ -g -lm main.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	gdb ./a.out
	rm a.out
t:
	ctags -R .
	g++ main.cpp -o app -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	./app
	vblank_mode=0 ./app
	rm app
git:
	git add .
	git commit
