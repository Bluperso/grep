#include "grep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int args, char** argv) {
  int mask = 0, flag = 0;
  arguments arg_arr[args];
  int cntptrn = 0, cntfl = 0;
  int regex_index = 0, index = 0;
  FILE* file = NULL;
  if (args > 1) {
    for (int i = 1; i < args && !flag; i++) {
      if (argv[i][0] == '-') flag = create_mask(argv[i], &mask);
    }
    fill_arg(args, argv, &mask, arg_arr, &cntptrn, &cntfl);
  } else if (args == 1) {
    fprintf(
        stderr,
        "Usage: grep [OPTION]... PATTERNS [FILE]...\nTry \'grep --help\' for "
        "more information.\n");
  }
  if (!flag) {
    regex_t regex[cntptrn];
    int compare_mode = mask & (1 << FLAG_i);
    while (regex_index < cntptrn || index < args) {
      if (arg_arr[index].type == ARG_PTTRN) {
        regcomp(&regex[regex_index], argv[index], compare_mode);
        regex_index++;
      }
      index++;
    }
    regex_index = 0;
    for (int i = 1; i < args; i++) {
      if (arg_arr[i].type == ARG_FILE)
        file = fopen(arg_arr[i].record, "r");
      else if ((arg_arr[i].type == ARG_STDIN) ||
               (cntfl == 0 && (i == (args - 1))))
        file = stdin;
      if (file != NULL) {
        main_process(arg_arr[i], file, regex, cntptrn, mask);
      } else if (file == NULL && (arg_arr[i].type == ARG_FILE)) {
        fprintf(stderr, "grep: %s: No such file or directory\n",
                arg_arr[i].record);
      }
      if ((arg_arr[i].type == ARG_FILE) && (file != NULL)) fclose(file);
      file = NULL;
    }
    for (int i = 0; i < cntptrn; i++) regfree(&regex[i]);
  }
  return 0;
}

int create_mask(char* argv, int* mask) {
  int flag = 0;
  if (argv[1] == '-') {
    if (!strcmp(argv, "--ignore-case"))
      (*mask) |= (1 << FLAG_i);
    else if (!strcmp(argv, "--invert-match"))
      (*mask) |= (1 << FLAG_v);
    else if (!strcmp(argv, "--count"))
      (*mask) |= (1 << FLAG_c);
    else if (!strcmp(argv, "--files-with-matches"))
      (*mask) |= (1 << FLAG_l);
    else if (!strcmp(argv, "--line-number"))
      (*mask) |= (1 << FLAG_n);
    else {
      fprintf(stderr, "grep: unrecognized option '%s'\n", argv);
      flag = 1;
    }
  } else {
    for (int j = 1; argv[j] != '\0' && !flag; j++) {
      switch (argv[j]) {
        case 'e':
          (*mask) |= (1 << FLAG_e);
          break;
        case 'i':
          (*mask) |= (1 << FLAG_i);
          break;
        case 'v':
          (*mask) |= (1 << FLAG_v);
          break;
        case 'c':
          (*mask) |= (1 << FLAG_c);
          break;
        case 'l':
          (*mask) |= (1 << FLAG_l);
          break;
        case 'n':
          (*mask) |= (1 << FLAG_n);
          break;
        default:
          fprintf(stderr, "grep: invalid option -- '%c'\n", argv[j]);
          flag = 1;
          break;
      }
    }
  }
  return flag;
}

void fill_arg(int args, char** argv, int* mask, arguments* arg_arr,
              int* cntptrn, int* cntfl) {
  int expect_pattern = 0;
  int pattern_found = 0;
  arg_arr[0].type = ARG_FLAG;
  arg_arr[0].record = argv[0];
  for (int i = 1; i < args; i++) {
    arg_arr[i].record = argv[i];
    if (expect_pattern) {
      arg_arr[i].type = ARG_PTTRN;
      if (strlen(argv[i]) == 0) arg_arr[i].record = argv[i];
      (*cntptrn)++;
      expect_pattern = 0;
      pattern_found = 1;
    } else if (strcmp(argv[i], "-") == 0) {
      arg_arr[i].type = ARG_STDIN;
      (*cntfl)++;
    } else if (argv[i][0] == '-') {
      arg_arr[i].type = ARG_FLAG;
      int flag_exit = 1;
      for (int j = 1; (argv[i][j] != '\0') && flag_exit; j++) {
        if (argv[i][j] == 'e') {
          expect_pattern = 1;
          flag_exit = 0;
        }
      }
    } else {
      if (!pattern_found && !((*mask) & (1 << FLAG_e))) {
        arg_arr[i].type = ARG_PTTRN;
        if (strlen(argv[i]) == 0) arg_arr[i].record = argv[i];
        (*cntptrn)++;
        pattern_found = 1;
      } else {
        arg_arr[i].type = ARG_FILE;
        (*cntfl)++;
      }
    }
  }
  if ((*cntfl) > 1) (*mask) |= (1 << FLAG_prntmode);
}

