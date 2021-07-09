#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>

#define PORTA_SERVIDOR 1500
#define MAX_MSG 100
#define IP_LOCAL "127.0.0.1"

int verifica_requisicao(int requisicao){
	if(requisicao == 1){
		printf("Nenhum arquivo foi requisitado.\n");
		return 0;
	}else return 1;
}

int verifica_banco(FILE *BD){
	fseek(BD, 0, SEEK_END);
	int arq_vazio = ftell(BD);
	if(arq_vazio != 0){
		fseek(BD, 0, SEEK_SET);
		return 1;
	}else printf("Banco de dados vazio.\n");

	return 0;
}

void configura_socket(int socket_serv, struct sockaddr_in endereco_serv, socklen_t tam_struct_clienteA){
	socket_serv = socket(AF_INET, SOCK_DGRAM, 0);
	endereco_serv.sin_family = AF_INET;
  	endereco_serv.sin_addr.s_addr = htonl(INADDR_ANY);
  	endereco_serv.sin_port = htons(PORTA_SERVIDOR);
  	bind(socket_serv, (struct sockaddr *) &endereco_serv, sizeof(endereco_serv));
  	listen(socket_serv, 2);
}

int busca_no_banco(FILE *BD, char *requisicao){
	char porta_cliente[10];
	char arquivo_BD[10];

	while(fscanf(BD, "%s %s", arquivo_BD, porta_cliente) != EOF)
		if(strcmp(arquivo_BD, requisicao) == 0)
			return atoi(porta_cliente);
		

	printf("Arquivo não existe no banco de dados.");
	return 0;
}

int main(int argc, char *argv[]){

	int socket_serv;
	int bind_serv;
	struct sockaddr_in endereco_serv;
	struct sockaddr_in endereco_clienteA;
	socklen_t tam_struct_clienteA;
	char requisicao[MAX_MSG];
	char *buffer = (char *) malloc(MAX_MSG * sizeof(char));
	FILE *BD;
	
	if(verifica_requisicao(argc) == 0) return 0;
	strcpy(requisicao, argv[1]);

	BD = fopen("banco_de_dados.txt", "rb");
	if(verifica_banco(BD) == 0) return 0;

	configura_socket(socket_serv, endereco_serv, tam_struct_clienteA);

	printf("%d\n", busca_no_banco(BD, requisicao));
	

  	while(1){
  		tam_struct_clienteA = sizeof(endereco_clienteA);
  		recvfrom(socket_serv, buffer, MAX_MSG, 0, (struct sockaddr *) &endereco_clienteA, &tam_struct_clienteA);
  	}

	

}
