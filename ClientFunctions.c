#ifndef CLIENTFUNCTIONS
#define CLIENTFUNCTIONS
#include "ClientFunctions.h"
#include <string.h>

char * getCmd(char * buffer){
  char * commande;
  char * pch = strtok (buffer," ");
  strcpy(commande, pch);
  return commande;
}

char * getData(char * buffer){
  char * data;
  char * pch;
  
  while (pch != NULL)
  {
    pch = strtok (NULL, " ,.-");
    if(pch != NULL){
     strcat(data, pch);
     strcat(data, " "); 
    }
  }
  return data;
}
#endif