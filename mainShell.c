#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_BACKGROUND 100 //change this to link list
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

int isFileExists(const char* filename){
  struct stat buffer;
  int exist = stat(filename,&buffer);
  if(exist == 0)
    return 1;
  else // -1
    return 0;
}

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                    args[ct] = &inputBuffer[start];
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

int main(void)
{
  char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
  int background; /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1]; /*command line arguments */

  pid_t pid=0;

  //array that contains the background processes
  int *backgroundArray=(int*)calloc(MAX_BACKGROUND, sizeof(int));

  while (1){
    background = 0;
    printf("myshell: ");
    fflush(stdout);
    /*setup() calls exit() when Control-D is entered */
    setup(inputBuffer, args, &background);

    pid = fork();

    if(pid == -1)
      perror("fork error");
    //child process
    else if(pid == 0){
      char *path =(char *) malloc(1000);
      path = strcpy(path, getenv("PATH"));
      char *token = strsep(&path, ":");

      while(token != NULL){
        char buffer[MAX_LINE] = "";
        strcat(buffer, token);
        strcat(buffer, "/");
        strcat(buffer, args[0]);

        if(isFileExists(buffer)){
          args[-1] = NULL;
          // this will be changed to execl
          execv(buffer, args);
          exit(0);
        }
        token = strsep(&path, ":");
      }
      exit(0);
    }
    //parent process
    else{
      //not background
      if(!background){
        waitpid(pid, NULL, 0);
      }
      else{
        int i;
        for(i = 0; i < MAX_BACKGROUND; i++){
          if(backgroundArray[i] == 0){
            break;
          }
        }
        backgroundArray[i] = pid;
        printf("[%d] %d running in background\n", i, pid);
      }
    }

    /* the steps are:
    (1) fork a child process using fork()
    (2) the child process will invoke execv()
		(3) if background == 0, the parent will wait,
        otherwise it will invoke the setup() function again. */
            }
}
