#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

double func(double x) {
  double y = x * x - 4 * x + 18;
  return y;
} // x^2 - 4x + 18

typedef struct calculation_result {

  double result;

  pthread_mutex_t thread_lock;

} calculation_result;

typedef struct thread_args {

  calculation_result *result_s;

  double a, b;
  double steps;

  double (*function)(double);

} thread_args;

void *calc_func(void *args) {
  thread_args args_s = *(thread_args *)args;
  calculation_result *result_s = args_s.result_s;

  double square = 0;

  double offset = (args_s.b - args_s.a) / args_s.steps;
  for (double i = args_s.a; i < args_s.b; i += offset) {
    double x = i + offset / 2;
    square += args_s.function(x);
  }

  square *= offset;
  // printf("Square: %lf\n", square);

  pthread_mutex_lock(&result_s->thread_lock);

  result_s->result += square;
  // printf("Result now: %lf\n", result_s->result);

  pthread_mutex_unlock(&result_s->thread_lock);

  return NULL;
}

void help_print() {
  printf("-h      Print help and exit\n");
  printf("-a      start point of calculation\n");
  printf("-b      end point of calculation\n");
  printf("-p      number of calculation's threads\n");
}

int main(int argc, char **argv) {

  char *opts = "ha:b:p:v";
  int opt;

  int thread_count = 2;
  double a = 0;
  double b = 10;

  while ((opt = getopt(argc, argv, opts)) != -1) {

    switch (opt) {
    case 'h': {
      help_print();

      return 0;
    }

    case 'p': {
      thread_count = atoi(optarg);
      if (thread_count < 1) {
        printf("Warning! You've entered non-positive count of threads\n "
               "Nothing will happen.\n");
        return 0;
      }

      break;
    }

    case 'a': {
      a = atof(optarg);

      break;
    }

    case 'b': {
      b = atof(optarg);

      break;
    }
    }
  }

  pthread_t p_arr[thread_count];
  thread_args args_arr[thread_count];
  double offset = (b - a) / thread_count;

  calculation_result calc_res;
  calc_res.result = 0;
  if (pthread_mutex_init(&(calc_res.thread_lock), NULL) != 0) {
    printf("Error! Couldn't initialize mutex!\n");

    exit(-1);
  }

  for (int i = 0; i < thread_count; i++) {
    thread_args args;
    args.a = a + i * offset;
    args.b = args.a + offset;
    args.steps = 1000000;
    args.function = func;
    args.result_s = &calc_res;

    args_arr[i] = args;

    pthread_t thread;
    if (pthread_create(&thread, NULL, calc_func, &args_arr[i]) != 0) {
      printf("Error! Couldn't initialize thread!\n");

      exit(-1);
    }

    p_arr[i] = thread;
  }

  printf("a: %lf,b: %lf\nThreads count: %d\n", a, b, thread_count);

  for (int i = 0; i < thread_count; i++) {
    pthread_join(p_arr[i], NULL);
  }

  printf("Square is: %lf\n", calc_res.result);

  return EXIT_SUCCESS;
}