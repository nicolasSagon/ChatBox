#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define TAILLEDATA = 140;

struct chat_message
{
	struct Header header;
	char data[TAILLEDATA];
};

struct Header
{
	char commande;
	int idUtilisateur;
	int timestamp;
	char idSalon;
	int taille;
	int numMessage;
};

enum Commande {

	CONNECT = 0,
	JOIN = 1,
	SAY = 2,
	LEAVE = 3,
	DISCONECT = 4,
	ACK = 5,
	ALIVE = 6,
	MESSAGE_SERVER = 7
};

