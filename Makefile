all:
	gcc -c src/main.c src/receiver.c src/writer.c src/sender.c -I include/ -lpthread
