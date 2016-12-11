/*****************************************
* Author: Kyle De Laurell
* Date: 11/17/2016
* Description: This is the header file for
* my command holder struct and the functions
* needed to allocate, populate, and deallocate
* this struct.
*******************************************/

#ifndef smallsh_h
#define smallsh_h
/*****************************************
* This structs holds the information
* needed to execute a command given by
* the user correctly.
******************************************/
struct CommandHolder{
  char command[32];
  int isForeground;
  int isComment;
  int isInputRD;
  int isOutputRD;
  int inputRDArgNum;
  int outputRDArgNum;
  char args[256][32];
  int numArgs;
};

//this funtion will allocate the memory for the command holder
struct CommandHolder* allocateComm();

//this function populates the struct with the information from user input
void generateComm(struct CommandHolder* commH, char* inputS);

//this function deallocates memory from the command holder struct
void deallocateComm(struct CommandHolder* kComm);
#endif
