#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int matrixSize;
int generateSize, logSize, modSize, addSize;

// Struct
typedef struct{
  int *matrix;
  int xloc;
  int yloc;
  int log;
  int mod;
} queue;

queue *que;
int queLoc;

int rowLoc;
int colLoc;

int que1Created;
int matrixCreated;

// Matrix
int *matrix2D;

pthread_mutex_t mutexque;
pthread_mutex_t mutexprint;
pthread_mutex_t mutexlog;

pthread_t *generateThread;
pthread_t *logThread;
pthread_t *modThread;
pthread_t *addThread;

void *generateOperation(void *arg){
  pthread_mutex_lock(&mutexque);
  if(que1Created == 0){
    que = (queue *)malloc(((matrixSize / 5) * (matrixSize / 5)) * sizeof(queue));
    queLoc = 0;
    que1Created = 1;
  }
  pthread_mutex_unlock(&mutexque);
  //printf("========================================%d starts working %d ==================================================================\n", (int)arg, queLoc);
  while(queLoc < ((matrixSize / 5) * (matrixSize / 5))){
    int x;
    int y;
    pthread_mutex_lock(&mutexque);
    /*
    if(queLoc >= ((matrixSize / 5) * (matrixSize / 5))){
        pthread_mutex_unlock(&mutexque);
        break;
    }
    */
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
    int *tempMatrix;
    tempMatrix = (int *)malloc(25 * sizeof(int));

    for(int i = 0; i < 25; i++){
      tempMatrix[i] = rand() % 100 + 1;
    }
    //sleep(1);
    queue temp;
    temp.matrix = tempMatrix;
    temp.xloc = x;
    temp.yloc = y;
    temp.log = 1;
    temp.mod = 1;

    pthread_mutex_lock(&mutexque);
    que[queLoc] = temp;

    queLoc += 1;
    pthread_mutex_unlock(&mutexque);

    pthread_mutex_lock(&mutexprint);
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
    pthread_mutex_unlock(&mutexprint);
  }

  pthread_exit((void*) 0);
}

void *logOperation(void *arg){
  if(matrixCreated == 0){
    pthread_mutex_lock(&mutexlog);
    matrix2D = (int *)malloc(((matrixSize * matrixSize) * sizeof(int)));
    matrixCreated = 1;
    pthread_mutex_unlock(&mutexlog);
  }

  int allwriten = 0;
  while(allwriten != ((matrixSize / 5) * (matrixSize / 5))){
    if(que1Created == 0){
      continue;
    }

    int loc = -1;
    allwriten = 0;
    pthread_mutex_lock(&mutexlog);
    for(int i = 0; i < ((matrixSize / 5) * (matrixSize / 5)); i++){
      // check if any submatrixes created by generation threads
      //if so log the submatrix in to real matrix
      if(que[i].log == 1){
        que[i].log = 2;
        loc = i;
        break;
      }
      // if all of submatrixes transfered to the real matrix
      else if(que[i].log == 2){
        allwriten++;
      }
    }

    if(loc == -1){
      pthread_mutex_unlock(&mutexlog);
      continue;
    }
    pthread_mutex_unlock(&mutexlog);

    int x = que[loc].xloc;
    int y = que[loc].yloc;

    for(int j = 0; j < 5; j++){
      for(int k = 0; k < 5; k++){
        matrix2D[(x + j) * matrixSize + (y + k)] = que[loc].matrix[(j * matrixSize) + k];
      }
    }

    pthread_mutex_lock(&mutexprint);
    printf("Log_%d\t\"Log_%d operate on [%d,%d] submatrix\n", (int)arg, (int)arg, (x/5), (y/5));
    pthread_mutex_unlock(&mutexprint);
  }

  pthread_exit((void *) 0);
}

int main(int argc, char *argv[]){
  //random number geration
  time_t t;
  srand((unsigned) time(&t));

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
  pthread_mutex_init(&mutexprint, NULL);
  pthread_mutex_init(&mutexlog, NULL);

  generateThread = (pthread_t *)malloc(generateSize * sizeof(pthread_t));
  logThread = (pthread_t *)malloc(logSize * sizeof(pthread_t));
  modThread = (pthread_t *)malloc(modSize * sizeof(pthread_t));
  addThread = (pthread_t *)malloc(addSize * sizeof(pthread_t));

  //matrix2D = (int *)malloc(((matrixSize * matrixSize) * sizeof(int)));

  //que = (queue *)malloc(((matrixSize / 5) * (matrixSize / 5)) * sizeof(queue));
  //queLoc = 0;
  matrixCreated = 0;
  que1Created = 0;

  rowLoc = -5;
  colLoc = 0;

  for(int k = 0; k < generateSize; k++){
    pthread_create(&generateThread[k], NULL, generateOperation, (void *)k);
  }
  for(int k = 0; k < logSize; k++){
    pthread_create(&logThread[k], NULL, logOperation, (void *)k);
  }
  for(int k = 0; k < generateSize; k++){
    pthread_join(generateThread[k], &status);
  }
  for(int k = 0; k < logSize; k++){
    pthread_join(logThread[k], &status);
  }

  //Operation

  //printf("%s\n", "Operation ends");
  pthread_mutex_destroy(&mutexque);
  pthread_mutex_destroy(&mutexprint);
  pthread_mutex_destroy(&mutexlog);
  pthread_exit(NULL);
}
