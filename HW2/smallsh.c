/*****************************************
* Author: Kyle De Laurell
* Date: 11/17/2016
* Description: This is the source file for
* my command holder struct and the functions
* needed to allocate, populate, and deallocate
* this struct. It also contains the function
* that runs the command line program.
*******************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include "smallsh.h"


/***************************************
* Description: This function is the main
* function of my command line program.
* It acts like a command line with cd,
* status, and exit commands built
***************************************/

int main(){
  //intialize all the necessary variables
  struct CommandHolder* currComm = NULL;
  int backgroundArray[50];
  int bGASize = 0;
  pid_t forkedId = -5;
  pid_t testId = -5;
  int exitMethod;
  int bgExitMethod;
  int cdRet = 0;
  int childPro = 0;
  char commandString[2048];
  char HOMEdir[1000];
  char cdDir[1000];
  int fd = 0;
  int fd2, fdDevNull, fd3, fd4;
  int index;

  //ensures variables are all zeroed out
  memset(HOMEdir, '\0', 1000);
  memset(cdDir, '\0', 1000);
  getcwd(HOMEdir, sizeof(HOMEdir));
  strcat(HOMEdir, "/");
  for(index = 0; index < 50; index++){
    backgroundArray[index] = 0;
  }
  index = 0;

  //makes it so the main program ignores the SIGINT command given
  //by the user
  signal(SIGINT, SIG_IGN);

  //do-while loop that exits when the exit command is given or an exec fails
  do{

    //makes sure the commandString is cleared of any previous data
    //and then gets user input and sends it to functions to allocate
    //and fill the Command holder struct
    memset(commandString, '\0', sizeof(commandString));
    printf(":");
    scanf ("%[^\n]%*c", commandString);
    currComm = allocateComm();
    generateComm(currComm, commandString);


    //if statement that looks to see what command was given by the user: comment, cd, status, exit
    //or another command and executes that command accordingly
    if(currComm->isComment == 1){
      //prints everything the user inputted to the screen as a comment
      printf("%s\n", currComm->command);
    }
    else if(strcmp("cd", currComm->command) == 0){
      //checks to see if user entered and argument with cd
      //if not then redirects to home or else directs to directory of user choice
      if(currComm->numArgs < 2){
        cdRet = chdir(getenv("HOME"));
        if(cdRet == -1){
          printf("No such directory\n");
        }
      }
      else{
        memset(cdDir, '\0', 1000);
        strcpy(cdDir, getenv("PATH"));
        strcat(cdDir, currComm->args[1]);
        cdRet = chdir(currComm->args[1]);
        if(cdRet == -1){
          printf("No such directory\n");
        }
      }
    }
    else if(strcmp("status", currComm->command) == 0){
      //checks to see the exit method of the previous foreground program run
      if(exitMethod == 2 || exitMethod == 3 || exitMethod == 6 || exitMethod == 15 || exitMethod == 9){
        printf("Terminated by signal %d\n", exitMethod);
      }
      else{
        printf("Exit Value %d\n", exitMethod);
      }
    }
    else if(strcmp("exit", currComm->command) == 0){
      //goes through all running background processes and sends them a kill signal
      if(bGASize > 0){
        for(index = 0; index <  bGASize; index++){
          if(backgroundArray[index] != 0){
            kill(backgroundArray[index], SIGKILL);
          }
        }
      }
    }
    //this else if checks to see if the user entered nothing and clears the buffer
    //to allow for next user input
    else if(currComm->command[0] == '\0'){
      while (getchar() != '\n');
    }
    else{
      //this splits the program into 2 processes the parent and the child processes
      //it redirects any input/output and then calls exec to run the function that
      //the user wanted to run with the user given arguments
      forkedId = fork();
      //this if statement is only to be run by the CHILD Process
      if(forkedId == 0){
        //checks to see if the user specified the program to run in the background
        //and redirects input and output to /dev/null
        //if a foreground process than it enables that process to be terminated by SIGINT
        if(currComm->isForeground == 0){
          fdDevNull = open("/dev/null", 0);
          fd3 = dup2(fdDevNull, 0);
          fcntl(fd3, F_SETFD, FD_CLOEXEC);
          fd4 = dup2(fdDevNull, 1);
          fcntl(fd4, F_SETFD, FD_CLOEXEC);
        }
        else{
          signal(SIGINT, SIG_DFL);
        }

        //sets the child Program as 1 so that if exec fails than it will
        //immediately end that process so as not to create a FORK BOMB
        childPro = 1;

        //if statement checks for input and output redirections and directs
        //input or output accordingly
        if(currComm->isInputRD == 1){
          fd = open(currComm->args[currComm->inputRDArgNum], O_RDONLY);
          fcntl(fd, F_SETFD, FD_CLOEXEC);
          if(fd == -1){
            printf("cannot open %s for input.\n", currComm->args[currComm->inputRDArgNum]);
            exit(1);
          }
          fd2 = dup2(fd, 0);
          if(fd2 == -1){
            perror("dup2");
            exit(1);
          }
        }
        if(currComm->isOutputRD == 1){
          fd = open(currComm->args[currComm->outputRDArgNum], O_WRONLY|O_CREAT|O_TRUNC, 0644);
          fcntl(fd, F_SETFD, FD_CLOEXEC);
          if(fd == -1){
            printf("cannot open %s for output.\n", currComm->args[currComm->outputRDArgNum]);
            exit(1);
          }
          fd2 = dup2(fd, 1);
          if(fd2 == -1){
            perror("dup2");
            exit(1);
          }
        }

        //creates and argument array to pass to the exec function since the
        //array needs to be char* and the char array in the struct is not a char*
        //ensures that the array is clear before populating
        char* argArr[currComm->numArgs];
        for(index = 0; index < currComm->numArgs; index++){
          argArr[index] = malloc(sizeof(char)*32);
          if (argArr[index] == 0){
            printf("malloc() failed!\n");
          }
          memset(argArr[index], '\0', 32);
          if(currComm->inputRDArgNum != index && currComm->outputRDArgNum != index){
            sprintf(argArr[index], currComm->args[index]);
          }
          else{
            argArr[index] = NULL;
          }
        }
        argArr[index] = NULL;
        int execH = execvp(argArr[0], argArr);
          printf("No such file or directory. Exit Value %d\n", execH);

      }
      else if(forkedId == -1){
        //lets user know if the fork failed for some reason
        printf("ERROR in FORKING\n");
      }
      //this else is only to be run by the PARENT process
      else{
        //checks to see if the process that was run was in the background
        //so that it can display to the user the processes id and add it to the
        //background process array to check for when it completes
        if(currComm->isForeground == 0){
          printf("background pid is %d\n", forkedId);
          pid_t childId = waitpid(forkedId, &exitMethod, WNOHANG);
          if(childId == 0){
            backgroundArray[bGASize] = forkedId;
            bGASize++;
          }
        }

        //this else if when it is a foreground process and ir checks to see if
        //SIGINT Terminated the process so it can display the message needed.
        //it also takes care of making the exit value 1 for an error I kept receiving
        else{
          waitpid( forkedId ,&exitMethod, 0);
          //checks to see if process was killed by user command
          if(exitMethod == 2 || exitMethod == 3 || exitMethod == 6 || exitMethod == 15 || exitMethod == 9){
            printf("Terminated by signal %d\n", exitMethod);
          }
          if(exitMethod > 255){
            exitMethod = 1;
          }
          //checks all backgroung processes to see if they exited so that the user can
          //be notified of the background process ending before being returned command
          //line access
          if(bGASize > 0){
            for(index = 0; index <  bGASize; index++){
              if(backgroundArray[index] != 0){
                testId = waitpid(backgroundArray[index], &bgExitMethod, WNOHANG);
                if(testId != 0){
                  //checks to see if process was killed by user command
                  if(bgExitMethod == 2 || bgExitMethod == 3 || bgExitMethod == 6 || bgExitMethod == 15 || bgExitMethod == 9){
                    printf("background pid %d is done! Terminated by signal %d\n", backgroundArray[index], bgExitMethod);
                  }
                  else{
                    printf("background pid %d is done! Exit value is %d\n", backgroundArray[index], bgExitMethod);
                  }
                  backgroundArray[index] = 0;
                }
              }
            }
          }
        }
      }
    }
    //deallocates struct to prepare for next command by user
    deallocateComm(currComm);
    //checks to see if user exited program or if a exec failed and
    //the child process can exit without causing a FORK BOMB
  }while(strcmp("exit", currComm->command) != 0 && childPro == 0);

  //ends program
  return 0;
}


