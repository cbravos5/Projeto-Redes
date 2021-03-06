#include "../RawSocket.h"
#include "../protocol.h"
#include <string.h>
#include <unistd.h>




int seq_send_s = 0;
int seq_recv_s = 0;
extern short int time_out;

//////////////////////////////////////////////////////////////////////////////////

char *readLine (FILE *infile)
{
   int n = 0, size = 128, ch;
   char *line;
   line = malloc (size + 1);
   while ((ch = getc (infile)) != '\n' && ch != EOF) {
      if (n == size) {
         size *= 2;
         line = realloc (line, size + 1);
      }
      line[n++] = ch;
   }
   if (n == 0 && ch == EOF) {
      free (line);
      return NULL;
   }
   line[n] = '\0';
   line = realloc (line, n + 1);
   return line;
}

//////////////////////////////////////////////////////////////////////////////////

int read_arch(unsigned char *data, FILE *fp)
{
  char ch;
  for (int i = 0; i < 15; ++i)
  {
    ch = fgetc(fp);
    if(ch == EOF)
      return 0;
    data[i] = ch;
  }
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////

void cd_s(int sckt,unsigned char *data, int *tam, unsigned char *send, unsigned char *recv)
{
  printf("dale\n");
  seq_check(&seq_recv_s);
  int n,type,seq;
  if(chdir((char *)data) == -1)
  {
    if(errno == 13)
      send = make_env((unsigned char)15,"1",seq_send_s,send);
    else
      send = make_env((unsigned char)15,"2",seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  }
  send_ack(&seq_send_s,sckt);
  seq_check(&seq_send_s);
}

//////////////////////////////////////////////////////////////////////////////////

void ls_s(int sckt, unsigned char *data, int *tam, unsigned char *send, unsigned char *recv)
{
  printf("dale\n");
  seq_check(&seq_recv_s);
  int n,type,seq;
  struct dirent *de; 
    
  DIR *dr = opendir("."); 
  
  if (dr == NULL)  // opendir returns NULL if couldn't open directory 
  { 
    send = make_env((unsigned char)15,"2",seq_send_s,send);
    n = write(sckt, send, 19);
    return;
  } 
  

  //envia nomes de arquivos e diretorios com nomes menores que 15 bytes
  while ((de = readdir(dr)) != NULL) 
  {
    if(strlen(de->d_name) <= 15)
    {
      send = make_env((unsigned char)11,de->d_name,seq_send_s,send);
      n = write(sckt, send, 19);
      seq_check(&seq_send_s);
      init_timer();
      while(1)
      {
        n = read(sckt,recv,19);
        if (read_env(recv,tam,NULL,&seq,&type) == -1)
        {
          seq_check(&seq_recv_s);
          return;
        }
        else if(type_check("NACK",type,seq_recv_s,seq))
        {
          seq_check(&seq_recv_s);
          n = write(sckt, send, 19);
          reset_timer();
        }
        else if(type_check("ACK",type,seq_recv_s,seq))
        {
          stop_timer();
          seq_check(&seq_recv_s);
          break;
        }
        if(time_out) {write(sckt, send, 19); time_out = 0;}
      }
    }  
  }

  //enviar final de dados
  send_final_data(&seq_send_s,&seq_recv_s,sckt,recv,send);
}

//////////////////////////////////////////////////////////////////////////////////

void ver_s(int sckt, unsigned char *data, int *tam, unsigned char *send, unsigned char *recv)
{
  printf("dale\n");
  seq_check(&seq_recv_s);
  int n,type,seq;
  
  FILE *fp = fopen(data,"r");

  if (fp == NULL)  // fopen returns NULL if has error
  { 
    if(errno == 13)
      send = make_env((unsigned char)15,"1",seq_send_s,send);
    else
      send = make_env((unsigned char)15,"3",seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  } 

  int end = 1; //flag para sinalizar fim do arquivo
  //envio dos dados do arquivo aberto
  while (end)
  {
    clear_data(data);
    end = read_arch(data,fp);
    send = make_env((unsigned char)12,data,seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    init_timer();
    //loop de espera de resposta do cliente
    while(1)
    {
      n = read(sckt,recv,19);
      if (read_env(recv,tam,NULL,&seq,&type) == -1)
      {
        seq_check(&seq_recv_s);
        return;
      }
      else if(type_check("NACK",type,seq_recv_s,seq))
      {
        seq_check(&seq_recv_s);
        n = write(sckt, send, 19);
        reset_timer();
      }
      else if(type_check("ACK",type,seq_recv_s,seq))
      {
        seq_check(&seq_recv_s);
        stop_timer();
        break;
      }
      if(time_out) {write(sckt, send, 19); time_out = 0;}
    }
  }


  //enviar final de dados
  send_final_data(&seq_send_s,&seq_recv_s,sckt,recv,send);
  fclose(fp);
}  

//////////////////////////////////////////////////////////////////////////////////

void edit_s(int sckt, unsigned char *data, int *tam, unsigned char *send, unsigned char *recv)
{
  printf("dale\n");
  seq_check(&seq_recv_s);
  int n,type,seq;
  unsigned int line;
  char filename[15];
  for (int i = 0; i < strlen((char*)data); ++i)
  {
    filename[i] = '\0';
    filename[i] = data[i];
  }

  FILE *fp = fopen(data,"r");
  
  if (fp == NULL)  // fopen returns NULL if has error
  { 
    if(errno == 13)
      send = make_env((unsigned char)15,"1",seq_send_s,send);
    else
      send = make_env((unsigned char)15,"3",seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  } 
  send_ack(&seq_send_s,sckt);
  seq_check(&seq_send_s);


  init_timer();
  while(1)//espera dados do cliente
  {
    n = read(sckt,recv,19);
    if (read_env(recv,tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
      reset_timer();
    }
    else if (type_check("start_end",type,seq_recv_s,seq))//ack
    {
      seq_check(&seq_recv_s);
      line = atoi(data);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }
  /*Arquivo temporario é criado para passar dados do arquivo aberto para ele.
    Caso a linha seja válida, os dados escritos em temp_data param de vir do arquivo
  e passam a vir do client; após o término dos dados do client, as demais linhas do arquvo são escritas
  em temp_data e ao final o arquivo é removido, sendo assim temp_data tem o nome alterado
  para o nome do arquivo original.*/
  FILE *fp_s = fopen("temp_data.txt","w");

  char *readed = malloc(255);
  int act_line = 1;
  size_t len = 0;
  ssize_t rd;
  while(act_line != line && (rd = getline(&readed, &len, fp)) != -1)
  {
    fprintf(fp_s,"%s",readed);
    act_line++;
  }
  if (read == NULL || fgetc(fp) == EOF)//caso especifico ultima linha + 1 (fgetc)
  {
    free(readed);
    fclose(fp);
    fclose(fp_s);
    remove("temp_data.txt");
    send = make_env((unsigned char)15,"4",seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  }
  send_ack(&seq_send_s,sckt);
  seq_check(&seq_send_s);

  
  init_timer();
  while(1)//leitura e envio de resposta para o cliente
  {
    clear_data(data);
    n = read(sckt,recv,19);
    if (read_env(recv,tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
      reset_timer();
    }
    else if (type_check("arch_data",type,seq_recv_s,seq))//ack
    {
      seq_check(&seq_recv_s);
      send_ack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
      fprintf(fp_s,"%s",data);
      reset_timer();
    }
    else if(type_check("end",type,seq_recv_s,seq))//fim transmissao
    {
      seq_check(&seq_recv_s);
      send_ack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
      fprintf(fp_s,"%c",'\n');
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }

  getline(&readed, &len, fp);
  while((rd = getline(&readed, &len, fp)) != -1)
  {
   fprintf(fp_s,"%s",readed); 
  }

  free(readed);
  fclose(fp);
  fclose(fp_s);
  remove(filename);
  rename("temp_data.txt",filename);
}

//////////////////////////////////////////////////////////////////////////////////

void linha_s(int sckt, unsigned char *data, int *tam, unsigned char *send, unsigned char *recv)
{
  printf("dale\n");
  seq_check(&seq_recv_s);
  int n,type,seq;
  unsigned int line;
  char filename[15];
  for (int i = 0; i < strlen((char*)data); ++i)
  {
    filename[i] = '\0';
    filename[i] = data[i];
  }

  FILE *fp = fopen(data,"r");
  
  if (fp == NULL)  // fopen returns NULL if has error
  { 
    if(errno == 13)
      send = make_env((unsigned char)15,"1",seq_send_s,send);
    else
      send = make_env((unsigned char)15,"3",seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  } 
  send_ack(&seq_send_s,sckt);
  seq_check(&seq_send_s);


  init_timer();
  while(1)//espera dados do cliente
  {
    n = read(sckt,recv,19);
    if (read_env(recv,tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
      reset_timer();
    }
    else if (type_check("start_end",type,seq_recv_s,seq))//ack
    {
      seq_check(&seq_recv_s);
      line = atoi(data);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }


  char *readed = malloc(255);
  int act_line = 1;
  size_t len = 0;
  ssize_t rd;
  while(act_line != line && (rd = getline(&readed, &len, fp)) != -1)
  {
    act_line++;
  }
  if (read == NULL)
  {
    free(readed);
    fclose(fp);
    send = make_env((unsigned char)12,NULL,seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  }


  //caso especifico na ultima linha + 1; é necessaria verificacao
  //e torna código mais complicado
  char ch;
  int end = 1;
  int err = 1;
  while(end)
  {
    clear_data(data);

    for (int i = 0; i < 15; ++i)
    {
      ch = fgetc(fp);
      if(ch == EOF) //so ocorre caso seja ultima linha
      {
        end = err = 0;
        break;
      }
      if(ch == '\n')
      {
        data[i] = ch;
        end = 0;
        break;
      }
      data[i] = ch;
    }
    if(err == 0)
      break;
    send = make_env((unsigned char)12,data,seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    init_timer();
    //loop de espera de resposta do cliente
    while(1)
    {
      n = read(sckt,recv,19);
      if (read_env(recv,tam,NULL,&seq,&type) == -1)
      {
        seq_check(&seq_recv_s);
        return;
      }
      else if(type_check("NACK",type,seq_recv_s,seq))
      {
        seq_check(&seq_recv_s);
        n = write(sckt, send, 19);
        reset_timer();
      }
      else if(type_check("ACK",type,seq_recv_s,seq))
      {
        seq_check(&seq_recv_s);
        stop_timer();
        break;
      }
      if(time_out) {write(sckt, send, 19); time_out = 0;}
    }
  }

  if(err)
  {
    //enviar final de dados
    send_final_data(&seq_send_s,&seq_recv_s,sckt,recv,send);
    fclose(fp);
  }
  else
  {
    send = make_env((unsigned char)12,NULL,seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  }
}

//////////////////////////////////////////////////////////////////////////////////

void linhas_s(int sckt, unsigned char *data, int *tam, unsigned char *send, unsigned char *recv)
{
  printf("dale\n");
  seq_check(&seq_recv_s);
  int n,type,seq;
  unsigned int line_start,line_end;
  char filename[15];
  for (int i = 0; i < strlen((char*)data); ++i)
  {
    filename[i] = '\0';
    filename[i] = data[i];
  }

  FILE *fp = fopen(data,"r");
  
  if (fp == NULL)  // fopen returns NULL if has error
  { 
    if(errno == 13)
      send = make_env((unsigned char)15,"1",seq_send_s,send);
    else
      send = make_env((unsigned char)15,"3",seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  } 
  send_ack(&seq_send_s,sckt);
  seq_check(&seq_send_s);


  init_timer();
  while(1)//espera dados do cliente
  {
    n = read(sckt,recv,19);
    if (read_env(recv,tam,data,&seq,&type) == -1)//send nack
    {
      send_nack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
      reset_timer();
    }
    else if (type_check("start_end",type,seq_recv_s,seq))//ack
    {
      seq_check(&seq_recv_s);
      unsigned char data1[7],data2[7];
      clear_data(data1);
      clear_data(data2);
      int j = 0;
      while(data[j] != '/')
      {
        data1[j] = data[j];
        j++;
      }
      int l = 0;
      while(j + l + 1 < *tam)
      {
        data2[l] = data[j + l + 1];
        l++;
      }
      line_start = atoi(data1);
      line_end = atoi(data2);
      stop_timer();
      break;
    }
    if(time_out) {write(sckt, send, 19); time_out = 0;}
  }


  char *readed = malloc(255);
  int act_line = 1;
  size_t len = 0;
  ssize_t rd;
  while(act_line != line_start && (rd = getline(&readed, &len, fp)) != -1)
  {
    act_line++;
  }
  if (read == NULL)
  {
    free(readed);
    fclose(fp);
    send = make_env((unsigned char)12,NULL,seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    return;
  }

  //caso especifico na ultima linha + 1; é necessaria verificacao
  //e torna código mais complicado
  char ch;
  int end = 1;
  while(end)
  {
    clear_data(data);

    for (int i = 0; i < 15; ++i)
    {
      ch = fgetc(fp);
      if(ch == EOF)
      {
        end = 0;
        break;
      }
      if(ch == '\n')
      {
        data[i] = ch;
        act_line++;
        if(act_line > line_end)
        {
          end = 0;
          break;
        }
      }
      data[i] = ch;
    }
    send = make_env((unsigned char)12,data,seq_send_s,send);
    n = write(sckt, send, 19);
    seq_check(&seq_send_s);
    init_timer();
    //loop de espera de resposta do cliente
    while(1)
    {
      n = read(sckt,recv,19);
      if (read_env(recv,tam,NULL,&seq,&type) == -1)
      {
        seq_check(&seq_recv_s);
        return;
      }
      else if(type_check("NACK",type,seq_recv_s,seq))
      {
        seq_check(&seq_recv_s);
        n = write(sckt, send, 19);
        reset_timer();
      }
      else if(type_check("ACK",type,seq_recv_s,seq))
      {
        seq_check(&seq_recv_s);
        stop_timer();
        break;
      }
      if(time_out) {write(sckt, send, 19); time_out = 0;}
    }
  }
  //enviar final de dados
  send_final_data(&seq_send_s,&seq_recv_s,sckt,recv,send);
  fclose(fp);
}

//////////////////////////////////////////////////////////////////////////////////

void main()
{
  int n,type,seq,tam;
  unsigned char *data = malloc(15);
  int sckt = ConexaoRawSocket("lo");
  unsigned char *send = malloc(19);
  unsigned char *recv = malloc(19);
  init_time_handler();
  while(1)
  {
    stop_timer();
    n = read(sckt,recv,19);
    if (read_env(recv,&tam,data,&seq,&type) == -1)
    {
      send_nack(&seq_send_s,sckt);
      seq_check(&seq_send_s);
    }
    else if(type_check("ls",type,seq_recv_s,seq)) ls_s(sckt,data, &tam, send, recv);
    else if(type_check("cd",type,seq_recv_s,seq)) cd_s(sckt,data, &tam, send, recv);
    else if(type_check("ver",type,seq_recv_s,seq)) ver_s(sckt,data, &tam, send, recv);
    else if(type_check("edit",type,seq_recv_s,seq)) edit_s(sckt,data, &tam, send, recv);
    else if(type_check("linha",type,seq_recv_s,seq)) linha_s(sckt,data, &tam, send, recv);
    else if(type_check("linhas",type,seq_recv_s,seq)) linhas_s(sckt,data, &tam, send, recv);
    //limpa buffer de dados
    clear_data(data);
  }
} 
