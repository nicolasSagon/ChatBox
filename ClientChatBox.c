#ifndef CLIENTCHATBOX
#define CLIENTCHATBOX

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

#include "protocole.h"
#include "ClientFunctions.h"

#define SERVER_PORT 1500
#define MAX_MSG 80

int sd,idUser, salonId;
struct sockaddr_in client_addr, serv_addr;
pthread_mutex_t mutexAck;
int ackNeeded;
char name[25];


int compareMot(char * mot) {
	FILE* dico = fopen("insultes.txt","r");
	char * line = NULL;
	int i;
	ssize_t read;
	size_t len = 0;
	if(mot != NULL){
		while ((read = getline(&line, &len, dico)) != -1) {
			if(strstr(mot,line) != NULL) {
				close(dico);			
				return 1;
			}
		}
		close(dico);
	}
	return 0;
}


int cmdStrToInt(char * str){
	if(strstr(str, "DISCONNECT") != NULL)
		return 4;
	else if(strstr(str, "JOIN") != NULL)
		return 1;
	else if(strstr(str, "SAY") != NULL)
		return 2;
	else if(strstr(str, "LEAVE") != NULL)
		return 3;
	else if(strstr(str, "CONNECT") != NULL)
		return 0;
	else if(strstr(str, "ACK") != NULL)
		return 5;
	else if(strstr(str, "ALIVE") != NULL)
		return 6;
	else if(strstr(str, "MESSAGE_SERVER") != NULL)
		return 7;
	else if(strstr(str, "SWITCH") != NULL)
		return 8;
	else
		return -1;
}

