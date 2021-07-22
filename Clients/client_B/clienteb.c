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
typedef struct pacote
{
    int numseq;         //número de sequência do pacote
    long int check_sum; //valor do checksum do pacote
    int tam;            //tamanho do pacote
    char dados[512];    //segmento do pacote
} pacote;

//função responsável por realizar o checksum
long int checksum(char segmento[], int tamanho)
{

    long int checksum = 0;

    //percorrer o tamanho do segmento
    for (int i = 0; i < tamanho; i++)
    {

        //converte pra inteiro o caracter e soma
        checksum += (int)(segmento[i]);
    }

    //retorna o valor do checksum
    return checksum;
}

//função responsável por transferir o arquivo para o cliente A
int enviaPacote(FILE *arquivo, int socket_clienteB, struct sockaddr_in endereco_clienteA, socklen_t tam_struct_clienteA, char *buffer)
{

    int num_seq = 0;
    pacote pkt;

   
    memset(&pkt, 0x0, sizeof(pkt));

    //enquanto o arquivo não acabar
    while (!feof(arquivo))
    {

        num_seq++;

        //preenche o pacote
        pkt.tam = fread(pkt.dados, 1, 512, arquivo);
        pkt.numseq = num_seq;
        pkt.check_sum = checksum(pkt.dados, pkt.tam);
    
        //envia o pacote
        while (1)
        {
            memset(buffer, '\0', MAX_BUFFER);
            if (sendto(socket_clienteB, &pkt, sizeof(pkt), 0, (struct sockaddr *)&endereco_clienteA, tam_struct_clienteA) < 0)
            {
                printf("Falha ao enviar o pacote de numero de sequencia = %d!\n", num_seq);
                return -1;
            }

        
            if (recvfrom(socket_clienteB, buffer, sizeof(buffer), 0, (struct sockaddr *)&endereco_clienteA, &tam_struct_clienteA) < 0)
            {
                printf("Erro ao receber ack do cliente A!\n");
                return -1;
            }
            
            //só passa para o próximo pacote quando chegar um ACK simbolizado pelo valor "1"
            if(strcmp(buffer, "1") == 0)
                break;
            
        }
    }

    //depois que termina o arquivo, envia um pacote final com tamanho igual a zero
    //indicando para o cliente A que a transferência do arquivo acabou
    pkt.tam = 0;
    strcpy(pkt.dados, "");
    pkt.numseq = 0;
    pkt.check_sum = 0;

    //envia o pacote final para o cliente A
    if (sendto(socket_clienteB, &pkt, sizeof(pkt), 0, (struct sockaddr *)&endereco_clienteA, tam_struct_clienteA) < 0)
    {
        printf("Erro ao tentar enviar o pacote final de confirmacao!\n");
        return -1;
    }
}

int configura_socket()
{
    int socket_clienteB;
    struct sockaddr_in endereco_clienteB;

    socket_clienteB = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_clienteB < 0)
    {
        printf("Criação do socket falhou!\n");
        exit(1);
    }

    memset(&endereco_clienteB, 0, sizeof(endereco_clienteB));
    endereco_clienteB.sin_family = AF_INET;
    endereco_clienteB.sin_port = htons(PORTA_CLIENTEB);

    if (bind(socket_clienteB, (struct sockaddr *)&endereco_clienteB, sizeof(endereco_clienteB)) < 0)
    {
        perror("bind");
        printf("Bind no socket falhou!\n");
        exit(1);
    }

    return socket_clienteB;
}

int verifica_buffer(char *buffer)
{
    if (buffer[0] != '\0')
        return 1;
    return 0;
}

int main()
{

    int socket_clienteB;
    char *buffer;
    struct sockaddr_in endereco_clienteA;
    socklen_t tam_struct_clienteA;
    FILE *arquivo_clienteB;

    buffer = (char *)malloc(MAX_BUFFER * sizeof(char));

    socket_clienteB = configura_socket();
    
    printf("Cliente B online\n\n");

    //Comunicação com o cliente A
    while (1)
    {

        printf("Aguardando solicitações...\n");
        memset(buffer, '\0', MAX_BUFFER);

        tam_struct_clienteA = sizeof(endereco_clienteA);

        recvfrom(socket_clienteB, buffer, MAX_BUFFER, 0, (struct sockaddr *)&endereco_clienteA, &tam_struct_clienteA);

        printf("Mensagem recebida do cliente A: %s\n", buffer);

        if (verifica_buffer(buffer))
        {

            arquivo_clienteB = fopen(buffer, "rb");
            memset(buffer, '\0', MAX_BUFFER);

            if (arquivo_clienteB == NULL)
            {
                buffer[0] = '0';
                sendto(socket_clienteB, buffer, MAX_BUFFER, 0, (struct sockaddr *)&endereco_clienteA, tam_struct_clienteA);
            }
            else
            {
                buffer[0] = '1';
                sendto(socket_clienteB, buffer, MAX_BUFFER, 0, (struct sockaddr *)&endereco_clienteA, tam_struct_clienteA);
                enviaPacote(arquivo_clienteB, socket_clienteB, endereco_clienteA, tam_struct_clienteA, buffer);
            }

            fclose(arquivo_clienteB);
        }
    }
}
