#ifndef SERVERCHATBOX
#define SERVERCHATBOX

#include "protocole.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVER_PORT 1500
#define MAX_MSG 80

struct ListUser *listUser;
int id=1;
int idRoom=1;
struct ListUser *initalization(){
	struct ListUser *listUser= malloc(sizeof(*listUser));
	struct User *user= malloc(sizeof(*user));
	user->id=0;
	strcpy(user->name,"");
	user->userNext=NULL;
	user->sd=0;
	
	listUser->first=user;
	return listUser;
}

void addUser(int sd, char* name){
	struct User *user= malloc(sizeof(*user));
	user->id=id;
	strcpy(user->name,name);
	user->sd=sd;
	user->userNext=listUser->first;
	listUser->first=user;
	id++;
}

void deleteUser(int id){

	struct User *user=listUser->first;
	struct User *userTmp = NULL;
	int find=0;
	
	while (user->userNext != NULL && find==0){
		if (user->id==id){
			find=1;
			printf("User delete %s\n",user->name);
		}
		else {
			userTmp=user;
			user=user->userNext;
		}
	}
	if (find==1){
		if (userTmp==NULL){
			printf("debut tmp \n");
			listUser->first=user->userNext;
		}
		else{
			printf("debug\n");
			userTmp->userNext=user->userNext;
		}
		free(user);
	}
}

void displayList(){
	struct User *user=listUser->first;
	while (user->userNext != NULL ){
		printf("User name : %s id %d\n", user->name, user->id);
		user=user->userNext;
	}
}

struct ListRoom *listRoom;
struct ListRoom *initalizationRoom(){
	struct ListRoom *listRoom= malloc(sizeof(*listRoom));
	struct Room *room= malloc(sizeof(*room));
	room->id=0;
	strcpy(room->name,"");
	room->roomNext=NULL;
	room->idUser[0]=1;
	listRoom->first=room;
	return listRoom;
}

struct Room addRoom(char* name){
	struct Room *room= malloc(sizeof(*room));
	room->id=idRoom;
	strcpy(room->name,name);
	room->roomNext=listRoom->first;
	listRoom->first=room;
	idRoom++;
	return *room;
}

void deleteRoom(int id){

	struct Room *room=listRoom->first;
	struct Room *roomTmp = NULL;
	int find=0;
	
	while (room->roomNext != NULL && find==0){
		if (room->id==id){
			find=1;
			printf("User delete %s\n",room->name);
		}
		else {
			roomTmp=room;
			room=room->roomNext;
		}
	}
	if (find==1){
		if (roomTmp==NULL){
			printf("debut tmp \n");
			listRoom->first=room->roomNext;
		}
		else{
			printf("debug\n");
			roomTmp->roomNext=room->roomNext;
		}
		free(room);
	}
}

void displayListRoom(){
	struct Room *room=listRoom->first;
	while (room->roomNext != NULL ){
		printf("room name : %s id %d\n", room->name, room->id);
		room=room->roomNext;
	}
}

void decripteHeader(struct Chat_message messageRecu,int sd){
	switch  (messageRecu.header.commande){
	
	case CONNECT:
		connectServer(sd,messageRecu.data);
	break;
	
	case JOIN:
		join(messageRecu);
	break;
	
	case SAY:
		say(messageRecu);
	break;
	
	case LEAVE:
		leave(messageRecu.header);
	break;
	
	case DISCONECT:
		disconnectServer(messageRecu.header);
	break;
	
	case ACK:
		ack(messageRecu);
	break;
	
	case ALIVE:
		alive(messageRecu.header);
	break;

	}
}

void connectServer(int sd, char* userName){
	struct User *user=listUser->first;
	struct User *userTmp;
	int find=0;
	while (user->userNext != NULL && find==0){
		if (user->sd==sd){
			find=1;
		}
		else {
			userTmp=user;
			user=user->userNext;
		}
	}
	if (find==0){
		addUser(sd,userName);
		displayList();
	}	
}

void disconnectServer(struct Header header){
	printf("delete id %d\n", header.idUtilisateur);
	deleteUser(header.idUtilisateur);
	
	displayList();
}

void say(struct Chat_message messageRecu){

	
}

void join(struct Chat_message messageRecu){
	//recherche si le salon recut est existe sinon on le crée
	printf("join to :%s\n", messageRecu.data);
	struct Room *room=listRoom->first;
	int find=0,findUser=0, idVid=99;
	struct Room newRoom;
	if (room->roomNext != NULL){
		while (room->roomNext != NULL && find==0){
			printf("boucle while\n");
			if (strcmp (room->name, messageRecu.data) == 0){
				findUser=1;
				printf("salon deja crée\n");
				//recherche si l'utilisateur est deja dans le salon
				int i;
				for (i=0;i<10;i++){
					if(room->idUser[i]==messageRecu.header.idUtilisateur){
						findUser=1;
						printf("Ajout de l'utilisateur au salon %s imposible\n",messageRecu.data);
					}
					if(room->idUser[i]==NULL){
						idVid=i;
					}
				}
				if (findUser==0 && idVid != 99){
					//ajout de l'utilisateur dans le salon
					room->idUser[idVid]=messageRecu.header.idUtilisateur;
				}
				find=1;
			}
			else {
				room=room->roomNext;
			}
		}
		if (find==0){
			printf("salon non crée\n");
			newRoom=addRoom(messageRecu.data);
			newRoom.idUser[0]=messageRecu.header.idUtilisateur;
			find=1;
		}
	}
	else{
		printf("salon non crée\n");
		newRoom=addRoom(messageRecu.data);
		newRoom.idUser[0]=messageRecu.header.idUtilisateur;
	}
	displayListRoom();

}

void leave(struct Header header ){

}

void alive(struct Header header){

}

void ack(struct Chat_message messageRecu){

}

int main(void)
{

  listUser = initalization();
  listRoom =initalizationRoom();
  int sd, n;
  socklen_t addr_len;
  struct sockaddr_in client_addr, server_addr;
  struct Chat_message messageRecu;
  // Create socket
  if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("socket creation");
    return 1;
  }
  // Bind it
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERVER_PORT);
  if (bind(sd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1)
  {
    perror("bind");
    return 1;
  }
  for (;;)
  {
    addr_len = sizeof(client_addr);
    n = recvfrom(sd, &messageRecu, MAX_MSG, 0,
        (struct sockaddr *)&client_addr, &addr_len);
    if (n == -1)
      perror("recvfrom");
    else {
      printf("received from %s: %d\n",
          inet_ntoa(client_addr.sin_addr), messageRecu.header.commande);
      decripteHeader(messageRecu, sd);
    }
  }
  return 0;

  
}

#endif
