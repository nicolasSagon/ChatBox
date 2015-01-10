#ifndef CLIENTCHATBOX
#define CLIENTCHATBOX

#include "protocole.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVER_PORT 1500
#define MAX_MSG 80
int main (int argc, char *argv[])
{
  int sd, i;
  struct sockaddr_in client_addr, serv_addr;
  // Command-line error check
  if (argc < 3)
    return 1;
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
    perror("bind");
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
  for (i = 2; i < argc; i++)
  {
    if (sendto(sd, argv[i], strlen(argv[i]) + 1, 0,
          (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
      perror("sendto");
      return 1;
    } else {
      printf("Sent to %s: %s\n", inet_ntoa(serv_addr.sin_addr), argv[i]);
    }
  }
  close(sd);
  return 0;
}

#endif
