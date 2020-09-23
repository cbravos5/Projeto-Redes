#include "protocol.h"
#include "RawSocket.h"

#define enquadramento 126

//estrutura que define um tratador de sinal
struct sigaction action ;

//estrutura de inicialização to timer
struct itimerval default_timer ;

struct itimerval zero_timer ;

short int time_out = 0;

////////////////////////////////////////////////////////////////////////
//inicia um temporizador de 2 segundos
void init_timer()
{
	setitimer (ITIMER_REAL, &default_timer, 0);
}

////////////////////////////////////////////////////////////////////////
//reseta um temporizador de 2 segundos
void reset_timer()
{
	setitimer (ITIMER_REAL, &default_timer, 0);
}

////////////////////////////////////////////////////////////////////////
//para um temporizador 
void stop_timer()
{
	setitimer (ITIMER_REAL, &zero_timer, 0);
}

////////////////////////////////////////////////////////////////////////
//indica um time_out
void time_out_handler(int signal)
{
	time_out = 1;
}

////////////////////////////////////////////////////////////////////////
//inicia estruturas de time_out
void init_time_handler()
{
	//act handler
	action.sa_handler = time_out_handler;
	sigemptyset (&action.sa_mask) ;
	action.sa_flags = 0 ;
	sigaction (SIGALRM, &action, 0);

	//tick set
	default_timer.it_value.tv_usec = 0;      // primeiro disparo, em micro-segundos
	default_timer.it_value.tv_sec  = 2;      	 	// primeiro disparo, em segundos
	default_timer.it_interval.tv_usec = 0;   // disparos subsequentes, em micro-segundos
	default_timer.it_interval.tv_sec  = 2;   	 	// disparos subsequentes, em segundos

	zero_timer.it_value.tv_usec = 0;      // primeiro disparo, em micro-segundos
	zero_timer.it_value.tv_sec  = 0;      	 	// primeiro disparo, em segundos
	zero_timer.it_interval.tv_usec = 0;   // disparos subsequentes, em micro-segundos
	zero_timer.it_interval.tv_sec  = 0;   	 	// disparos subsequentes, em segundos
	setitimer (ITIMER_REAL, &zero_timer, 0);
}

////////////////////////////////////////////////////////////////////////
//imprime dados e numero das linhas(caso parametro seja passado)
void print_data(unsigned char *data, int tam, short int param, int *n_line)
{
  for (int i = 0; i < tam; ++i)
  {
    printf("%c",(char)data[i]);
    if (!param)
      if(data[i] == '\n')
      {  
        *n_line += 1;
        printf("%d      ",*n_line);
      }
  }
}

////////////////////////////////////////////////////////////////////////
//funcao padrao de receber dados do servidor e imprimir na tela
void print_data_default(int *seq_send,int *seq_recv,int sckt, unsigned char *recv,unsigned char *send,
						 unsigned char *data, short int param1, short int param2, int *n_line, char* data_type)
{
	int n,tam,seq,type;
	init_timer();
	while(1)//leitura e envio de resposta para o servidor
  	{
	    n = read(sckt,recv,19);
	    if (read_env(recv,&tam,data,&seq,&type) == -1)//send nack
	    {
	      send_nack(seq_send,sckt);
	      seq_check(seq_send);
	      reset_timer();
	    }
	    else if (type_check(data_type,type,*seq_recv,seq))//ack
	    {
	      seq_check(seq_recv);
	      send_ack(seq_send,sckt);
	      seq_check(seq_send);
	      print_data(data,tam,param1,n_line);
	      reset_timer();
	      if(param2) printf("  ");
	    }
	    else if(type_check("end",type,*seq_recv,seq))//fim transmissao
	    {
	      seq_check(seq_recv);
	      send_ack(seq_send,sckt);
	      seq_check(seq_send);
	      stop_timer();
	      break;
	    }
	    if(time_out) {write(sckt, send, 19); time_out = 0;}
	}
}

////////////////////////////////////////////////////////////////////////
//funcao padrão de receber final de dados
void send_final_data(int *seq_send,int *seq_recv,int sckt, unsigned char *recv, unsigned char *send)
{
  int n,type,seq,tam;
  send = make_env((unsigned char)13,NULL,*seq_send,send);
  n = write(sckt, send, 19);
  seq_check(seq_send);
  //esperar pela mensgaem de confirmacao
  init_timer();
  while(1)
    {
      n = read(sckt,recv,19);
      if (read_env(recv,&tam,NULL,&seq,&type) == -1)
      {
        seq_check(seq_recv);
        return;
      }
      else if(type_check("NACK",type,*seq_recv,seq))
      {
        seq_check(seq_recv);
        n = write(sckt, send, 19);
        reset_timer();
      }
      else if(type_check("ACK",type,*seq_recv,seq))
      {
        seq_check(seq_recv);
        stop_timer();
        break;
      }
      if(time_out) {write(sckt, send, 19); time_out = 0;}
    }
}

////////////////////////////////////////////////////////////////////////
//limpa um buffer de dados
void clear_data(unsigned char * data)
{
	for (int i = 0; i < 15; ++i){data[i] = '\0';}
}

////////////////////////////////////////////////////////////////////////
//envia um ack
void send_ack(int *seq, int sckt)
{
	int n;
	unsigned char *msg = malloc(4);
	msg = make_env((unsigned char)8,NULL,*seq,msg);
	n = write(sckt, msg, 19);
	free(msg);
}

////////////////////////////////////////////////////////////////////////
//envia um nack
void send_nack(int *seq, int sckt)
{
	int n;
	unsigned char *msg = malloc(4);
	msg = make_env((unsigned char)9,NULL,*seq,msg);
	n = write(sckt, msg, 19);
	free(msg);
}

////////////////////////////////////////////////////////////////////////
//verificacao de tipo e sequencia
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
//impressao de erro
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
//atualiza sequencia
void seq_check(int *seq)
{
  if(*seq == 255)
    *seq = 0;
  else
    *seq = *seq + 1;
}

////////////////////////////////////////////////////////////////////////
//cria um envelope de até 19 bytes
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
	//coloca 2 inteiros de 4 bits em um byte
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

////////////////////////////////////////////////////////////////////////
//lê um envelope de até 19 bytes
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














