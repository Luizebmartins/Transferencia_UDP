/*
  Instruções para compilação e execução
  Disciplina COM240 - Redes de Computadores
  Professor Bruno Guazzelli Batista
  Compilar - gcc servidorudp.c -o servidorudp
  Executar - ./servidorudp
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */

#define LOCAL_SERVER_PORT 1500
#define MAX_MSG 100
#define IPADRESS "127.0.0.1"

int main(int argc, char *argv[]) {
  
  int sd, rc;
  



return 0;

}

