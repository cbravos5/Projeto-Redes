#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>


void init_timer();

void reset_timer();

void stop_timer();

void time_out_handler(int signal);

void init_time_handler();

void print_data(unsigned char *data, int tam, short int param, int *n_line);

void print_data_default(int *seq_send,int *seq_recv,int sckt, unsigned char *recv,unsigned char *send,
						 unsigned char *data, short int param1, short int param2, int *n_line, char* data_type);

void send_final_data(int *seq_send,int *seq_recv,int sckt, unsigned char *recv, unsigned char *send);

void clear_data(unsigned char * data);

uint16_t unpack(uint8_t *src, int tam);

int type_check(char *check,int type_r,int seq_recv, int seq);

void error(unsigned char *data);

void seq_check(int *seq);

void send_ack(int *seq, int sckt);  

void send_nack(int *seq, int sckt);

unsigned char * make_env(unsigned char type, unsigned char *data, unsigned char seq, unsigned char *env);

int read_env(unsigned char *msg, int *tam, unsigned char *data, int *seq, int *type);