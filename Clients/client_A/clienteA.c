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

typedef struct mensagem
{
    int porta_cliente;
    char arquivo[20];
} mensagem;

typedef struct pacote
{
    int numseq;
    long int check_sum;
    int tam;
    char dados[512];
} pacote;

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

//Funções para transformar char em int
int powInt(int x, int y)
{
    for (int i = 0; i < y; i++)
        x *= 10;

    return x;
}

int parseInt(char *chars)
{
    int sum = 0;
    int len = strlen(chars);
    for (int x = 0; x < len; x++)
    {
        int n = chars[len - (x + 1)] - '0';
        sum = sum + powInt(n, x);
    }
    return sum;
}

// Função para receber as mensagens
void recebeMensagem(int sd, struct sockaddr_in remoteAddr, char *buffer)
{
    int rc;
    socklen_t addrlen = sizeof(remoteAddr);
    
    while (1)
    {
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
    socklen_t addrlen = sizeof(remoteAddr);

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

void recebePacote(int sd, struct sockaddr_in remoteAddr, char *nomearq) {
    
    pacote pkt;
    FILE * arq;
    int rc, cont = 0;
    char ack[] = "1";
    char nak[] = "0";

    //Lê e escreve no arquivo em modo binário
    arq = fopen(nomearq, "wb");
    if(arq == NULL)
    {
        printf("Error - Arquivo não pôde ser criado\n");
        exit(1);
    }
    
    socklen_t addrlen = sizeof(remoteAddr);  

    //recebe pacote do cliente B
    while (1)
    {
        memset(&pkt, 0, sizeof(pacote));
    
        rc = recvfrom(sd, &pkt, sizeof(pkt), 0, (struct sockaddr *) &remoteAddr, &addrlen);
        if (rc == -1)
        {
            perror("Error");
            exit(1);
        }

        if(pkt.tam == 0)
            break;

        //cheksum corresponde
        if(checksum(pkt.dados, pkt.tam) == pkt.check_sum && pkt.numseq == cont + 1)
        {
            fwrite(pkt.dados, 1, pkt.tam, arq);
            sendto(sd, ack, strlen(ack), 0, (struct sockaddr *) &remoteAddr, addrlen);
            cont++;
        }
        //arquivo corrompeu no caminho
        else
            sendto(sd, nak, strlen(ack), 0, (struct sockaddr *) &remoteAddr, addrlen);
    }

    fclose(arq);
}

int main(int argc, char *argv[])
{

    int sd;
    int PORTA_CLIENTE_B;
    struct sockaddr_in remoteServAddr;
    struct sockaddr_in remoteClientB;

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
        PORTA_CLIENTE_B = parseInt(&buffer[1]);
        printf("O cliente na porta %d possui o arquivo.\n", PORTA_CLIENTE_B);
        memset(buffer, '\0', SIZE);
    }
    else
    {
        printf("Error - Arquivo não encontrado na base de dados.\n");
        exit(1);
    }

    //Comunicação com o cliente B
    //zerando a struct
    memset(&remoteClientB, 0, sizeof(remoteClientB));

    //struct com dados do cliente B
    remoteClientB.sin_family = AF_INET;
    remoteClientB.sin_port = htons(PORTA_CLIENTE_B);
    remoteClientB.sin_addr.s_addr = inet_addr(IP_LOCAL);

    strcpy(buffer, argv[1]);
    //envia uma requisição com o nome do arquivo para o cliente B
    enviaMensagem(sd, remoteClientB, buffer, 1);

    recebePacote(sd, remoteClientB, buffer);

    //Avisando o servidor que o cliente A também possui o arquivo
    strcpy(buffer, argv[1]);
    enviaMensagem(sd, remoteServAddr, buffer, 2);

    memset(buffer, '\0', SIZE);
    recebeMensagem(sd, remoteServAddr, buffer);
    if (buffer[0] == '1')
        printf("Cliente A agora presente na base de dados\n");

    free(buffer);
    return 0;
}