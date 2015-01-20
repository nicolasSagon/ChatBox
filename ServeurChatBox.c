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

void decripteHeader(struct Chat_message messageRecu,int sd){
	switch  (messageRecu.header.commande){
	
	case CONNECT:
		connect(sd,messageRecu.data);
	break;
	
	case JOIN:
		join(messageRecu)
	break;
	
	case SAY:
		say(messageRecu);
	break;
	
	case LEAVE:
		leave(messageRecu.header);
	break;
	
	case DISCONNECT:
		disconnect(messageRecu);
	break;
	
	case ACK:
		ack(messageRecu);
	break;
	
	case ALIVE:
		alive(messageRecu.header);
	break;

	}
}

void connect(int sd, char* userName){
	
}

void disconnect(struct Chat_message messageRecu){
	
}

void say(struct Chat_message messageRecu){

}

void join(struct Chat_message messageRecu){

}

void leave(struct Header header ){

}

void alive(struct Header header){

}

void ack(struct Chat_message messageRecu){

}

int main(void)
{
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
    }
  }
  return 0;
}

#endif
