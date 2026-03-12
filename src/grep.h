#ifndef GREP
#define GREP

#include <regex.h>
#include <stdio.h>

typedef enum { STR_MODE, INF_MODE } MODE;

typedef enum {
  FLAG_e,
  FLAG_i,
  FLAG_v,
  FLAG_c,
  FLAG_l,
  FLAG_n,
  FLAG_prntmode
} FLAGS;

typedef enum { ARG_PTTRN, ARG_FILE, ARG_FLAG, ARG_STDIN } arg_type;

typedef struct {
  arg_type type;
  char* record;
} arguments;

int create_mask(char* argv, int* mask);
void fill_arg(int args, char** argv, int* mask, arguments* arg_arr,
              int* cntptrn, int* cntfl);
void main_process(arguments arg, FILE* file, regex_t* regex, int cntptrn,
                  unsigned char mask);
void print_line(char* str, arguments arg, int count_match_line, int count_all,
                unsigned char mask, MODE mode);

#endif
