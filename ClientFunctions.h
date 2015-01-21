#ifndef ClientFunctionsh
#define ClientFunctionsh

int cmdStrToInt(char * str);
void *timer();
void *msgServer();
void sendMsg(struct sockaddr_in client_addr, struct Chat_message messageEnvoye);

#endif
