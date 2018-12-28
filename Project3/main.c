#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int matrixSize;
int generateSize, logSize, modSize, addSize;

// Struct for queue1
typedef struct{
  int *matrix;  //5x5 matrix
  int xloc;     //x_location on global matrix
  int yloc;     //y_location on global matrix
  int log;      //check if log operation done
  int mod;      //check if mod operation done
} queue;

// Struct for queue2
typedef struct{
  int *matrix;  //5x5 matrix
  int xloc;     //x_location on global matrix
  int yloc;     //y_location on global matrix
  int add;      //check if add operation done
} queue2;

//global queue1
queue *que;
int queLoc;
//global queue2
queue2 *que2;
int que2Size;

//global Location of matrix that is going to be generated
int rowLoc;
int colLoc;

//global sum variable for add operation
int sum;

//check for creating of ques on first thread also for matrix
int que1Created;
int que2Created;
int matrixCreated;

// Global Matrix that will be created via log thread
int *matrix2D;

//Mutex
pthread_mutex_t mutexque;
pthread_mutex_t mutexprint;
pthread_mutex_t mutexlog;
pthread_mutex_t mutexmod;
pthread_mutex_t mutexadd;

//Threads
pthread_t *generateThread;
pthread_t *logThread;
pthread_t *modThread;
pthread_t *addThread;

//Generate Thread
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
        printf("\n\t\t\t\t\t\t\t ");
      }
      if(i == 24)
        printf("%d", tempMatrix[i]);
      else
        printf("%d,", tempMatrix[i]);
    }
    printf("]\n\t\tThis matrix is [%d,%d] submatrix\"\n\n", (x/5), (y/5));
    pthread_mutex_unlock(&mutexprint);
  }

  pthread_exit((void*) 0);
}

//Log Thread
void *logOperation(void *arg){
  pthread_mutex_lock(&mutexlog);
  if(matrixCreated == 0){
    matrix2D = (int *)malloc(((matrixSize * matrixSize) * sizeof(int)));
    matrixCreated = 1;
  }
  pthread_mutex_unlock(&mutexlog);

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
        matrix2D[(x + j) * matrixSize + (y + k)] = que[loc].matrix[(j * 5) + k];
        //printf("%d,", matrix2D[(x + j) * matrixSize + (y + k)]);
      }
    }

    pthread_mutex_lock(&mutexprint);
    printf("Log_%d\t\"Log_%d operate on [%d,%d] submatrix\"\n\n", (int)arg, (int)arg, (x/5), (y/5));
    pthread_mutex_unlock(&mutexprint);
  }

  pthread_exit((void *) 0);
}

//Mod Thread
void *modOperation(void *arg){
  pthread_mutex_lock(&mutexmod);
  if(que2Created == 0){
    que2 = (queue2 *)malloc(((matrixSize / 5) * (matrixSize / 5)) * sizeof(queue2));
    que2Size = 0;
    que2Created = 1;
  }
  pthread_mutex_unlock(&mutexmod);

  int allModOperated = 0;
  while(allModOperated != ((matrixSize / 5) * (matrixSize / 5))){
    if(que1Created == 0){
      continue;
    }
    //same procedure as log operation
    int loc = -1;
    allModOperated = 0;
    pthread_mutex_lock(&mutexmod);
    for(int i = 0; i < ((matrixSize / 5) * (matrixSize / 5)); i++){
      if(que[i].mod == 1){
        que[i].mod = 2;
        loc = i;
        break;
      }
      else if(que[i].mod == 2){
        allModOperated++;
      }
    }

    if(loc == -1){
      pthread_mutex_unlock(&mutexmod);
      continue;
    }
    pthread_mutex_unlock(&mutexmod);

    int *tempMatrix;
    tempMatrix = (int *)malloc(25 * sizeof(int));

    int divider = que[loc].matrix[0];

    for(int i = 0; i < 25; i++){
      tempMatrix[i] = que[loc].matrix[i] % divider;
    }

    queue2 temp;
    temp.matrix = tempMatrix;
    temp.xloc = que[loc].xloc;
    temp.yloc = que[loc].yloc;
    temp.add = 1;

    pthread_mutex_lock(&mutexmod);
      que2[que2Size] = temp;

      que2Size++;
    pthread_mutex_unlock(&mutexmod);

    pthread_mutex_lock(&mutexprint);
    // Printing the result
    printf("Mod_%d\t\"Mod_%d generated following matrix:[", (int)arg, (int)arg);
    for(int i = 0; i < 25; i++){
      if(i != 0 && (i%5 == 0)){
        printf("\n\t\t\t\t\t   ");
      }
      if(i == 24)
        printf("%d", tempMatrix[i]);
      else
        printf("%d,", tempMatrix[i]);
    }
    printf("]\n\t\tThis matrix is [%d,%d] submatrix\"\n\n", (temp.xloc/5), (temp.yloc/5));
    pthread_mutex_unlock(&mutexprint);
  }

  pthread_exit((void *) 0);
}

