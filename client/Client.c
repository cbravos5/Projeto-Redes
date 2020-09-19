#include "../RawSocket.h"
#include "../protocol.h"
#include <time.h>
#include <string.h>
#include <unistd.h>


int n_line = 1;
int seq_send_c = 0;
int seq_recv_c = 0;

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

void print_data(unsigned char *data, int tam)
{
  for (int i = 0; i < tam; ++i)
  {
    printf("%c",(char)data[i]);
    if(data[i] == '\n')
    {  
      n_line++;
      printf("%d      ",n_line);
    }
  }
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
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }

  seq_check(&seq_send_c);
  
  while(1)
  { 
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
      return;
    else if (type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
  }
} 

//////////////////////////////////////////////////////////////////////// 

void ls(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;

  send = make_env((unsigned char)1,NULL,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }

  seq_check(&seq_send_c);
  
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
      print_data(data,tam);
      printf("  ");
      break;
    }
  }

  
  while(1)
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if (type_check("ls_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      print_data(data,tam);
      printf("  ");
    }
    else if(type_check("end",type,seq_recv_c,seq))//fim transmissao
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      break;
    }
  }
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
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);


  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
      print_data(data,tam);
      break;
    }
  }

  while(1)//leitura e envio de resposta para o servidor
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      print_data(data,tam);
    }
    else if(type_check("end",type,seq_recv_c,seq))//fim transmissao
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      break;
    }
  }
  n_line = 1;
  printf("\n");
} 

void edit(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  unsigned int line;
  char text[255];
  char arq[15];
  char aux_data[15];
  //
  scanf("%d %s",&line,arq);
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
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);

  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
      break;
    }
  }

  //envia linha
  memcpy(data,(char*)&line,2);
  send = make_env((unsigned char)10,data,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);

  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
      break;
    }
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
    if(n < 0)
    {
      printf("ERRO AO ENVIAR MENSAGEM\n");
      return;
    }
    seq_check(&seq_send_c);
    //loop de espera de resposta do server
    while(1)
    {
      n = read(sckt,recv,19);
      if (n < 0)
      {
        printf("ERROR reading from socket");
        return;
      }
      if (read_env(recv,&tam,NULL,&seq,&type) == -1)
      {
        seq_check(&seq_recv_c);
        printf("ERRO AO DESENVELOPAR MENSAGEM\n");
        return;
      }
      else if(type_check("NACK",type,seq_recv_c,seq))
      {
        seq_check(&seq_recv_c);
        n = write(sckt, send, 19);
        if(n < 0)
        {
          printf("ERRO AO ENVIAR MENSAGEM\n");
          return;
        }
      }
      else if(type_check("ACK",type,seq_recv_c,seq))
      {
        seq_check(&seq_recv_c);
        break;
      }
    }
  }
 
  //enviar final de dados
  send = make_env((unsigned char)13,NULL,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);
  //esperar pela mensagem de confirmacao
  while(1)
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,NULL,&seq,&type) == -1)
    {
      seq_check(&seq_recv_c);
      printf("ERRO AO DESENVELOPAR MENSAGEM\n");
      return;
    }
    else if(type_check("NACK",type,seq_recv_c,seq))
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
    }
    else if(type_check("ACK",type,seq_recv_c,seq))
    {
      seq_check(&seq_recv_c);
      break;
    }
  } 
}

void linha(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  unsigned int line;
  char arq[15];
  char aux_data[15];
  //
  scanf("%d %s",&line,arq);
  send = make_env((unsigned char)3,arq,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);

  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
      break;
    }
  }

  //envia linha
  memcpy(data,(char*)&line,2);
  send = make_env((unsigned char)10,data,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);

  while(1)//Le enquanto nao recebe uma confirmacao ou nack
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      break;
    }
  }

  if(tam == 0)
    return;
  printf("%s",(char *)data);


  while(1)//leitura e envio de resposta para o servidor
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      printf("%s",(char *)data);
    }
    else if(type_check("end",type,seq_recv_c,seq))//fim transmissao
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      break;
    }
  }
}

void linhas(int sckt, unsigned char *send,unsigned char *data , unsigned char *recv)
{
  int n,tam,seq,type;
  unsigned char line_start[7],line_end[7];
  char arq[15];
  char aux_data[15];
  //
  scanf("%s %s %s",line_start,line_end,arq);
  send = make_env((unsigned char)4,arq,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);
 
  while(1)//Le enquanto nao recebe uma confirmacao, nack ou erro
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
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
      break;
    }
  }

  //envia linha
  unsigned char lines[15];
  strcpy(lines,line_start);
  strcat(lines,"/");
  strcat(lines,line_end);
  send = make_env((unsigned char)10,lines,seq_send_c,send);
  n = write(sckt, send, 19);
  if(n < 0)
  {
    printf("ERRO AO ENVIAR MENSAGEM\n");
    return;
  }
  seq_check(&seq_send_c);

  while(1)//Le enquanto nao recebe uma confirmacao ou nack
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if(type_check("NACK",type,seq_recv_c,seq)) //NACK
    {
      seq_check(&seq_recv_c);
      n = write(sckt, send, 19);
      if(n < 0)
      {
        printf("ERRO AO ENVIAR MENSAGEM\n");
        return;
      }
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      break;
    }
  }

  if(tam == 0)
    return;
  n_line = atoi(line_start);
  printf("%d      ",n_line);
  print_data(data,tam);


  while(1)//leitura e envio de resposta para o servidor
  {
    n = read(sckt,recv,19);
    if (n < 0)
    {
      printf("ERROR reading from socket");
      return;
    }
    if (read_env(recv,&tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
    }
    else if (type_check("arch_data",type,seq_recv_c,seq))//ack
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      print_data(data,tam);
    }
    else if(type_check("end",type,seq_recv_c,seq))//fim transmissao
    {
      seq_check(&seq_recv_c);
      send_ack(&seq_send_c,sckt);
      seq_check(&seq_send_c);
      break;
    }
  }
  n_line = 1;
  printf("\n");
}

void main(void)
{
  unsigned char *send = malloc(19);
  unsigned char *recv = malloc(19);
  unsigned char *data = malloc(15);
  int sckt = ConexaoRawSocket("lo");
  char command[6];
  //int sckt = ConexaoRawSocket("lo");
  while(1)
  {
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

  







