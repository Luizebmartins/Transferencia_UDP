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
#define MAX_BUFFER 1024
#define IP_LOCAL "127.0.0.1"

int verifica_requisicao(int qtd_req){
	if(qtd_req == 1){
		printf("Nenhum arquivo foi requisitado.\n");
		return 1;
	}else return 0;
}

int verifica_banco(FILE *BD){
	fseek(BD, 0, SEEK_END);
	int arq_vazio = ftell(BD);
	if(arq_vazio != 0){
		fseek(BD, 0, SEEK_SET);
		return 0;
	}else printf("Banco de dados vazio.\n");

	return 1;
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
	char porta_cliente[MAX_MSG];
	char arquivo_BD[MAX_MSG];

	while(fscanf(BD, "%s %s", arquivo_BD, porta_cliente) != EOF)
		if(strcmp(arquivo_BD, requisicao) == 0){
			strcpy(requisicao, porta_cliente);
			return 0;
		}
		
	printf("Arquivo não existe no banco de dados.\n");
	return 1;
}

int verifica_buffer(char *buffer){
	if(buffer[0] != '\0') return 1;
	printf("Nenhum dado foi recebido.\n");
	return 0;
}

int main(int argc, char *argv[]){

	int socket_serv;
	int bind_serv;

	struct sockaddr_in endereco_serv;
	struct sockaddr_in endereco_clienteA;
	socklen_t tam_struct_clienteA;

	char requisicao[MAX_MSG];
	char cliente_com_arquivo[MAX_MSG];
	char *buffer = (char *) malloc(MAX_BUFFER * sizeof(char));
	
	FILE *BD;
	
	if(verifica_requisicao(argc)) return 1;
	strcpy(requisicao, argv[1]);

	BD = fopen("database.txt", "rb");
	
	if(busca_no_banco(BD, requisicao)) return 1;

	configura_socket(socket_serv, endereco_serv, tam_struct_clienteA);

	//printf("%s\n", requisicao);

  	while(1){
  		memset(buffer,'\0', MAX_BUFFER);
  		tam_struct_clienteA = sizeof(endereco_clienteA);
  		recvfrom(socket_serv, buffer, MAX_BUFFER, 0, (struct sockaddr *) &endereco_clienteA, &tam_struct_clienteA);
  		strcat(buffer, cliente_com_arquivo);
  		sendto(socket_serv, buffer, MAX_BUFFER, 0, (struct sockaddr *) &endereco_clienteA, tam_struct_clienteA);

  		if(verifica_buffer(buffer)){
			buffer[0] = '1';
			strcat(buffer, requisicao);
			sendto(socket_serv, buffer, MAX_BUFFER, 0, (struct sockaddr *) &endereco_clienteA, tam_struct_clienteA);
  		}else return 1;
  		
  	}

}