//Add Thread
void *addOperation(void *arg){
  int allSummed = 0;
  while(allSummed != ((matrixSize / 5) * (matrixSize / 5))){
    if(que2Created == 0){
      continue;
    }
    int loc = -1;
    allSummed = 0;
    pthread_mutex_lock(&mutexadd);
    for(int i = 0; i < ((matrixSize / 5) * (matrixSize / 5)); i++){
      if(que2[i].add == 1){
        que2[i].add = 2;
        loc = i;
        break;
      }
      else if(que2[i].add == 2){
        allSummed++;
      }
    }

    if(loc == -1){
      pthread_mutex_unlock(&mutexadd);
      continue;
    }
    pthread_mutex_unlock(&mutexadd);

    int localsum = 0;
    for(int i = 0; i < 25; i++){
      localsum = localsum + que2[loc].matrix[i];
    }

    pthread_mutex_lock(&mutexadd);
    int oldSum = sum;
    sum = sum + localsum;
    int printedSum = sum;
    pthread_mutex_unlock(&mutexadd);

    pthread_mutex_lock(&mutexprint);
    printf("Add_%d\t\"Add_%d has localsum: %d by [%d,%d] submatrix, globalsum before/after update: %d/%d\"\n\n", (int)arg, (int)arg, localsum, (que2[loc].xloc/5), (que2[loc].yloc/5), oldSum, printedSum);
    pthread_mutex_unlock(&mutexprint);
  }

  pthread_exit((void *) 0);
}

