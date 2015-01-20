CFLAGS = -Wall -Wextra -O2 -g -lpthread
all: ClientChatBox ServeurChatBox ClientFunctions
ClientChatBox: ClientChatBox.o
	gcc $^ -o $@
ServeurChatBox: ServeurChatBox.o 
	gcc $^ -o $@
ClientFunctions: ClientFunctions.o 
	gcc $^ -o $@
ClientChatBox.o ServeurChatBox.o ClientFunctions.o : protocole.h ClientFunctions.h
clean:
	@rm *.o ClientChatBox ServeurChatBox
