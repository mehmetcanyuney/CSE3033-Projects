#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int matrixSize;
int generateSize, logSize, modSize, addSize;

// Struct
typedef struct{
  int matrix;
  int xloc;
  int yloc;
} queue;

queue *que;
int queLoc;

int rowLoc;
int colLoc;

// Matrix
int *matrix2D;

pthread_mutex_t mutexque;
pthread_t *generateThread;
pthread_t *logThread;
pthread_t *modThread;
pthread_t *addThread;

void *generateOperation(void *arg){
  int x;
  int y;
  pthread_mutex_lock(&mutexque);
  rowLoc += 5;
  if(rowLoc == matrixSize){
    rowLoc = 0;
    colLoc += 5;
  }

  x = rowLoc;
  y = colLoc;
  pthread_mutex_unlock(&mutexque);

  if(colLoc >= matrixSize){
    pthread_exit((void*) 0);
  }
  int tempMatrix[25];

  for(int i = 0; i < 25; i++){
    tempMatrix[i] = rand() % 100 + 1;
  }

  queue temp;
  temp.matrix = *tempMatrix;
  temp.xloc = x;
  temp.yloc = y;

  pthread_mutex_lock(&mutexque);
  que[queLoc] = temp;

  queLoc += 1;
  pthread_mutex_unlock(&mutexque);

  // Printing the result
  printf("Generator_%d\t\"Generator_%d generated following matrix:[", (int)arg, (int)arg);
  for(int i = 0; i < 25; i++){
    if(i != 0 && (i%5 == 0)){
      printf("\n\t\t\t\t\t\t\t");
    }
    if(i == 24)
      printf("%d", tempMatrix[i]);
    else
      printf("%d,", tempMatrix[i]);
  }
  printf("]\n\t\tThis matrix is [%d,%d] submatrix\n\n", (x/5), (y/5));

  pthread_exit((void*) 0);
}

int main(int argc, char *argv[]){
  void *status;

  if(argc != 8){
    fprintf(stderr, "%s\n", "USAGE: ./main.o -d <matrixSize> -n <ThreadsSize(4 of them)> \n or \nUSAGE: ./main.o -n <ThreadsSize(4 of them)> -d <matrixSize>" );
    return 0;
  }

  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      i = i + 1;
      matrixSize = atoi(argv[i]);
    }
    else if(strcmp(argv[i], "-n") == 0){
      i = i + 1;
      generateSize = atoi(argv[i]);
      i = i + 1;
      logSize = atoi(argv[i]);
      i = i + 1;
      modSize = atoi(argv[i]);
      i = i + 1;
      addSize = atoi(argv[i]);
    }
  }

  pthread_mutex_init(&mutexque, NULL);

  generateThread = (pthread_t *)malloc(generateSize * sizeof(pthread_t));
  logThread = (pthread_t *)malloc(logSize * sizeof(pthread_t));
  modThread = (pthread_t *)malloc(modSize * sizeof(pthread_t));
  addThread = (pthread_t *)malloc(addSize * sizeof(pthread_t));

  matrix2D = (int *)malloc(((matrixSize * matrixSize) * sizeof(int)));

  que = (queue *)malloc(((matrixSize / 5) * (matrixSize / 5)) * sizeof(queue));
  queLoc = 0;

  rowLoc = -5;
  colLoc = 0;

  for(int k = 0; k < generateSize; k++){
    pthread_create(&generateThread[k], NULL, generateOperation, (void *)k);
  }
  for(int k = 0; k < generateSize; k++){
    pthread_join(generateThread[k], &status);
  }

  //printf("%s\n", "Operation ends");
  pthread_mutex_destroy(&mutexque);
  pthread_exit(NULL);
}