int main(int argc, char *argv[]){
  //random number geration
  time_t t;
  srand((unsigned) time(&t));

  //check if requed number of arguments given
  if(argc != 8){
    fprintf(stderr, "%s\n", "USAGE: ./main.o -d <matrixSize> -n <ThreadsSize(4 of them)> \n or \nUSAGE: ./main.o -n <ThreadsSize(4 of them)> -d <matrixSize>" );
    return 0;
  }
  //parsing the given inputs
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

  //initilazing threads return values
  void *generateStatus[generateSize];
  void *logStatus[logSize];
  void *modStatus[modSize];
  void *addStatus[addSize];

  //initilazing mutex-es
  pthread_mutex_init(&mutexque, NULL);
  pthread_mutex_init(&mutexprint, NULL);
  pthread_mutex_init(&mutexlog, NULL);
  pthread_mutex_init(&mutexmod, NULL);
  pthread_mutex_init(&mutexadd, NULL);

  //creating thread arrays
  generateThread = (pthread_t *)malloc(generateSize * sizeof(pthread_t));
  logThread = (pthread_t *)malloc(logSize * sizeof(pthread_t));
  modThread = (pthread_t *)malloc(modSize * sizeof(pthread_t));
  addThread = (pthread_t *)malloc(addSize * sizeof(pthread_t));

  //matrix2D = (int *)malloc(((matrixSize * matrixSize) * sizeof(int)));

  //que = (queue *)malloc(((matrixSize / 5) * (matrixSize / 5)) * sizeof(queue));
  //queLoc = 0;
  //check variables initilazing
  matrixCreated = 0;
  que1Created = 0;
  que2Created = 0;
  sum = 0;

  rowLoc = -5;
  colLoc = 0;

  // Generate Threads created...
  for(int k = 0; k < generateSize; k++){
    pthread_create(&generateThread[k], NULL, generateOperation, (void *)k);
  }
  // Log Threads created...
  for(int k = 0; k < logSize; k++){
    pthread_create(&logThread[k], NULL, logOperation, (void *)k);
  }
  // Mod Threads created...
  for(int k = 0; k < modSize; k++){
    pthread_create(&modThread[k], NULL, modOperation, (void *)k);
  }
  // Add Threads created...
  for(int k = 0; k < addSize; k++){
    pthread_create(&addThread[k], NULL, addOperation, (void *)k);
  }
  // Waiting for Generate Threads to end...
  for(int k = 0; k < generateSize; k++){
    pthread_join(generateThread[k], &generateStatus[k]);
  }
  // Waiting for Log Threads to end...
  for(int k = 0; k < logSize; k++){
    pthread_join(logThread[k], &logStatus[k]);
  }
  // Waiting for Mod Threads to end...
  for(int k = 0; k < modSize; k++){
    pthread_join(modThread[k], &modStatus[k]);
  }
  // Waiting for Add Threads to end...
  for(int k = 0; k < addSize; k++){
    pthread_join(addThread[k], &addStatus[k]);
  }
  //Checking the result of generate threads
  for(int i = 0; i < generateSize; i++){
    if((int)generateStatus[i] != 0){
      fprintf(stderr, "%s\n", "There is something wrong with one of the Generation Thread");
      pthread_mutex_destroy(&mutexque);
      pthread_mutex_destroy(&mutexprint);
      pthread_mutex_destroy(&mutexlog);
      pthread_mutex_destroy(&mutexmod);
      pthread_mutex_destroy(&mutexadd);
      pthread_exit(NULL);
    }
  }
  //Checking the result of log threads
  for(int i = 0; i < logSize; i++){
    if((int)logStatus[i] != 0){
      fprintf(stderr, "%s\n", "There is something wrong with one of the Log Thread");
      pthread_mutex_destroy(&mutexque);
      pthread_mutex_destroy(&mutexprint);
      pthread_mutex_destroy(&mutexlog);
      pthread_mutex_destroy(&mutexmod);
      pthread_mutex_destroy(&mutexadd);
      pthread_exit(NULL);
    }
  }
  //Checking the result of mod threads
  for(int i = 0; i < modSize; i++){
    if((int)modStatus[i] != 0){
      fprintf(stderr, "%s\n", "There is something wrong with one of the Mod Thread");
      pthread_mutex_destroy(&mutexque);
      pthread_mutex_destroy(&mutexprint);
      pthread_mutex_destroy(&mutexlog);
      pthread_mutex_destroy(&mutexmod);
      pthread_mutex_destroy(&mutexadd);
      pthread_exit(NULL);
    }
  }
  //Checking the result of add threads
  for(int i = 0; i < addSize; i++){
    if((int)addStatus[i] != 0){
      fprintf(stderr, "%s\n", "There is something wrong with one of the Add Thread");
      pthread_mutex_destroy(&mutexque);
      pthread_mutex_destroy(&mutexprint);
      pthread_mutex_destroy(&mutexlog);
      pthread_mutex_destroy(&mutexmod);
      pthread_mutex_destroy(&mutexadd);
      pthread_exit(NULL);
    }
  }

  // Creating file and writing the created matrix (from Log Threads)
  FILE *fp;
  fp = fopen("output.txt", "w");
  fprintf(fp, "%s\n\t\t[", "The matrix is");
  for(int i = 0; i < matrixSize; i++){
    for(int j = 0; j < matrixSize; j++){
      if((i == (matrixSize - 1)) && (j == (matrixSize - 1))){
        fprintf(fp, "%d", matrix2D[i * matrixSize + j]);
      }
      else{
        fprintf(fp, "%d,", matrix2D[i * matrixSize + j]);
      }
    }
    if(i == (matrixSize - 1)){
      fprintf(fp, "]\n");
    }
    else{
      fprintf(fp, "\n\t\t ");
    }
  }
  fprintf(fp, "\nThe global sum is: %d.\n", sum);
  fclose(fp);

  //printf("%s\n", "Operation ends");
  pthread_mutex_destroy(&mutexque);
  pthread_mutex_destroy(&mutexprint);
  pthread_mutex_destroy(&mutexlog);
  pthread_mutex_destroy(&mutexmod);
  pthread_mutex_destroy(&mutexadd);
  pthread_exit(NULL);
}
