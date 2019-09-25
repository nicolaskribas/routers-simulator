all:
	gcc -o bin/router.out src/main.c src/receiver.c src/writer.c src/sender.c -I include/ -lpthread
