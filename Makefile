CFLAGS = -Wall -Wextra -O2 -g -lpthread
Th_Flag = -lpthread
all: Client Serveur
Client: ClientChatBox.o ClientFunctions.o
	gcc -o $@ $^ $(Th_Flag)
ClientChatBox.o: ClientChatBox.c ClientFunctions.h protocole.h
	gcc -o $@ -c $< $(CFLAGS) 
Serveur: ServeurChatBox.o 
	gcc -o $@ $^ $(Th_Flag)
ServeurChatBox.o: ServeurChatBox.c protocole.h
	gcc -o $@ -c $< $(CFLAGS)
clean:
	@rm *.o Client Serveur
