#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct rwl_data {

  int *arr;
  int arr_size;

  pthread_rwlock_t rwlock;
} rwl_data;

typedef struct thread_args {
  int id;

  rwl_data *data;

} thread_args;

void help_print() {
  printf("-h      Print help and exit\n\n");

  printf("-n      numbersd of element in array\n");
  printf("-w      numbers of write threads\n");
  printf("-r      number of read threads (must be bigger then write!) \n\n");
}

void arr_init(int *arr, int arr_size) {
  for (int i = 0; i < arr_size; i++) {
    arr[i] = 0;
  }
}

void arr_print(int *arr, int arr_size) {
  for (int i = 0; i < arr_size; i++) {
    if (i % 10 == 0)
      printf("\n");
    printf("[%d]: %d ", i, arr[i]);
  }
  printf("\n");
}

void *write_func(void *args) {
  thread_args arg = *(thread_args *)args;

  rwl_data *data = arg.data;
  int *arr = data->arr;
  int arr_size = data->arr_size;

  sleep(rand() % 5 + 1);

  while (1) {
    pthread_rwlock_wrlock(&data->rwlock);
    int pos = rand() % arr_size;
    int val = rand() % 21 - 10;
    arr[pos] = val;

    printf(" < < Write thread %d wrote number %d to [%d] in array!\n", arg.id,
           val, pos);

    pthread_rwlock_unlock(&data->rwlock);
    sleep(rand() % 15 + 7);
  }

  return NULL;
}

void *read_func(void *args) {
  thread_args arg = *(thread_args *)args;

  rwl_data *data = arg.data;
  int *arr = data->arr;
  int arr_size = data->arr_size;

  while (1) {
    if (pthread_rwlock_rdlock(&data->rwlock) != 0) {
      sleep(1);
      continue;
    }
    int pos = rand() % arr_size;
    printf(" > > Read thread %d readed %d from [%d] in array\n", arg.id,
           arr[pos], pos);
    pthread_rwlock_unlock(&data->rwlock);
    sleep(rand() % 12 + 5);
  }
}

void *print_func(void *args) {
  thread_args arg = *(thread_args *)args;
  rwl_data *data = arg.data;
  int *arr = data->arr;
  int arr_size = data->arr_size;

  while (1) {
    if (pthread_rwlock_rdlock(&data->rwlock) != 0) {
      continue;
    }
    printf("\n - - -\n");
    arr_print(arr, arr_size);
    printf(" - - -\n\n");

    pthread_rwlock_unlock(&data->rwlock);
    sleep(5);
  }
}

int main(int argc, char **argv) {
  char *opts = "hn:w:r:";
  int opt;

  int rt_count = 3, wt_count = 2;
  int size = 10;

  while ((opt = getopt(argc, argv, opts)) != -1) {
    switch (opt) {

    case 'h': {
      help_print();

      return EXIT_SUCCESS;
    }

    case 'n': {
      size = atoi(optarg);

      break;
    }

    case 'r': {
      rt_count = atoi(optarg);
    }

    case 'w': {
      wt_count = atoi(optarg);
    }
    }
  }

  if (wt_count > rt_count) {
    printf("Number of read threads must be bigger then write threads!\n");

    return EXIT_FAILURE;
  }

  printf("To exit programm, print q\n");
  sleep(1);

  int arr[size];
  arr_init(arr, size);

  rwl_data data;
  data.arr = arr;
  data.arr_size = size;
  pthread_rwlock_init(&data.rwlock, NULL);

  thread_args r_args[rt_count];
  pthread_t r_threads[rt_count];

  thread_args w_args[wt_count];
  pthread_t w_threads[wt_count];

  for (int i = 0; i < wt_count; i++) {
    thread_args args;
    args.id = i;
    args.data = &data;

    pthread_t thread;
    if (pthread_create(&thread, NULL, write_func, &args) != 0) {
      printf("Error! Couldn't initialize thread!\n");

      return EXIT_FAILURE;
    }

    w_threads[i] = thread;
    w_args[i] = args;
  }

  for (int i = 0; i < rt_count; i++) {
    thread_args args;
    args.id = i;
    args.data = &data;

    pthread_t thread;
    if (pthread_create(&thread, NULL, read_func, &args) != 0) {
      printf("Error! Couldn't initialize thread!\n");

      return EXIT_FAILURE;
    }

    r_threads[i] = thread;
    r_args[i] = args;
  }

  thread_args p_args;
  p_args.id = 0;
  p_args.data = &data;

  pthread_attr_t p_attr;
  pthread_attr_init(&p_attr);
  pthread_attr_setdetachstate(&p_attr, PTHREAD_CREATE_DETACHED);

  pthread_t p_thread;
  pthread_create(&p_thread, &p_attr, print_func, &p_args);

  char c = 'A';
  while (c != 'q') {
    c = getchar();
  }

  return EXIT_SUCCESS;
}