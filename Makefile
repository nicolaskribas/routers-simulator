all:
	gcc -o bin/router.out src/main.c src/routing_table.c -I include/ -lpthread
