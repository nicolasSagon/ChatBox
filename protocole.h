#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct chat_message
{
	char *command;
	char *data;
	int timestamp;
};


