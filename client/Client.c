#include "../RawSocket.h"
#include "../protocol.h"
#include <time.h>
#include <string.h>
#include <unistd.h>


int n_line = 1;
int seq_send_c = 0;
int seq_recv_c = 0;
extern short int time_out;

////////////////////////////////////////////////////////////////////////
int get_text(unsigned char *data, char *text, int k)
{
  clear_data(data);
  int j = 0;
  for (int i = 15*k; i < (15*k)+15 ; ++i)
  {
    if(i >= strlen(text))
      return 0;
    data[j] = text[i];
    j++;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////

void cd(int sckt, unsigned char *send, unsigned char *data, unsigned char *recv)
{
  int n,tam,seq,type;
  char dir_path[255];
  scanf("%s",dir_path);
  if(strlen(dir_path) > 15)
  {
    printf("ERRO: TAMANHO DO CAMINHO MAIOR QUE 15 BYTES\n");
    return;
  }
  send = make_env((unsigned char)0,dir_path,seq_send_c,send);
  n = write(sckt, send, 19);

  seq_check(&seq_send_c);
  
  init_timer();
  while(1)
  { 
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
      return;
    else if (type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      error(data);
      seq_check(&seq_recv_c);
      return;
    }
    else if(type_check("ACK",type,seq_recv_c,seq)) //ack
    {
      seq_check(&seq_recv_c);
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }
} 

//////////////////////////////////////////////////////////////////////// 

void ls(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;

  send = make_env((unsigned char)1,NULL,seq_send_c,send);
  n = write(sckt, send, 19);

  seq_check(&seq_send_c);
  
  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      seq_check(&seq_recv_c);
      error(data);
      return;
    }
    else if (type_check("ls_data",type,seq_recv_c,seq))
    { 
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      print_data(data,tam,1,NULL);
      printf("  ");
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  
  print_data_default(&seq_send_c,&seq_recv_c,sckt,recv,send,data,1,1,NULL,"ls_data");
  printf("\n");
} 


////////////////////////////////////////////////////////////////////////

void lcd(void)
{
  char dir_path[255];
  scanf("%s",dir_path);
  if(chdir(dir_path) == -1)
    printf("%s\n", strerror(errno));
}

////////////////////////////////////////////////////////////////////////

void lls(void)
{
  struct dirent *de; 
  
    
  DIR *dr = opendir("."); 
  
  if (dr == NULL)  // opendir returns NULL if couldn't open directory 
  { 
      printf("Could not open current directory" ); 
       return; 
  } 
  
  while ((de = readdir(dr)) != NULL) 
    printf("%s  ", de->d_name); 

  printf("\n");
  
  closedir(dr);     
}

/////////////////////////////////////////////////////////////////////////

void ver(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  char arq[255];
  scanf("%s",arq);
  if(strlen(arq) > 15)
  {
    printf("ERRO: NOME DO ARQUIVO MAIOR QUE 15 BYTES\n");
    return;
  }
  send = make_env((unsigned char)2,arq,seq_send_c,send);
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);

  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      seq_check(&seq_recv_c);
      error(data);
      return;
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))
    { 
      printf("%d      ",n_line);
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      print_data(data,tam,0,&n_line);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  print_data_default(&seq_send_c,&seq_recv_c,sckt,recv,send,data,0,0,&n_line,"arch_data");
  n_line = 1;
  printf("\n");
} 

/////////////////////////////////////////////////////////////////////////

void edit(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  unsigned char line[15];
  char text[255];
  char arq[15];
  char aux_data[15];
  //
  scanf("%s %s",line,arq);
  fgetc(stdin); fgetc(stdin);
  //
  for (int i = 0; i < 255; ++i) text[i] = '\0';
  char c; scanf("%c",&c);
  int k = 0;
  while(c != '"')
  {
    text[k] = c;
    scanf("%c",&c);
    k++;
  }

  //envia comando de edit + nome do arquivo
  send = make_env((unsigned char)5,arq,seq_send_c,send);
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);

  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      seq_check(&seq_recv_c);
      error(data);
      return;
    }
    else if (type_check("ACK",type,seq_recv_c,seq))//ACK
    { 
      seq_check(&seq_recv_c);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  //envia linha
  send = make_env((unsigned char)10,line,seq_send_c,send);
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);

  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      seq_check(&seq_recv_c);
      error(data);
      return;
    }
    else if (type_check("ACK",type,seq_recv_c,seq))//ACK
    { 
      seq_check(&seq_recv_c);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  k = 0;
  int end = 1; //flag para sinalizar fim do texto
  //envio dos dados do texto
  while (end)
  {
    end = get_text(data,text,k);
    k++;
    send = make_env((unsigned char)12,data,seq_send_c,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_c);

    init_timer();
    //loop de espera de resposta do server
    while(1)
    {
      n = read(sckt,recv,19);
      if (read_env(recv,&tam,NULL,&seq,&type) == -1)
      {
        seq_check(&seq_recv_c);
        return;
      }
      else if(type_check("NACK",type,seq_recv_c,seq))
      {
        seq_check(&seq_recv_c);
        n = write(sckt, send, 19);
        reset_timer();
      }
      else if(type_check("ACK",type,seq_recv_c,seq))
      {
        seq_check(&seq_recv_c);
        stop_timer();
        break;
      }
      if(time_out) {write(sckt, send, 19); time_out = 0;}
    }
  }
 
  //enviar final de dados
  send_final_data(&seq_send_c,&seq_recv_c,sckt,recv,send);
}

