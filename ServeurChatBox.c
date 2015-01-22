#ifndef SERVERCHATBOX
#define SERVERCHATBOX

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocole.h"
#include <pthread.h>
#include <stdlib.h>
#include "ServeurChatBox.h"


#define SERVER_PORT 1500
#define MAX_MSG 80
#define MAX_USER 50

struct ListUser *listUser;
struct ListRoom *listRoom;
int id=1;
int idRoom=1;
int sd;
int maxUserSalon=10;

struct ListUser *initalization(){
	struct ListUser *listUser= malloc(sizeof(*listUser));
	struct User *user= malloc(sizeof(*user));
	user->id=0;
	strcpy(user->name,"");
	user->userNext=NULL;
	user->port = 0;
	strcpy(user->ipAdress,"");
	
	listUser->first=user;
	return listUser;
}

struct User *addUser(struct sockaddr_in client_addr, char* name){
	struct User *user= malloc(sizeof(*user));
	user->id=id;
	strcpy(user->name,name);
	user->port = ntohs(client_addr.sin_port);
	strcpy(user->ipAdress, inet_ntoa(client_addr.sin_addr));
	user->client_addr = client_addr;
	user->userNext=listUser->first;
	listUser->first=user;
	id++;
	return user;
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
		printf("User name : %s id %d, adress = %s, port = %d \n", user->name, user->id, user->ipAdress, user->port);
		user=user->userNext;
	}
}


struct ListRoom *initalizationRoom(){
	struct ListRoom *listRoom= malloc(sizeof(*listRoom));
	struct Room *room= malloc(sizeof(*room));
	room->id=0;
	strcpy(room->name,"");
	room->numMsg=0;
	room->roomNext=NULL;
	room->idUser[0]=1;
	listRoom->first=room;
	return listRoom;
}

struct Room* addRoom(char* name){
	struct Room *room= malloc(sizeof(*room));
	room->id=idRoom;
	strcpy(room->name,name);
	room->roomNext=listRoom->first;
	room->numMsg=0;
	listRoom->first=room;
	idRoom++;
	return room;
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
		//printf("room name : %s id %d persone %d\n", room->name, room->id, room->idUser[0]);
		//printf("room name : %s id %d\n", room->name, room->id);
		room=room->roomNext;
	}
}

