all:
	gcc -o router.out src/main.c src/routing_table.c -I include/ -lpthread
