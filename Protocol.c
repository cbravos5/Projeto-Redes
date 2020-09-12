#include "protocol.h"
#include "RawSocket.h"

#define enquadramento 126


void clear_data(unsigned char * data)
{
	for (int i = 0; i < 15; ++i){data[i] = '\0';}
}
////////////////////////////////////////////////////////////////////////

uint16_t unpack(uint8_t *data, int tam)
{
        uint16_t val;
        if (tam == 1)
        {
        	val = (uint16_t)data[0];
        }
        else
        {
        	val = (uint16_t)data[0] << 8 |
            	  (uint16_t)data[1];
        }
        return val;
}

////////////////////////////////////////////////////////////////////////

void send_ack(int *seq, int sckt)
{
	int n;
	unsigned char *msg = malloc(4);
	msg = make_env((unsigned char)8,NULL,*seq,msg);
	n = write(sckt, msg, 19);
	free(msg);
  	if(n < 0)
  	{
    	printf("ERRO AO ENVIAR MENSAGEM\n");
    	return;
  	}
}

////////////////////////////////////////////////////////////////////////

void send_nack(int *seq, int sckt)
{
	int n;
	unsigned char *msg = malloc(4);
	msg = make_env((unsigned char)9,NULL,*seq,msg);
	n = write(sckt, msg, 19);
	free(msg);
  	if(n < 0)
  	{
    	printf("ERRO AO ENVIAR MENSAGEM\n");
    	return;
  	}
}

////////////////////////////////////////////////////////////////////////

int type_check(char *check,int type_r,int seq_recv, int seq)
{
  if(seq != seq_recv)
    return 0;

  int type;
  if(strcmp(check,"cd") == 0) type = 0;
  else if(strcmp(check,"ls") == 0) type = 1;
  else if(strcmp(check,"ver") == 0) type = 2;
  else if(strcmp(check,"linha") == 0) type = 3;
  else if(strcmp(check,"linhas") == 0) type = 4;
  else if(strcmp(check,"edit") == 0) type = 5;
  else if(strcmp(check,"ACK") == 0) type = 8;
  else if(strcmp(check,"NACK") == 0) type = 9;
  else if(strcmp(check,"start_end") == 0) type = 10;
  else if(strcmp(check,"ls_data") == 0) type = 11;
  else if(strcmp(check,"arch_data") == 0) type = 12;
  else if(strcmp(check,"end") == 0) type = 13;
  else if(strcmp(check,"erro") == 0) type = 15;

  if(type == type_r)
    return 1;
  return 0;
}


////////////////////////////////////////////////////////////////////////

void error(unsigned char *data)
{
  switch ((int)(data[0]-'0'))
  {
  	case 1: printf("Acesso negado\n"); break;
  	case 2: printf("Diretorio inexistente\n"); break;
  	case 3: printf("Arquivo inexistente\n"); break;
  	case 4: printf("Linha inexistente\n"); break;
  }
}

//////////////////////////////////////////////////////////////////////// 

void seq_check(int *seq)
{
  if(*seq == 255)
    *seq = 0;
  else
    *seq = *seq + 1;
}

////////////////////////////////////////////////////////////////////////

unsigned char * make_env(unsigned char type, unsigned char *data, unsigned char seq, unsigned char *env)
{
	unsigned char raw_byte; //byte que recebera 4 bits do tipo e 4 bits do tamanho 
	unsigned char tam;
	unsigned char enq;
	unsigned char parity;

	//Atribuicoes iniciais
	enq = enquadramento;
	if(data != NULL)
		tam = strlen(data);
	else
		tam = 0; 
	raw_byte = tam << 4;
	raw_byte = raw_byte | type;

	//calculo paridade vertical 
	parity = enq ^ raw_byte ^ seq;
	for(int i = 0; i < (int)tam; i++) {parity = parity ^ data[i];}

	//atribuicoes do ponteiro
	env[0] = enq;
	env[1] = raw_byte;
	env[2] = seq;
	for (int i = 0; i < (int)tam; ++i)
	{
		env[3+i] = data[i];
	}
	env[3+tam] = parity;
	
	
	return env;
}


//Retorna -1 se houve algum erro, 0 caso contrario
int read_env(unsigned char *msg, int *tam, unsigned char *data, int *seq, int *type)
{
	unsigned char enq;
	unsigned char parity;

	//verificar se o enquadramento eh valido
	enq = msg[0];
	if (enq != enquadramento)
		return -1;

	//atribuicao do tamanho, tipo e sequencia
	*tam = (msg[1] >> 4);
	*type = (unsigned char)(msg[1] << 4) >> 4;
	*seq = (int)msg[2]; //convert char to int

	//verificacao de paridade. (utiliza um loop para caso tenha um campo de dados)
	parity = msg[0] ^ msg[1] ^ msg[2];
	for(int i = 0; i < *tam; i++) {parity = parity ^ msg[3+i];}
	if (parity != msg[3+*tam])
		return -1;

	//caso exista um campo de dados na msg data eh atribuida
	if(data != NULL){
		clear_data(data);
		for (int i = 0; i < *tam; ++i) {data[i] = msg[3+i];}
	}

	return 0;
}














