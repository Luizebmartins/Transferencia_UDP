#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUFFER 1024
#define PORTA_CLIENTEB 1502

//estrutura do pacote
typedef struct pacote{
	int numseq;     	        //número de sequência do pacote			
	long int check_sum;			//valor do checksum do pacote	
	char segmento[512];			//segmento do pacote
	int tam;                    //tamanho do pacote
}pacote;

//função responsável por realizar o checksum
long int checksum(char segmento[], int tamanho){

	long int checksum = 0;

	//percorrer o tamanho do segmento
	for(int i=0; i<tamanho; i++){
		
        //converte pra inteiro o caracter e soma
        checksum += (int) (segmento[i]);
	}

    //retorna o valor do checksum
	return checksum;
}

//função responsável por transferir o arquivo para o cliente A
int enviaPacote(char *mensagem, FILE *arquivo, int socket_clienteB, struct sockaddr_in endereco_clienteB, socklen_t tam_struct_clienteB){

    int tam = 0, contador_pacote = 0, num_seq = 0;
    char *buffer;
    pacote pkt;

    //aloca memória para o buffer
    buffer = (char *) malloc(MAX_BUFFER * sizeof(buffer));

    if(!buffer){
        printf("Nao foi possivel alocar memoria para o buffer!\n");
        return -1;
    }
    
    memset(&pkt, 0x0, sizeof(pkt));

    //enquanto o arquivo não acabar
    while(!feof(arquivo)){
        
        contador_pacote++;
        num_seq++;

        printf("\nEnviando o pacote %d de numero de sequencia %d. Por favor, aguarde...\n", contador_pacote, num_seq);
        
        //le 512 bytes do arquivo
        tam = fread(pkt.segmento, 1, 512, arquivo);

        pkt.tam = tam;
        //strcpy(pkt.segmento, mensagem);
        pkt.numseq = num_seq;
        pkt.check_sum = checksum(pkt.segmento, pkt.tam);

        //envia o pacote para o cliente A
        if(sendto(socket_clienteB, &pkt, sizeof(pkt)+1, 0, (struct sockaddr *) &endereco_clienteB, tam_struct_clienteB) < 0){
            printf("Falha ao enviar o pacote de numero de sequencia = %d!\n", num_seq);
            return -1;
        }

        //recebe a mensagem de ack (confirmação de que recebeu o pacote) do cliente A
        if(recvfrom(socket_clienteB, buffer, sizeof(buffer)+1, 0, (struct sockaddr *) &endereco_clienteB, &tam_struct_clienteB) < 0){
            printf("Erro ao receber ack do cliente A!\n");
            return -1;
        }

        printf("%s recebido do cliente A!\n", buffer);
    }

    //depois que termina o arquivo, envia um pacote final com tamanho igual a zero
    //indicando para o cliente A que a transferência do arquivo acabou
    pkt.tam = 0;
    strcpy(pkt.segmento, "");
    pkt.numseq = 0;
    pkt.check_sum = 0;

    //envia o pacote final para o cliente A
    if(sendto(socket_clienteB, &pkt, sizeof(pkt)+1, 0, (struct sockaddr *) &endereco_clienteB, tam_struct_clienteB) < 0){
        printf("Erro ao tentar enviar o pacote final de confirmacao!\n");
        return -1;
    }
}

int configura_socket(){
    int socket_clienteB;
    struct sockaddr_in endereco_clienteB;

    socket_clienteB = socket(AF_INET, SOCK_DGRAM, 0);

    if(socket_clienteB < 0){
        printf("Criação do socket falhou!\n");
        exit(1);
    }

    memset(&endereco_clienteB, 0, sizeof(endereco_clienteB));
    endereco_clienteB.sin_family = AF_INET;
    endereco_clienteB.sin_port = htons(PORTA_CLIENTEB);

    if(bind(socket_clienteB, (struct sockaddr *) &endereco_clienteB, sizeof(endereco_clienteB)) < 0){
        perror("bind");
        printf("Bind no socket falhou!\n");
        exit(1);
    } 
    
    listen(socket_clienteB, 2);

    return socket_clienteB;
}

int verifica_buffer(char *buffer){
    if(buffer[0] != '\0') return 1;
    return 0;
}

int main(){

    int socket_clienteB;
    struct sockaddr_in endereco_clienteB;

    char *buffer, *msg = (char *) malloc(512 * sizeof(char));
    socklen_t tam_struct_clienteB; 
    FILE *arquivo_clienteB;

    buffer = (char*) malloc(MAX_BUFFER * sizeof(char));

    socket_clienteB = configura_socket();

    printf("Aguardando solicitações...\n");

    //---------COMUNICAÇÃO COM O CLIENTE A---------
    while(1){

        memset(buffer, '\0', MAX_BUFFER);
        
        tam_struct_clienteB = sizeof(endereco_clienteB);

        recvfrom(socket_clienteB, buffer, MAX_BUFFER + 1, 0, (struct sockaddr *) &endereco_clienteB, &tam_struct_clienteB);

        printf("Mensagem recebida do cliente A: %s\n", buffer);
        
        if(verifica_buffer(buffer)){
           
            arquivo_clienteB = fopen(buffer, "rb");

            if(!arquivo_clienteB){
                strcpy(buffer,"Erro");
                sendto(socket_clienteB, buffer, strlen(buffer) + 1 , 0, (struct sockaddr *) &endereco_clienteB, tam_struct_clienteB);
            }else{
                strcpy(buffer, "Transferindo");
                sendto(socket_clienteB, buffer, strlen(buffer) + 1 , 0, (struct sockaddr *) &endereco_clienteB, tam_struct_clienteB);
                enviaPacote(msg, arquivo_clienteB, socket_clienteB, endereco_clienteB, tam_struct_clienteB);
           }

           printf("\n\nEnvio do arquivo finalizado sem erros!\n");
           break;
           fclose(arquivo_clienteB);
        }

    }
}
