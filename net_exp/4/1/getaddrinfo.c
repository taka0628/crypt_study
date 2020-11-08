#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

char* get_adress(char *hostname)
{
  struct addrinfo hints, *res;
  struct in_addr addr;
  char* buf = (char *)malloc(sizeof(char) * 16);
  if(buf != NULL){
    memset(buf, '\0', 16);
  }
  int err;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
  {
    printf("error %d\n", err);
    return NULL;
  }

  addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
  inet_ntop(AF_INET, &addr, buf, 16);
  printf("ip address : %s\n", buf);

  freeaddrinfo(res);
  return buf;
}

int main(int argc, char* argv[])
{
  char hostname[32];
  int port_num = 0;
  if(argc == 3){
    strcpy(hostname, argv[1]);
    port_num = atoi(argv[2]);
    char *temp = get_adress(hostname);
    strcpy(hostname, temp);
    free(temp);
  }else{
    strcpy(hostname, "localhost");
    port_num = 12345;
  }
  
  return 0;
}
