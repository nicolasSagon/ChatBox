CFLAGS = -Wall -Wextra -O2 -g
all: ClientChatBox ServeurChatBox
ClientChatBox: ClientChatBox.o
	gcc $^ -o $@
ServeurChatBox: ServeurChatBox.o 
	gcc $^ -o $@
ClientChatBox.o ServeurChatBox.o : protocole.h
clean:
	@rm *.o ClientChatBox ServeurChatBox
