all:
	gcc -o bin/router.out src/main.c src/receiver.c src/writer.c src/sender.c src/routing_table.c -I include/ -lpthread
