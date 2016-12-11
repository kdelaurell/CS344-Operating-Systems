/*************************************************
* Author: Kyle De Laurell
* Date: 10/27/16
* Description: Creates and uses text files to play
* adventure game.
**************************************************/
#include <stdlib.h>

//main function
int main(){
  //intialize char array and int array
  char dirNames[24];
  char fileNames[36];
  char roomNames[10][15];
  char selectedRooms[7][15];
  int connectionArrays[7][7];

  //randomize seed
  srand(time(NULL));
  int PID;

  //gets process ID
  PID = getpid();
  createRooms(roomNames);
  assignroomNames(PID, dirNames, fileNames, roomNames, selectedRooms, connectionArrays);
  runGame(PID,dirNames,selectedRooms);
  return 0;
}

//creates Room Names
createRooms(char roomName[][15]){
  strcpy(roomName[0], "Lobby");
  strcpy(roomName[1], "Galley");
  strcpy(roomName[2], "Closet");
  strcpy(roomName[3], "Window");
  strcpy(roomName[4], "Locker");
  strcpy(roomName[5], "Fan");
  strcpy(roomName[6], "Tanning");
  strcpy(roomName[7], "Living");
  strcpy(roomName[8], "Ballas");
  strcpy(roomName[9], "Hot");
}

//assigns room names randomnly and what rooms they are connected to
assignroomNames(int processId, char dirName[24], char fileName[36], char roomNames[][15], char selectedRooms[][15], int connectionArray[][7]){
   //intitiates variables
   FILE *file;
   int choice;
   int numConnect;
   int startRoom;
   int endRoom;
   int i;
   int j;

   //creates directory
   sprintf(dirName, "delaurek.rooms.%d", processId);
   int result = mkdir(dirName, 0755);
   int selectedCount = 0;
   for(i=0; i < 7; i++){
      choice = rand() % 10;
      if(strcmp(roomNames[choice], "")){
        strcpy(selectedRooms[selectedCount], roomNames[choice]);
        selectedCount++;
        strcpy(roomNames[choice], "");
      }
      else{
        i--;
      }
   }

   //intitiates array of connections
   for(i=0; i < 7; i++){
     for(j=0; j < 7; j++){
       if(i!=j){
         connectionArray[i][j] = 0;
       }
       else{
         connectionArray[i][j] = -1;
       }
     }
   }

   //builds array for connections
   for(i=0; i < 7; i++){
     while(sumArray(connectionArray, i) < 3){
       choice = rand() % 7;
       if(connectionArray[i][choice] != -1 && connectionArray[i][choice] != 1){
         connectionArray[i][choice] = 1;
         connectionArray[choice][i] = 1;
       }
     }
   }

   //ensures start and end room do not equal eachother
   startRoom = rand() % 7;
   do{
     endRoom = rand() % 7;
   }while(endRoom == startRoom);

   //fills files with room name file with connections and type
   for(i=0; i < 7; i++){
     sprintf(fileName, "%s/%s.txt", dirName, selectedRooms[i]);
     file = fopen(fileName, "w+");
     fprintf(file, "ROOM NAME: %s\n", selectedRooms[i]);
     numConnect = 0;
     for(j=0; j < 7; j++){
       if(connectionArray[i][j] == 1){
         numConnect++;
         fprintf(file, "CONNECTION %d: %s\n", numConnect, selectedRooms[j]);
       }
     }
     if(i == startRoom){
       fprintf(file, "ROOM TYPE: START_ROOM\n");
     }
     else if(i == endRoom){
       fprintf(file, "ROOM TYPE: END_ROOM\n");
     }
     else{
       fprintf(file, "ROOM TYPE: MID_ROOM\n");
     }
     fclose(file);
   }
}

//summs array
int sumArray(int array[][7], int i){
  int k;
  int sum = 0;
  for(k=0; k < 7; k++){
    sum = sum + array[i][k];
  }
  return sum;
}

