#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

int numberofArgument = 0;

struct background_process
{
  pid_t p_id;
  struct background_process *next;
};

int isFileExists(const char *filename)
{
  struct stat buffer;
  int exist = stat(filename, &buffer);
  if (exist == 0)
    return 1;
  else // -1
    return 0;
}

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */
void setup(char inputBuffer[], char *args[], int *background, int *red_type)
{
  int length, /* # of characters in the command line */
      i,      /* loop index for accessing inputBuffer array */
      start,  /* index where beginning of next command parameter is */
      ct;     /* index of where to place the next parameter into args[] */

  ct = 0;

  /* read what the user enters on the command line */
  length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

  /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

  start = -1;
  if (length == 0)
    exit(0); /* ^d was entered, end of user command stream */

  /* the signal interrupted the read system call */
  /* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
  if ((length < 0) && (errno != EINTR))
  {
    perror("error reading the command");
    exit(-1); /* terminate with error code of -1 */
  }

  //printf(">>%s<<",inputBuffer);
  for (i = 0; i < length; i++)
  { /* examine every character in the inputBuffer */

    switch (inputBuffer[i])
    {
    case ' ':
    case '>':
      if (inputBuffer[i] != ' ' && *red_type != 4)
        if(*red_type == 3)
          *red_type = 5;
        else
          *red_type += 1;
      if (start != -1)
      {
        args[ct] = &inputBuffer[start]; /* set up pointer */
        ct++;
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;

    case '<':
      *red_type = 3;
      if (start != -1)
      {
        args[ct] = &inputBuffer[start]; /* set up pointer */
        ct++;
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;
    case '2':
      if (inputBuffer[i + 1] == '>')
      {
        *red_type = 4;
        if (start != -1)
        {
          args[ct] = &inputBuffer[start]; /* set up pointer */
          ct++;
        }
        inputBuffer[i] = '\0'; /* add a null char; make a C string */
        start = -1;
      }
      break;
    case '\t': /* argument separators */
      if (start != -1)
      {
        args[ct] = &inputBuffer[start]; /* set up pointer */
        ct++;
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;

    case '\n': /* should be the final char examined */
      if (start != -1)
      {
        args[ct] = &inputBuffer[start];
        ct++;
      }
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
      break;

    default: /* some other character */
      if (start == -1)
        start = i;
      if (inputBuffer[i] == '&')
      {
        *background = 1;
        inputBuffer[i - 1] = '\0';
      }
    }              /* end of switch */
  }                /* end of for */
  args[ct] = NULL; /* just in case the input line was > 80 */
  numberofArgument = ct;
  //for (i = 0; i <= ct; i++)
  //printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

int main(void)
{
  char inputBuffer[MAX_LINE];   /*buffer to hold command entered */
  int background;               /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE / 2 + 1]; /*command line arguments */
  int red_type = 0;             /* redirection type '>' = 1 , '>>' = 2, '<' = 3, '2>' = 4 default = 0,|| the fifth case will be done*/
  int flag = 0;
  int fd[2];
  pid_t pid = 0;
  int execute;

  //link list's head that contains the background processes
  struct background_process *head = NULL;

  while (1)
  {
    red_type = 0;
    execute = 1;
    background = 0;
    printf("myshell: ");
    fflush(stdout);
    /*setup() calls exit() when Control-D is entered */
    setup(inputBuffer, args, &background, &red_type);

    if (red_type == 1)
    {
      fd[0] = open(args[numberofArgument - 1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
      args[numberofArgument-1] = NULL;
      flag = 1;
    }
    else if (red_type == 2 || red_type == 4)
    {
      fd[0] = open(args[numberofArgument - 1], O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);
      args[numberofArgument-1] = NULL;
      if(red_type == 4)
        flag = 4;
      else
        flag = 2;
    }
    else if (red_type == 3)
    {
      fd[1] = open(args[numberofArgument - 1], O_RDONLY, S_IRUSR | S_IWUSR);
      args[numberofArgument-1] = NULL;
      
      flag = 3;
    }else if (red_type == 5)
    {
      fd[0] = open(args[numberofArgument-1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
      fd[1] = open(args[numberofArgument-2], O_RDONLY, S_IRUSR | S_IWUSR);
      args[numberofArgument-1] = NULL;
      args[numberofArgument-2] = NULL;
      flag = 5;
    }else{
      flag = 0;
      red_type = 0;
    }

    if (fd < 0)
    {
      fprintf(stderr,"Open failed!\n");
    }

    //exit
    if (strcmp(inputBuffer, "exit") == 0)
    {
      execute = 0;
      int backgroundRunner = 0;

      struct background_process *current = head;
      while (current != NULL)
      {
        while (waitpid(-1, 0, WNOHANG) > 0)
          ;

        if (0 == kill(current->p_id, 0))
        {
          backgroundRunner = 1;
          fprintf(stderr, "There are still processes that run in the background\n");
          break;
        }
        current = current->next;
      }

      if (!backgroundRunner)
        exit(0);
    }
    //clr
    if (strcmp(inputBuffer, "clr") == 0){
      execute = 0;
      system("@cls||clear");
    }
    //fg
    if (strcmp(inputBuffer, "fg") == 0){
      execute = 0;

      int backgroundRunner = 0;

      struct background_process *current = head;
      while (current != NULL)
      {
        while (waitpid(-1, 0, WNOHANG) > 0)
          ;

        if (0 == kill(current->p_id, 0))
        {
          backgroundRunner = 1;
          fprintf(stderr, "There are still processes that run in the background\n");
          break;
        }
        current = current->next;
      }

      if(head == NULL || !backgroundRunner){
        fprintf(stderr, "%s\n", "There is no background process...");
      }
      else{
        //if there is background process
        struct background_process *current = head;
        while(current != NULL){
          if(kill(current->p_id, 0) == 0){
            fprintf(stderr, "%d %s\n", current->p_id, "running in the foreground now...");
            //moving each background process to foreground
            signal(SIGTTOU, SIG_IGN);
            tcsetpgrp(current->p_id, current->p_id);
            signal(SIGTTOU, SIG_DFL);
            waitpid(current->p_id, NULL, WUNTRACED);

            current = current->next;
          }
        }
      }
    }

    if (execute)
    {
      pid = fork();

      if (pid == -1)
        perror("fork error");
      //child process
      else if (pid == 0)
      {
        char *path = (char *)malloc(1000);
        path = strcpy(path, getenv("PATH"));
        char *token = strsep(&path, ":");
        while (token != NULL)
        {
          char buffer[MAX_LINE] = "";
          strcat(buffer, token);
          strcat(buffer, "/");
          strcat(buffer, args[0]);
          if (isFileExists(buffer))
          {
            args[-1] = NULL;
            // this will be changed to execl
            if (flag == 1 || flag == 2)
            {
              dup2(fd[0], STDOUT_FILENO);
              close(fd[0]);
              execv(buffer, args);
              flag = 0;
              exit(0);
              
            }
            if (flag == 3)
            {
              dup2(fd[1], STDIN_FILENO);
              close(fd[1]);
              execv(buffer, args);
              exit(0);
 
            }
            if (flag == 4)
            {
              dup2(fd[0], STDERR_FILENO);
              close(fd[0]);
              execv(buffer, args);
              exit(0);
      
            }
            if (flag == 5)
            {
              dup2(fd[1], STDIN_FILENO);
              dup2(fd[0], STDOUT_FILENO);
              close(fd[0]);
              close(fd[1]);
              execv(buffer, args);
              exit(0);
     
            }
            else
            {
             
              execv(buffer, args);
              exit(0);
            }
          }
          token = strsep(&path, ":");
        }
        exit(0);
      }
      //parent process
      else
      {
        //not background
        if (!background)
        {
          waitpid(pid, NULL, 0);
        }
        else
        {
          struct background_process *temp = NULL;
          struct background_process *temp2 = NULL;

          if (head == NULL)
          {
            temp = (struct background_process *)malloc(sizeof(struct background_process));

            temp->p_id = pid;
            temp->next = NULL;
            head = temp;
            printf("%d running in background\n", pid);
          }
          else if (head->next == NULL)
          {
            temp = (struct background_process *)malloc(sizeof(struct background_process));

            temp->p_id = pid;
            temp->next = NULL;
            head->next = temp;
            printf("%d running in background\n", pid);
          }
          else
          {
            temp2 = (struct background_process *)malloc(sizeof(struct background_process));

            temp = head;
            while (temp->next != NULL)
            {
              temp = temp->next;
            }
            temp2->p_id = pid;
            temp2->next = NULL;

            temp->next = temp2;
            printf("%d running in background\n", pid);
          }
        }
      }
    }

    /* the steps are:
    (1) fork a child process using fork()
    (2) the child process will invoke execv()
		(3) if background == 0, the parent will wait,
        otherwise it will invoke the setup() function again. */
  }
}
