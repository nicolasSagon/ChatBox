#ifndef PROTOCOLE
#define PROTOCOLE

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define TAILLEDATA 140

struct Header
{
	char commande;
	int idUtilisateur;
	int timestamp;
	char idSalon;
	int taille;
	int numMessage;
};

struct Chat_message
{
	struct Header header;
	char data[TAILLEDATA];
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

struct User {

	int id;
	char name[50];
	int sd;
	struct User *userNext;
};

struct ListUser {
	struct User *first;
};
struct Room {

	int id;
	char name[50];
	int  idUser[10];
	struct Room *roomNext;
};

struct ListRoom {
	struct Room *first;
};



#endif