/*********************************************
* Description: This function allocates the
* memory used by the command holder struct
*********************************************/
struct CommandHolder* allocateComm(){
   struct CommandHolder* newComm;
   newComm = malloc(sizeof(struct CommandHolder));
   return newComm;
}



/*********************************************
* Description: This function takes the user input,
* parses the string ,and determines all the
* necessary information needed to execute the
* command correctly.
*********************************************/
void generateComm(struct CommandHolder* commH, char* inputS){
  //intializes variables needed and zeroes out all data in the
  //struct before entering struct data
  int index = 0;
  int lastSpace = 0;
  int firstArg = 0;
  int argCharCount = 0;
  commH->isOutputRD = 0;
  commH->isInputRD = 0;
  commH->inputRDArgNum = -1;
  commH->outputRDArgNum = -1;
  commH->isForeground = 1;
  commH->isComment = 0;
  commH->numArgs = 0;
  memset(commH->command, '\0', sizeof(commH->command));
  for(index = 0; index < 256; index++){
    memset(commH->args[index], '\0', sizeof(commH->args[index]));
  }
  index = 0;

  //checks to see if the user specified a comment and if he did then it enters
  //the entire user input into the command varibale of the struct
  if(inputS[index] == '#'){
    commH->isComment = 1;
    strcpy(commH->command, inputS);
  }
  else{
    //this while loop parses the entire string character by character until it
    //reaches the end of the string at which point it terminates
    while(inputS[index] != '\0' ){
      //checks to see if this is the first argument and if it is it checks for a
      //space char in which it will change the first Arg value to 1 to signal that
      //it is no longer the first arg
      //if it is the first arg it copies the char into the command string
      if(firstArg == 0){
        if(inputS[index] == ' '){
          firstArg = 1;
        }
        else{
          commH->command[index] = inputS[index];
        }
      }
      //if statement that checks for special chars such as ' ', '&', '<', '>'
      //if any of the special characters it changes one of the struct variables
      //to ensure the command executes as user specified.
      //checks to see if a background or foreground process
      if(inputS[index] == '&'){
        commH->isForeground = 0;
      }
      //checks to see if argument ended
      else if(inputS[index] == ' '){
        lastSpace = 1;
      }
      //checks to see if a input redirection is required and stores where the
      //the file will be held in the args array
      else if(inputS[index] == '<'){
        commH->isInputRD = 1;
        commH->inputRDArgNum = commH->numArgs + 1;
      }
      //checks to see if a output redirection is required and stores where the
      //the file will be held in the args array
      else if(inputS[index] == '>'){
        commH->isOutputRD = 1;
        commH->outputRDArgNum = commH->numArgs + 1;
      }
      else{
        //checks to see if the last char was a space so that it can move on to
        //the next argument
        if(lastSpace == 1){
          lastSpace = 0;
          argCharCount = 0;
          commH->numArgs++;
        }
        commH->args[commH->numArgs][argCharCount] = inputS[index];
        argCharCount++;
      }
      index++;
    }
  }

  //checks to see if there are 0 args so that it can always show the right num of args
  if(commH->args[0][0] != '\0'){
    commH->numArgs++;
  }
}


/**********************************************
* Description: This function deallocates the
* memory used by the command holder struct
**********************************************/
void deallocateComm(struct CommandHolder* kComm){
  //frees command holder struct is not already empty
  if(kComm != 0){
    free(kComm);
  }
}
