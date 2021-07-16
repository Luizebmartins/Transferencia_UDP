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
#define PORTA_CLIENTE_A 1501
#define IP_LOCAL "127.0.0.1"
#define SIZE 1024

//struct de resposta do cliente A ao servidor
typedef struct mensagem {
  int porta_cliente;
  char arquivo[20];
} mensagem;

// Função para receber as mensagens
void recebeMensagem(int sd, struct sockaddr_in remoteAddr, char *buffer)
{
  int rc;
  while (1)
  {
    int addrlen = sizeof(remoteAddr);
    rc = recvfrom(sd, buffer, SIZE, 0, (struct sockaddr *)&remoteAddr, &addrlen);

    //checagem de erro
    if (rc == -1)
    {
      perror("Error");
      exit(1);
    }
    else
      break;
  }
}

//função para enviar as mensagens
void enviaMensagem(int sd, struct sockaddr_in remoteAddr, char *buffer, int tipo)
{
  int rc;
  int addrlen = sizeof(remoteAddr);

  //Se for do tipo 1, envia apenas o nome do arquivo que deseja
  if (tipo == 1)
  {
    //envia a requisição do arquivo desejado
    rc = sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *)&remoteAddr, addrlen);

    //checagem de erro
    if (rc == -1)
    {
      perror("Error");
      exit(1);
    }
  }

  //Se for do tipo 2, é a resposta contendo uma struct ao servidor.
  else if (tipo == 2)
  {
    mensagem mensagem;
    mensagem.porta_cliente = PORTA_CLIENTE_A;
    strcpy(mensagem.arquivo, buffer);

    //envia a mensagem
    rc = sendto(sd, &mensagem, sizeof(mensagem), 0, (struct sockaddr *)&remoteAddr, addrlen);

    //checagem de erro
    if (rc == -1)
    {
      perror("Error");
      exit(1);
    }
  }
}

int main(int argc, char *argv[])
{

  int sd, rc;
  char PORTA_CLIENTE_B[4];
  struct sockaddr_in remoteServAddr;

  //buffer para guardar dados temporários
  char *buffer = (char *)malloc(SIZE * sizeof(char));

  if (!buffer)
  {
    perror("Error");
    exit(1);
  }

  // Validação do parâmetro passado
  if (argc == 1)
  {
    printf("Error -- Inserir nome do arquivo desejado\n");
    exit(1);
  }

  //passa o nome do arquivo desejado para o buffer
  strcpy(buffer, argv[1]);

  //Criação do socket
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  //Checagem de erro
  if (sd == -1)
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

  //enviando requisição ao servidor, em busca de algum cliente
  //que tenha tal arquivo
  enviaMensagem(sd, remoteServAddr, buffer, 1);


  printf("esperando resposta do servidor..\n");
  //zerando o buffer para receber a resposta do servidor
  memset(buffer, '\0', SIZE);

  //Recebe a resposta do servidor, contendo a porta do cliente
  //que possui o arquivo
  recebeMensagem(sd, remoteServAddr, buffer);
  if (buffer[0] == '1')
  {
    strcpy(PORTA_CLIENTE_B, &buffer[1]);
    printf("O cliente na porta %s possui o arquivo.\n", PORTA_CLIENTE_B);
    memset(buffer, '\0', SIZE);
  }
  else
  {
    printf("Error - Arquivo não encontrado na base de dados.\n");
    exit(1);
  }


  //Avisando o servidor que o cliente A também possui o arquivo
  strcpy(buffer, argv[1]);
  enviaMensagem(sd, remoteServAddr, buffer, 2);

  memset(buffer, '\0', SIZE);
  recebeMensagem(sd, remoteServAddr, buffer);
  if(buffer[0] == '1')
    printf("Cliente A agora presente na base de dados\n");

  free(buffer);
  return 0;
}