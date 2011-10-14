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
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <netinet/in.h>

int bAllListening = 1;
pthread_t threads[1024];

void AllStopListening() {
  bAllListening = 0;
}
void free_key(void *currDir) {
    free(currDir);
}

void endChildListening(void *bListening) {
  puts("BAM!\n");
    *(char*)bListening = 0;
}

void RunCmdThread(int sockfd) {
  printf("sockfd = %d", sockfd);
  int nConnections = 0;
  //fd_set & select required to add a timeout.
  fd_set socketSet;
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  //Set timeout to 5000?
  pthread_key_create(&workerKey, *free_key);
  pthread_key_create(&listeningKey, *endChildListening);
  listen(sockfd, 5);
  while(bAllListening) {
    //Must define the set of sockets each time it is selected.
    FD_ZERO(&socketSet);
    FD_SET(sockfd, &socketSet);
    int temp = select(sockfd+1, &socketSet, NULL, NULL, &timeout);
    if(temp == 0) {
      //puts(".");
      continue;
    }
    //const struct sockaddr* socket = malloc(sizeof(const struct sockaddr));
    
    int* clientFD = accept(sockfd, NULL, NULL);
    //threads[nConnections] = malloc(sizeof(p_thread));
    pthread_create(&threads[nConnections], NULL, *run, clientFD);
    nConnections++;
    nConnections = nConnections % 1024;
  }
  int i;
 pthread_key_delete(listeningKey);
  for(i = 0; i < 1024; i++) {
    pthread_join(threads[i], NULL); 
  }
  close(sockfd);

 pthread_key_delete(workerKey);

}
/*
int main(int argc, char** argv) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(20701);
  address.sin_addr.s_addr = INADDR_ANY;
  int bindResult;
  puts("BIM!\n");
  if( (bindResult = bind(sockfd, (struct sockaddr *)&address, sizeof(address)))==0 )
  {
    puts("BAM!\n");
    RunCmdThread(sockfd);
    puts("BOMB?");
  }
  else {
    printf("Result = %d\n", bindResult);
  }

}
*/






