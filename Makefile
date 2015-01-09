(FLAGS = -Wall -Wextra -O2 -g
all: client serveur
client: client.o
	gcc $^ -o $@
serveur: serveur.o sem.o
	gcc $^ -o $@
client.o serveur.o : protocole.h
clean:
	@rm *.o client serveur
