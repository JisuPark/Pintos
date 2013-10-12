#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/****Added function****/
void parse_filename(char argv[64][128],int* argc,char* file_name);
void construct_ESP(void** esp,char argv[64][128],int arg_tot_len,int argc);

#endif /* userprog/process.h */
