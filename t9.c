
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct thread_args {

  int *int_p;
  int count;

} thread_args;

void *func_plus(void *args) {
  thread_args targs = *(thread_args *)args;
  int *num = targs.int_p;

  for (int i = 0; i < targs.count; i++) {
    *num += 1;
  }

  return NULL;
}

void *func_minus(void *args) {
  thread_args targs = *(thread_args *)args;
  int *num = targs.int_p;

  for (int i = 0; i < targs.count; i++) {
    *num -= 1;
  }

  return NULL;
}

void help_print() {
  printf("-h      Print help and exit\n");
  printf("-p      number of threads pair to create ( 1 by fdefault )\n");
  printf("-o      number of operations in threads (10 by default)\n");
}

int main(int argc, char **argv) {

  char *opts = "hp:o:v";
  int opt;

  int thread_count = 2;
  int operations_count = 10;
  int num = 0;
  while ((opt = getopt(argc, argv, opts)) != -1) {

    switch (opt) {
    case 'h': {
      help_print();
      return 0;
    }

    case 'p': {
      thread_count = atoi(optarg) * 2;
      if (thread_count < 1) {
        printf("Warning! You've entered non-positive count of thread pairs\n "
               "Nothing will happen.\n");

        return 0;
      }

      break;
    }

    case 'o': {
      operations_count = atoi(optarg);
      if (operations_count < 1) {
        printf("Warning! You've entered non-positive count of operations\n "
               "Nothing will happen.\n");

        return 0;
      }

      break;
    }
    }
  }

  printf("Total count of threads: %d\n", thread_count);
  printf("Total count of operations: %d\n", operations_count);

  thread_args args;
  args.int_p = &num;
  args.count = operations_count;

  pthread_t p_arr[thread_count];
  for (int i = 0; i < thread_count; i += 2) {
    pthread_t thread_m, thread_p;
    pthread_create(&thread_m, NULL, func_minus, (void *)&args);
    pthread_create(&thread_p, NULL, func_plus, (void *)&args);
    p_arr[i] = thread_m;
    p_arr[i + 1] = thread_p;
  }

  for (int i = 0; i < thread_count; i++) {
    pthread_join(p_arr[i], NULL);
  }

  printf("Final result: %d\n", num);
}