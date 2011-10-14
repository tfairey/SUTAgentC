/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Android SUTAgent code.
 *
 * The Initial Developer of the Original Code is
 * Bob Moss.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Bob Moss <bmoss@mozilla.com>
 *  Trevor Fairey <tfairey@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of coeither the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "CmdWorkerThread.h"
#include "RunCmdThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
extern int* used;

void readLine(FILE* in, char* response) {
 char temp[1024];
 fgets(temp, 1024, in);
 while(temp[strlen(temp)-1] == '\n' || temp[strlen(temp)-1] == '\r') {
   temp[strlen(temp)-1] = '\0';
 }
 strcpy( response, temp );
}

void run(int socket) {
  char input[1024]="";
  char outputLine[4096]="";
  char prompt[3] = "$>";
  FILE* cmdOut = tmpfile();
  FILE* cmdIn = tmpfile();
  char tempBuf[1024];
  int tmpBytes;
  char* curDir = malloc(1024*sizeof(char));
  int bListening = 1;
  strcpy(curDir,"/");
  pthread_setspecific(workerKey, curDir);
  pthread_setspecific(listeningKey, &bListening);

  send(socket, prompt, 3, 0); 
  while(bListening) {
    /*if( !feof(cmdIn) ) {
    
  

     }*/
    tmpBytes = -1;
    rewind(cmdIn);
    while(tmpBytes < 0) {
      while( (tmpBytes = recv(socket, tempBuf, 1, 0)) && tmpBytes > 0 )
      {
        //if(tempBuf[0] == '\n')
        //{
        //  break;
        //}
        //printf("tmpBytes == %d\n", tempBytes);
        //printf("IBuffer == %s\n", tempBuf);
        if(tempBuf[0] == '\n')
        {
          break;
        }
        fwrite(tempBuf, 1, 1, cmdIn);
      }
    }
    //printf("NumBytes = %d\n", tmpBytes);
    //printf("Buffer = %s\n", tempBuf);
    fwrite(tempBuf,1, 1, cmdIn);
    fflush(cmdIn);
    rewind(cmdIn);
    strcpy(input, "");
    readLine(cmdIn, input);
    //recv(socket, tempBuf, strlen(input), 0);
    //printf("numBytesReceived=%d\n",tmpBytes);
    //printf("numBytesUsed=%d\n",strlen(input));
    if( strcmp(input, "")) {
      strcpy(outputLine, "");
      printf("Input = %s\n", input);
      processCommand(input, socket, outputLine);
      //printf("output = %s", outputLine);
      if(strlen(outputLine) > 0) {
        fputs(outputLine, cmdOut);
        fputc('\n',cmdOut);
        fputs(prompt, cmdOut);
        fputc('\0', cmdOut);
        fflush(cmdOut);
        rewind(cmdOut);
       //printf("tempBuf = %s\n", tempBuf);
        int bytez = 0;
        while( (bytez = fread(tempBuf, 1, 1024, cmdOut)) == 1024)
        {
          send(socket, tempBuf, bytez, 0);
        }
       //printf("tempBuf = %s\n", tempBuf);
        send(socket, tempBuf, bytez, 0); 
        strcpy(tempBuf, "");
        rewind(cmdOut);
      }
      else {
        send(socket, prompt, 3, 0); 
      }
      fclose(cmdOut);
      fclose(cmdIn);
      cmdIn = tmpfile();
      cmdOut = tmpfile();
      if(!strcmp(outputLine, "exit")) {
        AllStopListening();
        bListening = 0;
      }
      if(!strcmp(outputLine, "quit")) {
        bListening = 0;
      }
    }
    else {
      break;
    }
  }
  fclose(cmdIn);
  fclose(cmdOut);
  
  close(socket);
}
/*
int main(int argc, char *argv[]) {
  run();
}*/