void *timer() {
	struct Chat_message msgKeepAlive;
	for (;;){
		usleep(15000000);
		strcpy(msgKeepAlive.data,"");
		msgKeepAlive.header.commande=6;
		msgKeepAlive.header.idUtilisateur=idUser;
		msgKeepAlive.header.timestamp=time(NULL);
		msgKeepAlive.header.idSalon=1;
		msgKeepAlive.header.taille=sizeof(msgKeepAlive.data);
		msgKeepAlive.header.numMessage=1; 
		if (sendto(sd, &msgKeepAlive, sizeof(msgKeepAlive) + 1, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)		{
			perror("sendto\n");
			pthread_exit((void*) 1);
		}
	}
	pthread_exit(0);
}

void *msgServer(){
	struct Chat_message msgServer;
	int n;
	struct Chat_message messageEnvoye;
	socklen_t addr_len;
	
	addr_len = sizeof(serv_addr);
	while(1){
		n = recvfrom(sd, &msgServer, MAX_MSG, 0,(struct sockaddr *)&serv_addr, &addr_len);
		if (n == -1){
			if(ackNeeded != -1){
					printf("Message non Envoyé\n");
					pthread_mutex_unlock(&mutexAck);
			}
		}
		else {
			if(msgServer.header.lastCommandeId == 1){
				if(ackNeeded == 1){
						ackNeeded = -1;
						salonId = msgServer.header.idSalon;
						pthread_mutex_unlock(&mutexAck);
				}
			}
			else if(msgServer.header.lastCommandeId == 2){
					if(ackNeeded == 2){
							ackNeeded = -1;
							pthread_mutex_unlock(&mutexAck);
					}
			}
			else if(msgServer.header.lastCommandeId == 3){
					if(ackNeeded == 3){
							ackNeeded = -1;
							if(strcmp(msgServer.data,"1")==0){
								printf("Salon changé\n");
								salonId = 0;
							}else{
								printf("Erreur, salon non quité\n");
							}
							pthread_mutex_unlock(&mutexAck);
					}
			}
			else if(msgServer.header.lastCommandeId == 8){
					if(ackNeeded == 8){
							ackNeeded = -1;
							if(strcmp(msgServer.data,"1")==0){
								printf("Salon quitté\n");
								salonId = msgServer.header.idSalon;
							}else{
								printf("Erreur, salon non changer\n");
							}
							pthread_mutex_unlock(&mutexAck);
					}
			}
			else if(msgServer.header.commande == 6){
					strcpy(messageEnvoye.data,"1");
					messageEnvoye.header.commande=6;
					messageEnvoye.header.idUtilisateur=idUser;
					messageEnvoye.header.timestamp=time(NULL);
					messageEnvoye.header.idSalon=0;
					messageEnvoye.header.taille=sizeof(messageEnvoye.data);
					messageEnvoye.header.numMessage=1; 
					if (sendto(sd, &messageEnvoye, sizeof(messageEnvoye) + 1, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
						perror("sendto\n");
						pthread_exit((void*) 1);
					}
			}
			if(msgServer.header.commande == 7)
			{
				printf("\33[2K\r%s",msgServer.data);
				printf("%s : ", name);
				fflush(stdout);
			}
			
		}
	}
	pthread_exit(0);
}

struct Chat_message msgConnection(struct Chat_message messageEnvoye){
	printf("Entrez votre nom d'utilisateur : \n");
	if(fgets(name, 25, stdin) != NULL){
			name[strlen(name)-1] = '\0';
			strcpy(messageEnvoye.data, name);
	}
	messageEnvoye.header.commande=CONNECT;
	messageEnvoye.header.lastCommandeId = -1;
	messageEnvoye.header.idUtilisateur=0;
	messageEnvoye.header.timestamp=time(NULL);
	messageEnvoye.header.idSalon=0;
	
	messageEnvoye.header.taille=sizeof(messageEnvoye.data);
	messageEnvoye.header.numMessage=0;
	return messageEnvoye;
}

void sendMsg(struct sockaddr_in client_addr, struct Chat_message messageEnvoye){
// send all messages
	if (sendto(sd, &messageEnvoye, sizeof(messageEnvoye) + 1, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		perror("sendto\n");
	} else {
// 		printf("Sent to %s\n", inet_ntoa(serv_addr.sin_addr) ); //Debug
	}
}

void init(char *ip){
	// Create socket
	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
		perror("erreur ip: Creation de socket");
		return 1;
	}
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
	}
	// Bind socket
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	client_addr.sin_port        = htons(0);
	if (bind(sd,(struct sockaddr *)&client_addr, sizeof client_addr) == -1)
	{
		perror("bind\n");
		return 1;
	}
	// Fill server address structure
	serv_addr.sin_family = AF_INET;
	if (inet_aton(ip, &(serv_addr.sin_addr)) == 0)
	{
		printf("Invalid IP address format <%s>\n", ip);
		return 1;
	}
	serv_addr.sin_port = htons(SERVER_PORT);	
}

int main (int argc, char *argv[]){
	
	pthread_t th_timer;
	pthread_t th_msgServer;
	
	struct Chat_message messageEnvoye;
	struct Chat_message messageRecu;
	socklen_t addr_len;
	
	char buffer[TAILLEDATA];
	char saveBuffer[TAILLEDATA];
	char commande[20] = "";
	char data[TAILLEDATA] = "";
	char *mot;
	int n;
	
	
	messageEnvoye = msgConnection(messageEnvoye);
	
	//Création, bind et Affectation du socket
	init(argv[1]);
	
	
	
	// send connexion msg
	sendMsg(client_addr,messageEnvoye);

	//Attente reception confirmation serveur
	addr_len = sizeof(serv_addr);
	n = recvfrom(sd, &messageRecu, MAX_MSG, 0,(struct sockaddr *)&serv_addr, &addr_len);
    if (n == -1)
		perror("recvfrom");
	else {
		printf("received from server@ %s\n", inet_ntoa(serv_addr.sin_addr), messageRecu.header.commande);
		idUser = messageRecu.header.idUtilisateur;
		printf("idUser = %i\n",idUser);
    }
    
    //Création du Thread Timer
	if (pthread_create(&th_timer, NULL, timer, NULL) != 0){
		perror("Erreur création du thread timer\n");
		return 1;
	}
	
// 	Création du thread MessageServer
	if (pthread_create(&th_msgServer, NULL, msgServer, NULL) != 0){
		perror("Erreur création du thread MESSAGE_SERVER\n");
		return 1;
	}
	
	if(pthread_mutex_init(&mutexAck, NULL) != 0 ){
			perror("mutex");
			exit(1);
	}
	
	pthread_mutex_lock(&mutexAck);
	
	ackNeeded = -1;
	while(1){
		printf("%s : ", name);
		if(fgets(buffer, 140, stdin) != NULL){
			strcpy(saveBuffer, buffer);
		}
		else
			perror("debug: Erreur fgets\n");

		strcpy(commande, strtok(buffer," ")); //Extraction de la commande (Buffer jusqu'au premier espace)		

		strcpy(messageEnvoye.data, ""); //reset des strings
		strcpy(data, "");//reset des strings

		if(cmdStrToInt(commande) == -1){ //si cmd non définie
			messageEnvoye.header.commande = 2; // c'est un SAY
			//strcpy(messageEnvoye.data, saveBuffer);
			mot = strtok (NULL, " ");
			if (compareMot(commande) == 1) {
				strcat(data, "@!#~=§ ");
			}
			else
			{
				strcat(data, commande);
				strcat(data, " ");
			}
			while (mot != NULL){ //Extraction de data
				if (compareMot(mot) == 1)
				{
					strcat(data, "@!#~=§ ");
				}
				else
				{
					strcat(data, mot);
					strcat(data, " ");
				}
				mot = strtok (NULL, " ");
			}
			strcpy(messageEnvoye.data, data);
		}
		else{
			messageEnvoye.header.commande=cmdStrToInt(commande); //Affectation de la var cmd au message
			
			mot = strtok (NULL, " ");
			while (mot != NULL){ //Extraction de data
				if (compareMot(mot) == 1)
				{
					strcat(data, "@!#~=§ ");
				}
				else
				{
					strcat(data, mot);
					strcat(data, " ");
				}
				mot = strtok (NULL, " ");
			}
			strcpy(messageEnvoye.data, data);
		}
		
		strcat(messageEnvoye.data, "\n");
		messageEnvoye.header.idUtilisateur=idUser;
		messageEnvoye.header.timestamp=time(NULL);
		messageEnvoye.header.lastCommandeId = -1;
		messageEnvoye.header.idSalon=salonId;
		messageEnvoye.header.taille=sizeof(messageEnvoye.data);
		messageEnvoye.header.numMessage=2;
		
		sendMsg(client_addr,messageEnvoye);
		
		if(cmdStrToInt(commande) == 4)
				exit(0);
		
		ackNeeded = messageEnvoye.header.commande;
		pthread_mutex_lock(&mutexAck);
		ackNeeded = -1;
		
	}
	close(sd);
	return 0;
}

#endif
