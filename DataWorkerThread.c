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

#include "DataWorkerThread.h"
#include "RunDataThread.h"
#include "CmdWorkerThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
extern int* used;
extern char uniqueID[20];

void timestamp(char* sRet)
  {
    strcpy(sRet, "");
    FILE* inputstream;
    inputstream = popen("date \"+%Y%m%d-%H:%M:%S\"","r");
    char temp;
    while( (temp = fgetc(inputstream)) != EOF && temp != '\n')
    {
      strncat(sRet, &temp, 1);
    }       
    strcat(sRet, "");
    pclose(inputstream);                                   
    return;
  }

void runData(int socket) {
  FILE* cmdOut = tmpfile();
  FILE* cmdIn = tmpfile();
  char heartbeat[1024];
  char* curDir = malloc(1024*sizeof(char));
  int bListening = 1;
  long lEndTime = time(NULL) + 60;
  strcpy(curDir,"/");
  pthread_setspecific(dworkerKey, curDir);
  pthread_setspecific(dlisteningKey, &bListening);
  timestamp(heartbeat);
  strcat(heartbeat, " trace output\n");
  send(socket, heartbeat, strlen(heartbeat), 0); 
  while(bListening) {
    //printf("%ld vs %ld\n", time(NULL), lEndTime); 
    if( time(NULL) > lEndTime )
    {
      timestamp(heartbeat);
      strcat(heartbeat, " Thump thump - ");
      strcat(heartbeat, uniqueID);
      if(send(socket, heartbeat, strlen(heartbeat), 0) <= 0)
      {
        break;
      }
      lEndTime += 60;
    }/*
    tmpBytes = -1;
    rewind(cmdIn);
    while( (tmpBytes = recv(socket, tempBuf, 1024, 0)) == 0 )
    {
      //printf("Buffer == %s", tempBuf);
      fwrite(tempBuf, 1, 1024, cmdIn);
    }
    if(tmpBytes > 0) {
      fwrite(tempBuf,1, tmpBytes, cmdIn);
      fflush(cmdIn);
      rewind(cmdIn);
    }
    strcpy(input, "");
    readLine(cmdIn, input);
    //printf("NumBytes = %d", tmpBytes);
    //printf("Buffer = %s\n", tempBuf);
    //printf("input=%s\n",input);
    if( strcmp(input, "")) {
      strcpy(outputLine, "");
      processCommand(input, cmdIn, socket,outputLine);
      //printf("output = %s", outputLine);
      if(strlen(outputLine) > 0) {
        fputs(outputLine, cmdOut);
      }
      fputs("\n",cmdOut);
      fflush(cmdOut);
      rewind(cmdOut);
     //printf("tempBuf = %s\n", tempBuf);
      int bytez = 0;
      while( (bytez += fread(tempBuf, 1, 1024, cmdOut)) % 1024 == 0)
      {
        send(socket, tempBuf, 1024, 0);
      }
      tempBuf[bytez] = '\0';
     //printf("tempBuf = %s\n", tempBuf);
      send(socket, tempBuf, bytez%1024, 0); 
      strcpy(tempBuf, "");
      rewind(cmdOut);
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
      }*/
    
  }
  fclose(cmdIn);
  fclose(cmdOut);
  
    close(socket);
}
/*
int main(int argc, char *argv[]) {
  run();
}*/