//runs the game
runGame(int ProId, char dirN[24], char gameRoomNames[][15]){
  char roomFileName[36];
  struct dirent* in_dir;
  char inputText[60];
  char inGameRoomNames[7][15];
  char inGameConnections[7][7][15];
  char inGameRoomType[7][15];
  char connArr[7][15];
  char userChoice[15];
  char inputString[15];
  char userTrack[30][15];
  int userTrackNum;
  int userChoiceRN;
  int numConnected;
  int length;
  int i;
  int k;
  int l;
  int startRoomNum;
  int endRoomNum;
  FILE *roomFile;


  //reads files and creates arrays for game
  for(i=0; i < 7; i++){
    numConnected = 0;
    sprintf(roomFileName, "%s/%s.txt", dirN, gameRoomNames[i]);
    roomFile = fopen(roomFileName, "r+");

    //reads room file
    while(fgets(inputText, 60, roomFile) != NULL){
      length = strlen(inputText);
      memmove(inputText+(length-1), "\0", 2);
      if(strncmp(inputText, "ROOM NAME", 9) == 0){
        strcpy(inGameRoomNames[i], &inputText[11]);
      }
      else if(strncmp(inputText, "CONNECTION", 10) == 0){
        strcpy(inGameConnections[i][numConnected], &inputText[14]);
        numConnected++;
      }
      else if(strncmp(inputText, "ROOM TYPE", 9) == 0){
        strcpy(inGameRoomType[i], &inputText[11]);
        if(strcmp(inGameRoomType[i], "START_ROOM") == 0){
          startRoomNum = i;
        }
        else if(strcmp(inGameRoomType[i], "END_ROOM") == 0){
          endRoomNum = i;
        }
      }
    }
    while(numConnected <= 6){
      strcpy(inGameConnections[i][numConnected], "");
      numConnected++;
    }
    fclose(roomFile);
  }


  userTrackNum = 0;
  userChoiceRN = startRoomNum;
  strcpy(userTrack[userTrackNum], inGameRoomNames[startRoomNum]);
  printf("\nCURRENT LOCATION: %s\n", inGameRoomNames[startRoomNum]);
  printf("POSSIBLE CONNECTIONS: ");
  numConnected = 0;
  while(strcmp(inGameConnections[startRoomNum][numConnected], "") != 0 && numConnected <= 6){
    if(numConnected == 0){
      printf("%s", inGameConnections[startRoomNum][numConnected]);
    }
    else{
      printf(", %s", inGameConnections[startRoomNum][numConnected]);
    }
    numConnected++;
  }
  printf(".\nWHERE TO? >");

  //loops through gets user input validates it and displays room until
  //reach the end room
  while(userChoice != inGameRoomNames[endRoomNum]){
    fgets(inputString, 15, stdin);
    sscanf(inputString, "%s", userChoice);

    userChoiceRN = ValidateInput(inGameRoomNames, userChoice);
    if(userChoiceRN == -1){
      printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
      userChoiceRN = ValidateInput(inGameRoomNames, userTrack[userTrackNum]);
    }
    else if(userChoiceRN == endRoomNum){
      userTrackNum++;
      strcpy(userTrack[userTrackNum], inGameRoomNames[userChoiceRN]);
      printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
      printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", userTrackNum+1);
      for(l = 0; l <= userTrackNum; l++){
        printf("%s\n", userTrack[l]);
      }
      return;
    }
    else{
      userTrackNum++;
      strcpy(userTrack[userTrackNum], inGameRoomNames[userChoiceRN]);
    }
    printf("\nCURRENT LOCATION: %s\n", inGameRoomNames[userChoiceRN]);
    printf("POSSIBLE CONNECTIONS: ");
    numConnected = 0;
    while(strcmp(inGameConnections[userChoiceRN][numConnected], "") != 0 && numConnected <= 6){
      if(numConnected == 0){
        printf("%s", inGameConnections[userChoiceRN][numConnected]);
      }
      else{
        printf(", %s", inGameConnections[userChoiceRN][numConnected]);
      }
      numConnected++;
    }
    printf(".\nWHERE TO? >");
  }
}

//validates room
int ValidateInput(char RN[][15], char UC[15]){
  int j;
  for(j = 0; j < 7; j++){
    if(strcmp(RN[j], UC) == 0){
      return j;
    }
  }
  return -1;
}