void main_process(arguments arg, FILE* file, regex_t* regex, int cntptrn,
                  unsigned char mask) {
  int count_match = 0, count_all = 0, count_match_line = 0, match_flag = 0;
  char* str = NULL;
  size_t len = 0;
  ssize_t read;
  regmatch_t match;
  while ((read = getline(&str, &len, file)) != -1) {
    regmatch_t* offsets = malloc(sizeof(regmatch_t));
    count_all++;
    int flag_exit = 1;
    int i = 0;
    int regex_index = 0;
    int cur_pos = 0;
    match_flag = 0;
    while (regex_index < cntptrn) {
      while (regexec(&regex[regex_index], str + cur_pos, 1, &match, 0) == 0 &&
             flag_exit) {
        count_match++;
        match_flag = 1;
        offsets[i] = match;
        if (match.rm_so == match.rm_eo)
          cur_pos++;
        else
          cur_pos += match.rm_eo;
        i++;
        offsets = realloc(offsets, (i + 1) * sizeof(regmatch_t));
        if (cur_pos >= (int)strlen(str)) flag_exit = 0;
      }
      regex_index++;
    }
    if (match_flag == 1) ++count_match_line;
    if (!((mask & (1 << FLAG_c)) || (mask & (1 << FLAG_l)))) {
      if ((count_match != 0) && !(mask & (1 << FLAG_v))) {
        print_line(str, arg, count_match_line, count_all, mask, STR_MODE);
        if (str[read - 1] != '\n') printf("\n");
      } else if ((count_match == 0) && (mask & (1 << FLAG_v))) {
        print_line(str, arg, count_match_line, count_all, mask, STR_MODE);
        if (str[read - 1] != '\n') printf("\n");
      }
    }
    count_match = 0;
    free(offsets);
  }
  if (((mask & (1 << FLAG_c)) || (mask & (1 << FLAG_l)))) {
    print_line(str, arg, count_match_line, count_all, mask, INF_MODE);
  }
  free(str);
}

void print_line(char* str, arguments arg, int count_match_line, int count_all,
                unsigned char mask, MODE mode) {
  if (mode == STR_MODE) {
    if ((mask & (1 << FLAG_prntmode)) && !(mask & (1 << FLAG_n))) {
      printf("%s:%s", arg.record, str);
    } else if ((mask & (1 << FLAG_prntmode)) && (mask & (1 << FLAG_n))) {
      printf("%s:%d:%s", arg.record, count_all, str);
    } else if (!(mask & (1 << FLAG_prntmode)) && !(mask & (1 << FLAG_n))) {
      printf("%s", str);
    } else if (!(mask & (1 << FLAG_prntmode)) && (mask & (1 << FLAG_n))) {
      printf("%d:%s", count_all, str);
    }
  } else if (mode == INF_MODE) {
    if (mask & (1 << FLAG_l) &&
        (((count_match_line != 0) && !(mask & (1 << FLAG_v))) ||
         ((count_all - count_match_line != 0) && (mask & (1 << FLAG_v))))) {
      printf("%s\n", arg.record);
    } else if ((mask & (1 << FLAG_c)) && !(mask & (1 << FLAG_v)) &&
               !(mask & (1 << FLAG_l))) {
      if (mask & (1 << FLAG_prntmode)) {
        printf("%s:%d\n", arg.record, count_match_line);
      } else if (!(mask & (1 << FLAG_prntmode))) {
        printf("%d\n", count_match_line);
      }
    } else if ((mask & (1 << FLAG_c)) && (mask & (1 << FLAG_v)) &&
               !(mask & (1 << FLAG_l))) {
      if (mask & (1 << FLAG_prntmode)) {
        printf("%s:%d\n", arg.record, count_all - count_match_line);
      } else if (!(mask & (1 << FLAG_prntmode))) {
        printf("%d\n", count_all - count_match_line);
      }
    }
  }
}
