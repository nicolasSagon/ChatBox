#ifndef CLIENTFUNCTIONS.C
#define CLIENTFUNCTIONS.C
#include <string.h>


char* getData(char *buffer){
  char *data = NULL;
  char *pch = NULL;
  
  //strcat(data, "");
  
  while (pch != NULL)
  {
    pch = strtok (NULL, " ,.-");
    if(pch != NULL){
     strcat(data, pch);
     strcat(data, " "); 
    }
  }
  printf("%s",data);
  return data;
}
#endif