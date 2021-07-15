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

//struct de resposta do cliente A ao servidor
typedef struct mensagem{
    int porta_cliente;          
    char arquivo[20];     
}mensagem;

int verifica_banco(FILE *BD){
	fseek(BD, 0, SEEK_END);
	int arq_vazio = ftell(BD);
	if(arq_vazio != 0){
		fseek(BD, 0, SEEK_SET);
		return 0;
	}else printf("Banco de dados vazio.\n");

	return 1;
}

int configura_socket(){
	struct sockaddr_in endereco_serv;
	int socket_serv;

	socket_serv = socket(AF_INET, SOCK_DGRAM, 0);

	if(socket_serv < 0){
		printf("Criação do socket falhou!\n");
		return 1;
	}

	endereco_serv.sin_family = AF_INET;
  	endereco_serv.sin_addr.s_addr = htonl(INADDR_ANY);
  	endereco_serv.sin_port = htons(PORTA_SERVIDOR);

  	if(bind(socket_serv, (struct sockaddr *) &endereco_serv, sizeof(endereco_serv)) < 0){
  		printf("Bind no socket falhou!\n");
  		return 1;
  	} 

  	return socket_serv;
	
}

int busca_no_banco(FILE *BD, char *buffer, char *porta_cliente_com_arquivo){

	char porta_cliente[MAX_MSG];
	char arquivo_BD[MAX_MSG];

	while(fscanf(BD, "%s %s", arquivo_BD, porta_cliente) != EOF){
		if(strcmp(arquivo_BD, buffer) == 0){
			strcpy(porta_cliente_com_arquivo, porta_cliente);
			return 1;
		}
	}

	return 0;
}

int verifica_buffer(char *buffer){
	if(buffer[0] != '\0') return 1;
	return 0;
}


int main(int argc, char *argv[]){

	int socket_serv;

	
	struct sockaddr_in endereco_cliente;
	socklen_t tam_struct_clienteA;

	char cliente_com_arquivo[MAX_MSG];
	char *buffer = (char *) malloc(MAX_BUFFER * sizeof(char));
	
	FILE *BD;

	BD = fopen("database.txt", "rb");

	socket_serv = configura_socket();

  	while(1){
  		
  		memset(buffer,'\0', MAX_BUFFER);
  		
  		tam_struct_clienteA = sizeof(endereco_cliente);

  		recvfrom(socket_serv, buffer, MAX_BUFFER, 0, (struct sockaddr *) &endereco_cliente, &tam_struct_clienteA);

  		if(verifica_buffer(buffer)){
  			if(verifica_banco(BD)) return 1;
  			if(busca_no_banco(BD, buffer, cliente_com_arquivo)){
  				memset(buffer,'\0', MAX_BUFFER);
				buffer[0] = '1';
				strcat(buffer, cliente_com_arquivo);
				sendto(socket_serv, buffer, MAX_BUFFER, 0, (struct sockaddr *) &endereco_cliente, tam_struct_clienteA);
  			}else{
  				memset(buffer,'\0', MAX_BUFFER);
				buffer[0] = '0';
				sendto(socket_serv, buffer, MAX_BUFFER, 0, (struct sockaddr *) &endereco_cliente, tam_struct_clienteA);
  			}
  		}
  	}

  	mensagem blk;

  	while(1){
		memset(&blk, 0x0, sizeof(mensagem));
		memset(buffer,'\0', MAX_BUFFER);
		recvfrom(socket_serv, &blk, sizeof(blk), 0, (struct sockaddr *) &endereco_cliente, &tam_struct_clienteA);
		strcpy(buffer, "1");
		sendto(socket_serv, buffer, sizeof(buffer), 0, (struct sockaddr *) &endereco_cliente, tam_struct_clienteA);
		fprintf(BD, "\n%s %d", blk.arquivo, blk.porta_cliente);
		fflush(BD);
		fclose(BD);
        break;
  	}

	free(buffer);
	return 0;
}
