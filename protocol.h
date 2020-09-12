#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>

void clear_data(unsigned char * data);

uint16_t unpack(uint8_t *src, int tam);

int type_check(char *check,int type_r,int seq_recv, int seq);

void error(unsigned char *data);

void seq_check(int *seq);

void send_ack(int *seq, int sckt);  

void send_nack(int *seq, int sckt);

unsigned char * make_env(unsigned char type, unsigned char *data, unsigned char seq, unsigned char *env);

int read_env(unsigned char *msg, int *tam, unsigned char *data, int *seq, int *type);