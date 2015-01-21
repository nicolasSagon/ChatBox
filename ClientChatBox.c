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

int sd;
struct sockaddr_in client_addr, serv_addr;

int cmdStrToInt(char * str){
	if(strcmp(str, "CONNECT") == 0)
		return 0;
	else if(strcmp(str, "JOIN") == 0)
		return 1;
	else if(strcmp(str, "SAY") == 0)
		return 2;
	else if(strcmp(str, "LEAVE") == 0)
		return 3;
	else if(strcmp(str, "DISCONNECT") == 0)
		return 4;
	else if(strcmp(str, "ACK") == 0)
		return 5;
	else if(strcmp(str, "ALIVE") == 0)
		return 6;
	else if(strcmp(str, "MESSAGE_SERVER") == 0)
		return 7;
	else
		return -1;
}

void *timer() {
	struct Chat_message msgKeepAlive;
	//printf("Thread lancé\n");   //Debug
	for (;;){
		usleep(15000000);
		strcpy(msgKeepAlive.data,"");
		msgKeepAlive.header.commande=6;
		msgKeepAlive.header.idUtilisateur=1;
		msgKeepAlive.header.timestamp=time(NULL);
		msgKeepAlive.header.idSalon=1;
		msgKeepAlive.header.taille=sizeof(msgKeepAlive.data);
		msgKeepAlive.header.numMessage=1; 
		if (sendto(sd, &msgKeepAlive, sizeof(msgKeepAlive) + 1, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
			perror("sendto\n");
			pthread_exit((void*) 1);
		}
		else{
			//printf("Je vole!!\n"); //Debug message Keep alive envoyé     
		}
	}
	pthread_exit(0);
}

void *msgServer(){
	struct Chat_message msgServer;
	int n;
	socklen_t addr_len;
	
	addr_len = sizeof(serv_addr);
	while(1){
		n = recvfrom(sd, &msgServer, MAX_MSG, 0,(struct sockaddr *)&serv_addr, &addr_len);
		if (n == -1)
			perror("recvfrom");
		else {
			printf("received from %s: %d\n", inet_ntoa(serv_addr.sin_addr), msgServer.header.commande);
		}
	}
	pthread_exit(0);
}

struct Chat_message msgConnection(struct Chat_message messageEnvoye){
	strcpy(messageEnvoye.data,"");
	messageEnvoye.header.commande=CONNECT;
	messageEnvoye.header.idUtilisateur=1;
	messageEnvoye.header.timestamp=time(NULL);
	messageEnvoye.header.idSalon=1;
	messageEnvoye.header.taille=sizeof(messageEnvoye.data);
	messageEnvoye.header.numMessage=1;
	return messageEnvoye;
}

void sendMsg(struct sockaddr_in client_addr, struct Chat_message messageEnvoye){
// send all messages
	if (sendto(sd, &messageEnvoye, sizeof(messageEnvoye) + 1, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		perror("sendto\n");
		exit(1);
	} else {
		printf("Sent to %s\n", inet_ntoa(serv_addr.sin_addr) );
	}
}

int main (int argc, char *argv[]){
	
	pthread_t th_timer;
	pthread_t th_msgServer;
	
	struct Chat_message messageEnvoye;
	struct Chat_message messageRecu;
	socklen_t addr_len;
	
	char buffer[TAILLEDATA];
	char saveBuffer[TAILLEDATA];
	char commande[20];
	char data[TAILLEDATA];
	char *mot;
	int n;
	
	messageEnvoye = msgConnection(messageEnvoye);
		
	strcpy(data, "");
	strcpy(commande, "");
		
	printf("%s: trying to send to %s\n", argv[0], argv[1]);
	
	// Create socket
	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror(argv[0]);
		return 1;
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
	if (inet_aton(argv[1], &(serv_addr.sin_addr)) == 0)
	{
		printf("Invalid IP address format <%s>\n", argv[1]);
		return 1;
	}
	serv_addr.sin_port = htons(SERVER_PORT);
	
	// send all messages
	sendMsg(client_addr,messageEnvoye);
		
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
	
	//Attente reception confirmation serveur
	addr_len = sizeof(serv_addr);
	n = recvfrom(sd, &messageRecu, MAX_MSG, 0,(struct sockaddr *)&serv_addr, &addr_len);
    if (n == -1)
		perror("recvfrom");
	else {
		printf("received from %s: %d\n", inet_ntoa(serv_addr.sin_addr), messageRecu.header.commande);
    }
	
	while(1){
	//printf("debug: entree dans boucle while 1\n");
	
		if(fgets(buffer, 140, stdin) != NULL){
			//printf("debug: fgets ok\n");
			strcpy(saveBuffer, buffer);
		}
		else
		perror("debug: Erreur fgets\n");

		strcpy(commande, strtok(buffer," ")); //Extraction de la commande (Buffer jusqu'au premier espace)		

		strcpy(messageEnvoye.data, ""); //reset des strings
		strcpy(data, "");//reset des strings
		
		if(cmdStrToInt(commande) == -1){ //si cmd non définie
			messageEnvoye.header.commande = 2; // c'est un SAY
			strcpy(messageEnvoye.data, saveBuffer);
		}
		else{
			messageEnvoye.header.commande=cmdStrToInt(commande); //Affectation de la var cmd au message
			
			mot = strtok (NULL, " ,.-");
			while (mot != NULL){ //Extraction de data
				strcat(data, mot);
				strcat(data, " ");
				mot = strtok (NULL, " ,.-");
			}
			strcpy(messageEnvoye.data, data);
		}
		
		printf("Commande = %i\n", messageEnvoye.header.commande);
		printf("Data = %s\n", messageEnvoye.data);
		messageEnvoye.header.idUtilisateur=1;
		messageEnvoye.header.timestamp=time(NULL);
		messageEnvoye.header.idSalon=1;
		messageEnvoye.header.taille=sizeof(messageEnvoye.data);
		messageEnvoye.header.numMessage=2;
		
		if (sendto(sd, &messageEnvoye, sizeof(messageEnvoye) + 1, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
			perror("sendto\n");
			return 1;
		} else {
			printf("Sent to %s\n", inet_ntoa(serv_addr.sin_addr) );
		}

	}
	close(sd);
	return 0;
}

#endif
