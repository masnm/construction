all:
	cat Makefile
run:
	g++ main.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	./a.out
	rm a.out
mrun:
	g++ main.cpp -o app -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	vblank_mode=0 ./app
	rm app
debug:
	g++ -g -lm main.cpp -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	gdb ./a.out
	rm a.out
t:
	g++ main.cpp -o app -lX11 -lGL -lpthread -lstdc++fs -std=c++17
	./app
	vblank_mode=0 ./app
	rm app
git:
	git add .
	git commit