void decripteHeader(struct Chat_message messageRecu,struct sockaddr_in client_addr){
	switch  (messageRecu.header.commande){
	
	case CONNECT:
		connectServer(client_addr,messageRecu.data);

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
	
	case DISCONNECT:
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

void connectServer(struct sockaddr_in client_addr, char* userName){
	struct User *user=listUser->first;
	int find=0;
	struct Chat_message messageEnvoye;

	while (user->userNext != NULL && find==0){
		if (user->port==ntohs(client_addr.sin_port) && strcmp(user->ipAdress, inet_ntoa(client_addr.sin_addr)) == 0){
			find=1;
		}
		else {
			user=user->userNext;
		}
	}
	if (find==0){
		user = addUser(client_addr,userName);
		displayList();

		strcpy(messageEnvoye.data, "1");
		messageEnvoye.header.commande = ACK;
		messageEnvoye.header.idUtilisateur=user->id;
		messageEnvoye.header.timestamp=time(NULL);
		messageEnvoye.header.idSalon=0;
		messageEnvoye.header.taille=sizeof(messageEnvoye.data);
		messageEnvoye.header.numMessage=0;

		sendMessage(user->client_addr, messageEnvoye);

	}

	else{
		strcpy(messageEnvoye.data, "0");
		messageEnvoye.header.commande = ACK;
		messageEnvoye.header.idUtilisateur=user->id;
		messageEnvoye.header.timestamp=time(NULL);
		messageEnvoye.header.idSalon=0;
		messageEnvoye.header.taille=sizeof(messageEnvoye.data);
		messageEnvoye.header.numMessage=0;
		sendMessage(user->client_addr, messageEnvoye);
	}	
}

void sendMessage(struct sockaddr_in client_addr, struct Chat_message messageEnvoye){

	if (sendto(sd, &messageEnvoye, sizeof(messageEnvoye) + 1, 0,(struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
	{
		perror("sendto\n");
		return 1;
	} else {
		printf("SERV SEND to %s\n", inet_ntoa(client_addr.sin_addr) );
	}
}

void disconnectServer(struct Header header){
	printf("delete id %d\n", header.idUtilisateur);
	struct Room *room=listRoom->first;
	struct Chat_message messageEnvoye;
	struct User *userMsgEnvoie;
	struct User *userMsgEnvoieVerif;
	int i;
	if(header.idUtilisateur!=0){
	  if (room->roomNext != NULL){
		  while (room->roomNext != NULL){
			  printf("Room %s\n",room->name);
			  for (i=0;i<maxUserSalon;i++){
				  if(room->idUser[i]==header.idUtilisateur){
					  printf("Room id user%d\n",room->idUser[i]);
					  room->idUser[i]=0;
				  }
			  }
			  room=room->roomNext;
		  }
	  }
      
	  userMsgEnvoie=findUser(header.idUtilisateur);
	  if(userMsgEnvoie != NULL){ 	
	    deleteUser(header.idUtilisateur);
	  }
	  else{ 	
	    ackDisconnect(header.idUtilisateur,userMsgEnvoie->client_addr,"0");
	  }
	  
	  userMsgEnvoieVerif=findUser(header.idUtilisateur);
	  if(userMsgEnvoieVerif == NULL){ 	
	    ackDisconnect(header.idUtilisateur,userMsgEnvoie->client_addr,"1");
	  }
	  else{ 	
	    ackDisconnect(header.idUtilisateur,userMsgEnvoie->client_addr,"0");
	  }
	}
}
void ackDisconnect (int idUser,struct sockaddr_in client_addr,int etat){
  	struct Chat_message messageEnvoye;
	strcpy(messageEnvoye.data, etat);
	messageEnvoye.header.commande = ACK;
	messageEnvoye.header.idUtilisateur=idUser;
	messageEnvoye.header.timestamp=time(NULL);
	messageEnvoye.header.idSalon=idRoom;
	messageEnvoye.header.taille=sizeof(messageEnvoye.data);
	messageEnvoye.header.numMessage=0;
	sendMessage(client_addr, messageEnvoye);
}
struct User* findUser(int id){
	struct User *user=listUser->first;
	int find=0;
	while (user->userNext != NULL && find==0){
		if(user->id==id){
			find=1;
			return user;
		}
		else {
			user=user->userNext;
		}
	}
	return NULL;
}

void say(struct Chat_message messageRecu){
	//parcourt de la liste pour savoir a qui envoyer le message
	struct Room *room=listRoom->first;
	struct Chat_message messageEnvoye;
	int i,find=0;
	if (room->roomNext != NULL){
		while (room->roomNext != NULL && find==0){
			if (room->id==messageRecu.header.idSalon){
				// header : 
				struct User *userMsgRecu;
				userMsgRecu=findUser(messageRecu.header.idUtilisateur);
				strcpy(messageEnvoye.data,userMsgRecu->name);
				strcat(messageEnvoye.data," : " );
				strcat(messageEnvoye.data,messageRecu.data);
				messageEnvoye.header.commande=MESSAGE_SERVER;
				messageEnvoye.header.idUtilisateur=NULL;
				messageEnvoye.header.timestamp=time(NULL);
				messageEnvoye.header.idSalon=messageRecu.header.idSalon;
				messageEnvoye.header.taille=sizeof(messageEnvoye.data);
				messageEnvoye.header.numMessage=room->numMsg;
				room->numMsg++;
				for (i=0;i<maxUserSalon;i++){
					if(room->idUser[i]!=NULL && room->idUser[i]!=messageRecu.header.idUtilisateur){
						//printf("Message a envoyer a id :%d\n",room->idUser[i]);
						// aller cherche clients adresse
						struct User *userMsgEnvoie;
						userMsgEnvoie=findUser(room->idUser[i]);
						// ajout code envoie de message					
						sendMessage(userMsgEnvoie->client_addr, messageEnvoye);
					}
				}
				find=1;		
			}
			else {
				room=room->roomNext;
			}
		}
	}
}

void join(struct Chat_message messageRecu){
	//recherche si le salon recut est existe sinon on le crée
	struct Room *room=listRoom->first;
	int find=0,findUser=0, idVid=0;
	struct Room *newRoom;
	if (room->roomNext != NULL){
		while (room->roomNext != NULL && find==0){
			//printf("boucle while\n");
			if (strcmp (room->name, messageRecu.data) == 0){
				//printf("salon deja crée\n");
				//recherche si l'utilisateur est deja dans le salon
				int i;
				for (i=0;i<maxUserSalon;i++){
					if(room->idUser[i]==messageRecu.header.idUtilisateur){
						findUser=1;
					}
					if(room->idUser[i] == 0){
						idVid=i;
					}else
					{
						idVid=99;
					}
				}
				if (findUser==0 && idVid != 99){
					printf("id %d\n",idVid);
					//ajout de l'utilisateur dans le salon
					room->idUser[idVid]=messageRecu.header.idUtilisateur;
					ackSalon(room->id,messageRecu.header.idUtilisateur,"1");
				}
				else{
					ackSalon("0",messageRecu.header.idUtilisateur,"0");
				}
				find=1;
			}
			else {
				room=room->roomNext;
			}
		}
		if (find==0){
			newRoom=addRoom(messageRecu.data);
			//printf("id utilisateur %d\n",messageRecu.header.idUtilisateur);
			newRoom->idUser[0]=messageRecu.header.idUtilisateur;
			ackSalon(newRoom->id,messageRecu.header.idUtilisateur,"1");
			find=1;
		}
	}
	else{
		newRoom=addRoom(messageRecu.data);
		newRoom->idUser[0]=messageRecu.header.idUtilisateur;
		ackSalon(newRoom->id,messageRecu.header.idUtilisateur,"1");
		//printf("Valeur idUser dans room %d\n", newRoom->idUser[0]);
	}
	displayListRoom();
}
void ackSalon (int idRoom, int idUser,int etat){
	struct User *userMsgEnvoie;
	struct Chat_message messageEnvoye;
	userMsgEnvoie=findUser(idUser);
	strcpy(messageEnvoye.data, etat);
	messageEnvoye.header.commande = ACK;
	messageEnvoye.header.idUtilisateur=idUser;
	messageEnvoye.header.timestamp=time(NULL);
	messageEnvoye.header.idSalon=idRoom;
	messageEnvoye.header.taille=sizeof(messageEnvoye.data);
	messageEnvoye.header.numMessage=0;
	sendMessage(userMsgEnvoie->client_addr, messageEnvoye);
}
void leave(struct Header header ){
	struct Room *room=listRoom->first;
	int find=0,findUser=0,i;
	while (room->roomNext != NULL && find==0){
		if (room->id == header.idSalon){
			find=1;
			for (i=0;i<maxUserSalon;i++){
				if(room->idUser[i]==header.idUtilisateur){
					printf("trouver utilisateur dans salon dans %d\n", i);
					room->idUser[i]=0;
					printf("Velur tab i%d\n",room->idUser[i]);
					findUser=1;
				}
			}
		}
		else {
			room=room->roomNext;
		}
	}
	if (findUser==1){
	  ackSalon(room->id,header.idUtilisateur,"1");
	}
	else
	  ackSalon(room->id,header.idUtilisateur,"0");
	displayListRoom();
}

void alive(struct Header header){ 
	//Chercher utilisateur
	struct User *user;
	user=findUser(header.idUtilisateur);
	//Modifie timeLastActivity si l'utisateur est connu
	if (user!=NULL){
		user->timeLastActivity=header.timestamp;
	}
	printf("Timstemp %d\n",user->timeLastActivity);
}

void ack(struct Chat_message messageRecu){
	
}
void *verifAlive() {
	//printf("Thread lancé\n");   //Debug
	for (;;){
		if (listUser->first!=NULL){
			struct User *user=listUser->first;
			usleep(5000000);
			if (user->id !=0 ){
				while (user->userNext != NULL ){
					printf("User \n");
					int disconnect=0;
					/*disconnect=user->timeLastActivity+2;
					
					if (disconnect<= time(NULL)){
					 	//appel disconnect
					 
					}*/
					
					user=user->userNext;
				}
			}else{
				printf("USer non dispo\n");
			}
		}
		else{
			printf("USer non dispo\n");
		}
	}
	pthread_exit(0);
}

int main(void)
{

  pthread_t th_alive;
  listUser = initalization();
  listRoom =initalizationRoom();
  int n;
  socklen_t addr_len;
  //Création du Thread Alive
	if (pthread_create(&th_alive, NULL, verifAlive, NULL) != 0){
		perror("Erreur création du thread alive\n");
		return 1;
	}
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
		printf("received from %s: %d  -  %s\n", inet_ntoa(client_addr.sin_addr), messageRecu.header.commande, messageRecu.data);
		decripteHeader(messageRecu, client_addr);
    }
  }
  return 0;  
}

#endif