/////////////////////////////////////////////////////////////////////////

void linha(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  unsigned char line[15];
  char arq[15];
  char aux_data[15];
  //envia nome do arquiuvo + comando
  scanf("%s %s",line,arq);
  send = make_env((unsigned char)3,arq,seq_send_c,send);
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);

  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      seq_check(&seq_recv_c);
      error(data);
      return;
    }
    else if (type_check("ACK",type,seq_recv_c,seq))//ACK
    { 
      seq_check(&seq_recv_c);
      stop_timer();
      break;
    }
  }

  //envia linha
  send = make_env((unsigned char)10,line,seq_send_c,send);
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);

  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao ou nack
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  if(tam == 0)
    return;
  print_data(data,tam,1,&n_line);
  print_data_default(&seq_send_c,&seq_recv_c,sckt,recv,send,data,1,0,NULL,"arch_data");
}

/////////////////////////////////////////////////////////////////////////

void linhas(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  unsigned char line_start[7],line_end[7];
  char arq[15];
  char aux_data[15];
  //
  scanf("%s %s %s",line_start,line_end,arq);
  send = make_env((unsigned char)4,arq,seq_send_c,send);//envia nome do arquivo + comando
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);
 
  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("erro",type,seq_recv_c,seq)) //erro
    {
      seq_check(&seq_recv_c);
      error(data);
      return;
    }
    else if (type_check("ACK",type,seq_recv_c,seq))//ACK
    { 
      seq_check(&seq_recv_c);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  //envia linha
  unsigned char lines[15];
  strcpy(lines,line_start);
  strcat(lines,"/");//separa as linhas por um /
  strcat(lines,line_end);
  send = make_env((unsigned char)10,lines,seq_send_c,send);
  n = write(sckt, send, 19);
  seq_check(&seq_send_c);

  init_timer();
  while(1)//Le enquanto nao recebe uma confirmacao ou nack
  {
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      reset_timer();
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      reset_timer();
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  if(tam == 0)
    return;
  n_line = atoi(line_start);
  printf("%d      ",n_line);
  print_data(data,tam,0,&n_line);
  print_data_default(&seq_send_c,&seq_recv_c,sckt,recv,send,data,0,0,&n_line,"arch_data");
  n_line = 1;
  printf("\n");
}

/////////////////////////////////////////////////////////////////////////

void main(void)
{
  unsigned char *send = malloc(19);
  unsigned char *recv = malloc(19);
  unsigned char *data = malloc(15);
  int sckt = ConexaoRawSocket("lo");
  char command[6];
  init_time_handler();
  while(1)
  {
    stop_timer();
    printf("client@terminal---->");
    scanf("%s",command);
    if(strcmp(command,"lcd") == 0) lcd();
    else if(strcmp(command,"lls") == 0) lls();
    else if(strcmp(command,"exit") == 0) break;
    else if(strcmp(command,"clear") == 0) system("clear");
    else if(strcmp(command,"cd") == 0) cd(sckt,send,data,recv);
    else if(strcmp(command,"ls") == 0) ls(sckt,send,data,recv);
    else if(strcmp(command,"ver") == 0) ver(sckt,send,data,recv);
    else if(strcmp(command,"edit") == 0) edit(sckt,send,data,recv);
    else if(strcmp(command,"linha") == 0) linha(sckt,send,data,recv);
    else if(strcmp(command,"linhas") == 0) linhas(sckt,send,data,recv);
  }
} 

  







