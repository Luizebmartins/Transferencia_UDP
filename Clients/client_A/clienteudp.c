#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>   /* memset() */
#include <sys/time.h> /* select() */

#define REMOTE_SERVER_PORT 1500
#define IP_LOCAL "127.0.0.1"
#define SIZE 1024


void enviaRequisicao(int sd, struct sockaddr_in remoteServAddr, char * buffer) {
  int n;

  //envia a requisição do arquivo desejado
  n = sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));

  //checagem de erro
  if(n == -1)
  {
    perror("[ERROR] Enviar requisição para o servidor");
    exit(1);
  }


  //Esperando resposta do servidor
  while (1)
  {
    //zerando o buffer para receber a resposta do servidor
    memset(buffer, '\0', SIZE);


  }
  

}


int main(int argc, char *argv[])
{

  int sd, rc;
  struct sockaddr_in remoteServAddr;

  char *buffer = (char*) malloc(SIZE * sizeof(char));

  // Validação do parâmetro passado
  if(argc == 1)
  {
    printf("Error -- Inserir nome do arquivo desejado");
    exit(1);
  }

  strcpy(buffer, argv[1]);

  //Criação do socket
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  //Checagem de erro
  if(sd == -1)
  {
    perror("Error");
    exit(1);
  }

  //zerando a struct
  memset(&remoteServAddr, 0, sizeof(remoteServAddr));

  //struct com dados do servidor
  remoteServAddr.sin_family = AF_INET;
  remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);
  remoteServAddr.sin_addr.s_addr = inet_addr(IP_LOCAL);
  

  enviaRequisicao(sd, remoteServAddr, buffer);



  free(buffer);
  return 0;
}
