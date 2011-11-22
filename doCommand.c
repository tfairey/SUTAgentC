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

#include "doCommand.h"
#include "CmdWorkerThread.h"
#include "RedirOutputThread.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
//char* lineSep = "\n";
char*    sErrorPrefix = "##AGENT-WARNING## ";
int  bTraceOn = 0;


char* ffxProvider = "org.mozilla.ffxcp";
char* fenProvider = "org.mozilla.fencp";

char* prgVersion = "SUTAgentC Version 1.02";

int getCmd(char* sCmd)
{
  int retCmd = UNKNOWN;
  int i;
  //Go through the commands and if it matches, select the appropriate ENUM.
  //Note, this only works if you keep the two lists in sync.
  for(i = 0; i < sizeof(commands)/sizeof(char*); i++)
  {
    if(strcasecmp(commands[i], sCmd) == 0)
    {
      retCmd = i;
      break;
    }
  }
  return (int)(retCmd);
}



void processCommand(char* theCmdLine, int socket, char* strReturn)
      {
      int cCmd = UNKNOWN;
      int cSubCmd = UNKNOWN;

      int Argc = 1;
      printf("\nCmdLine = '%s'\n",  theCmdLine);
      //Note, Argv is malloc'd by parseCmdLine2, and must be freed,
      //along with malloc'd strings withing it at the end of this function.
      char** Argv = parseCmdLine2(theCmdLine, &Argc);
      strcpy(strReturn, "");

      cCmd = getCmd(Argv[0]);
      printf("\nCommand Number %d with %d arguments\n",cCmd, Argc-1);

      switch(cCmd)
          {
          case TRACE:
              if (Argc == 2) {
                  bTraceOn = strcasecmp(Argv[1],"on") == 0 ? 1 : 0;
              } else {
                  sprintf(strReturn, "%sWrong number of arguments for trace command!",sErrorPrefix);
              }
              break;

          case VER:
              strcpy(strReturn, prgVersion);
              break;

          case CLOK:
              GetClok(strReturn); 
              break;

          case TZGET:
              //strcpy(strReturn,  GetTimeZone()); 
              break;

          case TZSET:
              if (Argc == 2) {
                  //strcpy(strReturn,  SetTimeZone(Argv[1])); 
              } else {
                  sprintf(strReturn, "%sWrong number of arguments for settz command!",sErrorPrefix);
              }
              break;

          case UPDT:
              if (Argc >= 2){
                  //strcpy(strReturn,  StrtUpdtOMatic(Argv[1], Argv[2], (Argc > 3 ? Argv[3] : null), (Argc > 4 ? Argv[4] : null))); 
              } else {
                  sprintf(strReturn, "%sWrong number of arguments for updt command!", sErrorPrefix);
              }
              break;

            case SETTIME:
                SetSystemTime(Argv[1], (Argc > 2 ? Argv[2] : NULL), socket, strReturn); 
                break;

            case CWD:
                strcpy(strReturn, (char*)pthread_getspecific(workerKey));
                break;

            case CD:
                if (Argc == 2){
                    changeDir(Argv[1], strReturn);
                } else {
                  sprintf(strReturn, "%sWrong number of arguments for cd command!",sErrorPrefix);
                }
                break;

            case LS:
                printf("Current Directory = %s\n", (char*)pthread_getspecific(workerKey));
                PrintDir(((Argc > 1) ? Argv[1] : (char*)pthread_getspecific(workerKey)), strReturn);
                break;

            case GETAPPROOT:
                if (Argc == 2) {
                    GetAppRoot(Argv[1], strReturn); 
                } else { 
                  sprintf(strReturn, "%sWrong number of arguments for getapproot command!",sErrorPrefix);
                }
                break;

            case ISDIR:
                if (Argc == 2) {
                    isDirectory(Argv[1], strReturn); 
                } else {
                  sprintf(strReturn, "%sWrong number of arguments for isdir command!",sErrorPrefix);
                }
                break;

            case TESTROOT:
                GetTestRoot(strReturn); 
                break;

            case DEAD:
                if (Argc == 2) {
                    if(IsProcessDead(Argv[1])==0) {
                      sprintf(strReturn, "%s is hung or unresponsive", Argv[1]);
                    } else {
                      sprintf(strReturn, "%s is ok", Argv[1]);
                    }
                } else {
                  sprintf(strReturn, "%sWrong number of arguments for dead command!",sErrorPrefix);
                }
                break;

            case PS:
                GetProcessInfo(strReturn); 
                break;

            case PULL:
                if (Argc >= 2) {
                    long lOff = 0;
                    long lLen = -1;
                    if (Argc > 2) {
                        printf("offset = %s", Argv[2]);
                    }
                    if (Argc == 4) {
                        printf("length = %s", Argv[3]);
                    }
                    Pull(Argv[1], lOff, lLen, socket, strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for pull command!",sErrorPrefix);
                }
                break;

            case PUSH:
                if (Argc == 3) {
                    printf("long l = %s", Argv[2]);
                    Push(Argv[1], socket, atol(Argv[2]), strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for push command!",sErrorPrefix);
                }
                break;

            case INST:
                if (Argc >= 2) {
                    //strcpy(strReturn,  InstallApp(Argv[1], outSocket)); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for inst command!",sErrorPrefix);
                }
                break;

            case UNINST:
                if (Argc >= 2) {
                    //strcpy(strReturn,  UnInstallApp(Argv[1], outSocket)); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for uninst command!",sErrorPrefix);
                }
                break;

            case ALRT:
                if (Argc > 1) {
                    if (strcmp(Argv[1],"on") == 0)
                        {
                        char* sTitle = "Agent Alert";
                        char* sMsg = "The Agent Alert System has been activated!";
                        if (Argc == 3) {
                            sTitle = Argv[2];
                            sMsg = "";
                        } else if (Argc == 4) {
                            sTitle = Argv[2];
                            sMsg = Argv[3];
                        }
                        StartAlert(sTitle, sMsg);
                        } else {
                        StopAlert();
                        }
                  } else {
                    sprintf(strReturn, "%sWrong number of arguments for alrt command!",sErrorPrefix);
                  }
                break;

            case REBT:
                if (Argc >= 1) {
                    RunReboot(socket, (Argc > 1 ? Argv[1] : NULL), (Argc > 2 ? Argv[2] : NULL), strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for rebt command!",sErrorPrefix);
                }
                break;

            case TMPD:
                //strcpy(strReturn,  GetTmpDir()); 
                break;

            case DEVINFO:
                if (Argc == 1) {
                 //   strcat(strReturn, SUTAgentC.sUniqueID);
                    GetOSInfo(strReturn);
                    GetSystemTime(strReturn);
                    GetUptime(strReturn);
                 //   strcat(strReturn, GetScreenInfo());
                 //   strcat(strReturn, GetMemoryInfo());
                  //  strcat(strReturn, GetPowerInfo());
                    GetProcessInfo(strReturn);
                  } else {
                    cSubCmd = getCmd(Argv[1]);
                    switch(cSubCmd)
                        {
                        case ID:
                  //          strcpy(strReturn,  SUTAgentC.sUniqueID); 
                            break;

                        case SCREEN:
                  //          strcpy(strReturn,  GetScreenInfo()); 
                            break;

                        case PROCESS:
                            GetProcessInfo(strReturn); 
                            break;

                        case OS:
                            GetOSInfo(strReturn); 
                            break;

                        case SYSTIME:
                            GetSystemTime(strReturn); 
                            break;

                        case UPTIME:
                            GetUptime(strReturn); 
                            break;

                        case MEMORY:
                      //      strcpy(strReturn,  GetMemoryInfo()); 
                            break;

                        case POWER:
                        //    strcpy(strReturn,  GetPowerInfo()); 
                            break;

                        default:
                            break;
                        }
                    }
                break;

            case STAT:
                if (Argc == 2) {
                    //strcpy(strReturn,  StatProcess(Argv[1])); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for stat command!", sErrorPrefix);
                }
                break;

            case PING:
                if (Argc == 2) {
                    SendPing(Argv[1], socket, strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for ping command!", sErrorPrefix);
                }
                break;

            case HASH:
                if (Argc == 2) {
                    HashFile(Argv[1], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for hash command!", sErrorPrefix);
                }
                break;

            case PRUNE:
                if (Argc == 2) {
                    PruneDir(Argv[1], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for prune command!", sErrorPrefix);
                }
                break;

            case FTPG:
                if (Argc == 4) {
                    FTPGetFile(Argv[1], Argv[2], Argv[3], socket, strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for ftpg command!", sErrorPrefix);
                }
                break;

            case CAT:
                if (Argc == 2) {
                  Cat(Argv[1], socket, strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for cat command!", sErrorPrefix);
                }
                break;

            case DIRWRITABLE:
                if (Argc == 2) {
                  IsDirWritable(Argv[1], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for dirwritable command!", sErrorPrefix);
                }
                break;

            case TIME:
                if (Argc == 2) {
                    PrintFileTimestamp(Argv[1], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for time command!", sErrorPrefix);
                }
                break;

            case MKDR:
                if (Argc == 2) {
                  MakeDir(Argv[1], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for mkdr command!", sErrorPrefix);
                }
                break;

            case RM:
                if (Argc == 2) {
                    RemoveFile(Argv[1], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for rm command!", sErrorPrefix);
                }
                break;

            case MV:
                if (Argc == 3) {
                    Move(Argv[1], Argv[2], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for mv command!", sErrorPrefix);
                }
                break;

            case CP:
                if (Argc == 3) {
                    CopyFile(Argv[1], Argv[2], strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for cp command!", sErrorPrefix);
                }
                break;

            case QUIT:
            case EXIT:
                strcpy(strReturn,  Argv[0]); 
                break;

            case DBG:
              //  Debug.waitForDebugger();
              //  strcpy(strReturn,  "waitForDebugger on"); 
                break;

            case ADB:
            sprintf(strReturn, "%sThis isn't an android device, but if it is you should use SUTAgentAndroid!", sErrorPrefix);
            /*
                if (Argc == 2) {
                    if (strstr(Argv[1], "ip")!=NULL || strstr(Argv[1],"usb")!=NULL) {
                       // strcpy(strReturn,  SetADB(Argv[1])); 
                    } else {
                        sprintf(strReturn, "%sUnrecognized argument for adb command!", sErrorPrefix);
                    }
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for adb command!", sErrorPrefix);
                }*/
                break;

            case TEST:
            /*
                long lFreeMemory = 0;//Runtime.getRuntime().freeMemory();
                long lTotMemory = 0;//Runtime.getRuntime().totalMemory();
                long lMaxMemory = 0;//Runtime.getRuntime().maxMemory();


                if (lFreeMemory > 0) {
                    sprintf(strReturn, "Max memory:  %s\nTotal Memory: %s\nFree memory: %s", lMaxMemory,lTotMemory,lFreeMemory);
                    break;
                }

                ContentResolver cr = contextWrapper.getContentResolver();
                Uri ffxFiles = null;

                if (Argv[1].contains("fennec")) {
                    ffxFiles = Uri.parse("content://" + fenProvider + "/dir");
                } else if (Argv[1].contains("firefox")) {
                    ffxFiles = Uri.parse("content://" + ffxProvider + "/dir");
                }

//                Uri ffxFiles = Uri.parse("content://org.mozilla.fencp/file");
                char*[] columns = new char*[] {
                        "_id",
                        "isdir",
                        "filename",
                        "length"
                    };
//                char*[] columns = new char*[] {
//                        "_id",
//                        "chunk"
//                     };
                Cursor myCursor = cr.query(    ffxFiles,
                                            columns,                         // Which columns to return
                                            (Argc > 1 ? Argv[1] : null),    // Which rows to return (all rows)
                                            null,                           // Selection arguments (none)
                                            null);                            // Put the results in ascending order by name

                if (myCursor != null) {
                    int nRows = myCursor.getCount();
                    char* [] colNames = myCursor.getColumnNames();
                    int    nID = 0;
                    int nBytesRecvd = 0;

                    for (int lcv = 0; lcv < nRows; lcv++) {
                        if  (myCursor.moveToPosition(lcv)) {
                            nID = myCursor.getInt(0);
                            byte [] buf = myCursor.getBlob(1);
                            if (buf != null) {
                                nBytesRecvd += buf.length;
                                strReturn += new char*(buf);
                                buf = null;
                            }
                        }
                    }
                    strReturn += "[eof - " + nBytesRecvd + "]";
                    myCursor.close();
                }


                if (myCursor != null)
                    {
                    int nRows = myCursor.getCount();
                    int    nID = 0;
                    char* sFileName = "";
                    long lFileSize = 0;
                    boolean bIsDir = false;

                    for (int lcv = 0; lcv < nRows; lcv++)
                        {
                        if  (myCursor.moveToPosition(lcv))
                            {
                            nID = myCursor.getInt(0);
                            bIsDir = (myCursor.getInt(1) == 1 ? true : false);
                            sFileName = myCursor.getchar*(2);
                            lFileSize = myCursor.getLong(3);
                            strReturn += "" + nID + "\t" + (bIsDir ? "<dir> " : "      ") + sFileName + "\t" + lFileSize + "\n";
                            }
                        }
                    myCursor.close();
                    }
                break;
*/
            case EXEC:
            case ENVRUN:
                if (Argc >= 2)
                    {
                    int i;
                    for(i = 0; i < Argc; i++)
                    {
                      printf("arg %d = %s\n", i, Argv[i]); 
                    }


                    Argv += 1;
                    StartPrg2(Argv, Argc-1, socket, strReturn); 
                    Argv -= 1;
                    }
                else
                    {
                    sprintf(strReturn, "%sWrong number of arguments for %s command!", sErrorPrefix, Argv[0]);
                    }
                break;
            case RUN:
                if (Argc >= 2)
                    {
                        Argv += 1;
                        StartPrg(Argv, Argc-1, socket, strReturn);
                        Argv -= 1;
                    }
                else
                    {
                    sprintf(strReturn, "%sWrong number of arguments for %s command!", sErrorPrefix, Argv[0]);
                    }
                break;
            case KILL:
                if (Argc == 2) {
                    KillProcess(Argv[1], socket, strReturn); 
                } else {
                    sprintf(strReturn, "%sWrong number of arguments for kill command!", sErrorPrefix);
                }
                break;

            case DISK:
                //strcpy(strReturn,  GetDiskInfo((Argc == 2 ? Argv[1] : "/"))); 
                break;

            case UNZP:
                Unzip(Argv[1], (Argc == 3 ? Argv[2] : ""), strReturn); 
                break;

            case ZIP:
                Zip(Argv[1], (Argc == 3 ? Argv[2] : ""), strReturn); 
                break;

            case HELP:
                PrintUsage(strReturn); 
                break;

            default:
                sprintf(strReturn, "%s[%s] command", sErrorPrefix, Argv[0]);
                if (Argc > 1) {
                    char temp[1024];
                    int lcv;
                    strcat(strReturn, " with arg(s) =");
                    for (lcv = 1; lcv < Argc; lcv++) {
                        sprintf(temp, " [%s]", Argv[lcv]);
                        strcat(strReturn,temp);
                    }

                }
                strcat(strReturn," is currently not implemented.");
                break;
            }
        
          if(Argv != NULL && Argc > 0) {
              int i;
              for(i = 0; i < Argc; i++) {
                  free(Argv[i]);
              }
              free(Argv);
          }
          
          
          
        return;
      }
/*
private void SendNotification(char* tickerText, char* expandedText) {
        NotificationManager notificationManager = (NotificationManager)contextWrapper.getSystemService(Context.NOTIFICATION_SERVICE);
        int icon = R.drawable.ateamlogo;
        long when = System.currentTimeMillis();

        Notification notification = new Notification(icon, tickerText, when);

        notification.flags |= (Notification.FLAG_INSISTENT | Notification.FLAG_AUTO_CANCEL);
        notification.defaults |= Notification.DEFAULT_SOUND;
        notification.defaults |= Notification.DEFAULT_VIBRATE;
        notification.defaults |= Notification.DEFAULT_LIGHTS;

        Context context = contextWrapper.getApplicationContext();

        // Intent to launch an activity when the extended text is clicked
        Intent intent2 = new Intent(contextWrapper, SUTAgentAndroid.class);
        PendingIntent launchIntent = PendingIntent.getActivity(context, 0, intent2, 0);

        notification.setLatestEventInfo(context, tickerText, expandedText, launchIntent);

        notificationManager.notify(1959, notification);
    }

private void CancelNotification()
    {
    NotificationManager notificationManager = (NotificationManager)contextWrapper.getSystemService(Context.NOTIFICATION_SERVICE);
    notificationManager.cancel(1959);
    }
*/
void StartAlert(char* sTitle, char* sMsg)
        {
        // start the alert message
        //SendNotification(sTitle, sMsg);
        }

void StopAlert()
        {
        //CancelNotification();
        }

char *replace_str(char *str, char *orig, char *rep)
{
    static char buffer[4096];
      char *p;

      if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
            return str;

      strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
      buffer[p-str] = '\0';

      sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

    return buffer;
}

// NOTE, still need to get rid of the arraylist.
char** parseCmdLine2(char* theCmdLine, int* argc)
{
  char*  cmdString;
  char*  workingString;
  char  workingString2[1024]="";
  char** theArgs;
  char* lst[100];
  int nLength = 0;
  int args = 0;
  char* pFirstSpace = NULL;

  // Null cmd line
  if (theCmdLine == NULL) {
    *argc = 0;
    return NULL;
  }
  pFirstSpace = strchr(theCmdLine, ' ');
  if (pFirstSpace == NULL) {
    *argc = 1;
    theArgs = malloc(sizeof(char*));
    *theArgs = malloc(sizeof(theCmdLine));
    strcpy(*theArgs, theCmdLine);
    return(theArgs);
  }

  // Get the command
  int cmdSize = pFirstSpace-theCmdLine;
  cmdString = malloc(cmdSize+1);
  strncpy(cmdString, theCmdLine, cmdSize);
  cmdString[cmdSize] = '\0';
  
  lst[args++] = cmdString;
  // Jump past the command and trim
  workingString = pFirstSpace+1;

  //printf("cS = %d;",990);
  while ((nLength = strlen(workingString)) > 0) {
    char* pEnd = NULL;
    int nStart = 0;
    //printf("pFirstSpace=%s", pFirstSpace);
    //printf("nlength = %d; workingString = %s", nLength, workingString);
    //printf("nStart = %d; workingString+nStart = %s", nStart, workingString+nStart);
    //puts(">>>>>>\n");
    // if we have a quote
    if (workingString[0] == '"') {
    
      // point to the first non quote char
      nStart = 1;
      // find the matching quote
      pEnd = strchr(workingString+nStart, '"');

      char prevChar;
      while(pEnd != NULL) {
          // check to see if the quotation mark has been escaped
          prevChar = *(pEnd - 1);
          if (prevChar == '\\') {
              // if escaped, point past this quotation mark and find the next
              pEnd++;
              if (pEnd == '\0') { 
                  pEnd = strchr(pEnd,'"');
              } else {
                  pEnd = NULL;
              }
            } else {
                break;
            }
      }
      // there isn't one
      nStart = 0;
      if (pEnd == NULL) {
        // point at the quote
        // so find the next space
        pEnd = strchr(workingString,' ');
        // there isn't one of those either
        if (pEnd == NULL) {
           pEnd = workingString+nLength-1;    // Just grab the rest of the cmdline
        }
      }
    } else { // no quote so find the next space 
      pEnd = strchr(workingString+nStart,' ');
          // there isn't one of those
      //printf("pEnd=%s", pEnd);
      //printf("nlength = %d; workingString = %s", nLength, workingString);
      //puts(">>>>>>\n");

      if (pEnd == NULL) {
        pEnd = workingString + nLength-1;    // Just grab the rest of the cmdline
      }
    }
    // get the substring
    strncpy(workingString2, workingString+nStart, (int)(pEnd-workingString)+nStart+1);
    workingString2[(int)(pEnd-workingString)+nStart+1] = '\0';
    // if we have escaped quotes
    if (strstr(workingString2, "\\\"") != NULL) {
      do
          {
          // replace escaped quote with embedded quote
          strcpy(workingString2, replace_str(workingString2, "\\\"","\""));
          }
      while(strstr(workingString2,"\\\"") != NULL);
    }
    //printf("wS+nS = %s, pEnd-wS+sT = %d\n", workingString+nStart, pEnd-workingString+nStart);

    // add it to the list
    lst[args] = malloc(sizeof(workingString2)+1);
    strcpy(lst[args++],workingString2);
    //printf("\nWorkingString = %s\n",  workingString2);


    // if we are dealing with a quote
    if (nStart > 0)
        pEnd++; //  point past the end one

    // jump past the substring and trim it
    workingString = pEnd+1;
  }
  int lcv;
  //puts("BAM!");
  // ok we're done package up the results
  theArgs = (char**)(malloc(args * sizeof(char*)));
  //printf("args = %d",args);
  for (lcv = 0; lcv < args; lcv++)
  {
     // printf("Arg %d = %s", lcv, lst[lcv]);
      theArgs[lcv]= (lst[lcv]);
     // printf("Arg %d = %s", lcv, theArgs[lcv]);
  }
  *argc = args;
  return(theArgs);
}

//UNUSED
/*
char* [] parseCmdLine(char* theCmdLine) {
        char*    cmdchar*;
        char*    workingchar*;
        char*    workingchar*2;
        List<char*> lst = new ArrayList<char*>();
        int nLength = 0;
        int nFirstSpace = -1;

        // Null cmd line
        if (theCmdLine == null)
            {
            char* [] theArgs = new char* [1];
            theArgs[0] = new char*("");
            return(theArgs);
            }
        else
            {
            nLength = theCmdLine.length();
            nFirstSpace = theCmdLine.indexOf(' ');
            }

        if (nFirstSpace == -1)
            {
            char* [] theArgs = new char* [1];
            theArgs[0] = new char*(theCmdLine);
            return(theArgs);
            }

        // Get the command
        cmdchar* = new char*(theCmdLine.substring(0, nFirstSpace));
        lst.add(cmdchar*);

        // Jump past the command and trim
        workingchar* = (theCmdLine.substring(nFirstSpace + 1, nLength)).trim();

        while ((nLength = workingchar*.length()) > 0)
            {
            int nEnd = 0;
            int    nStart = 0;

            // if we have a quote
            if (workingchar*.startsWith("\""))
                {
                // point to the first non quote char
                nStart = 1;
                // find the matching quote
                nEnd = workingchar*.indexOf('"', nStart);
                // there isn't one
                if (nEnd == -1)
                    {
                    // point at the quote
                    nStart = 0;
                    // so find the next space
                    nEnd = workingchar*.indexOf(' ', nStart);
                    // there isn't one of those either
                    if (nEnd == -1)
                        nEnd = nLength;    // Just grab the rest of the cmdline
                    }
                else
                    {
                    nStart = 0;
                    nEnd++;
                    }
                }
            else // no quote so find the next space
                {
                nEnd = workingchar*.indexOf(' ', nStart);

                // there isn't one of those
                if (nEnd == -1)
                    nEnd = nLength;    // Just grab the rest of the cmdline
                }

            // get the substring
            workingchar*2 = workingchar*.substring(nStart, nEnd);

            // add it to the list
            lst.add(new char*(workingchar*2));

            // jump past the substring and trim it
            workingchar* = (workingchar*.substring(nEnd)).trim();
            }

        int nItems = lst.size();

        char* [] theArgs = new char* [nItems];

        for (int lcv = 0; lcv < nItems; lcv++)
            {
            theArgs[lcv] = lst.get(lcv);
            }

        return(theArgs);
        }
*/
void fixFileName(char* fileName, char* result)
      {
        char* sRet;
        char sTmpFileName[1024] = "";

        sRet = replace_str(fileName,"\\", "/");

        if (sRet[0] == '/' || (sRet[0] == '"'&& sRet[1] == '/')) {
            strcpy(sTmpFileName,sRet);
        } else {
            if(sRet[0] == '"') {
              sprintf(sTmpFileName,"\"%s/%s", (char*)pthread_getspecific(workerKey), sRet+1);
            } else {
              sprintf(sTmpFileName,"%s/%s", (char*)pthread_getspecific(workerKey), sRet);
            }
        }
        sRet = replace_str(sTmpFileName,"\\", "/");
        strcpy(sTmpFileName,sRet);
        sRet = replace_str(sTmpFileName,"//", "/");
        //Remove trailing spaces.
        while(sRet[strlen(sRet)-1]==' ') {
          sRet[strlen(sRet)-1] = '\0';
        }
        strcpy(result, sRet);
        return;
      }

void Zip(char* zipFileName, char* srcName, char* sRet)
        {
          char* sTmpZip = malloc(4096);
          fixFileName(zipFileName, sTmpZip);
          char sCommand[1024];
          sprintf(sCommand,"tar -C %s -czf %s %s", (char*)pthread_getspecific(workerKey), sTmpZip, srcName);
          FILE* inputstream;
          
          inputstream = popen(sCommand, "r");
          char temp; 
          strcpy(sRet, "");
          while( (temp = fgetc(inputstream)) != EOF && temp != '\0')
          {
           strncat(sRet, &temp, 1);
          }
          if(pclose(inputstream) > 0)
          {
            sprintf(sRet,"%sCould not zip files %s", sErrorPrefix, srcName);
          }
          strcat(sRet, "\nNo checksum in C version");
          free(sTmpZip);
          /*LOOK INTO ZLIB
        char*    fixedZipFileName = fixFileName(zipFileName);
        char*    fixedSrcName = fixFileName(srcName);
        char* sRet = "";

        try {
            FileOutputStream dest = new FileOutputStream(fixedZipFileName);
            CheckedOutputStream checksum = new CheckedOutputStream(dest, new Adler32());
            ZipOutputStream out = new ZipOutputStream(new BufferedOutputStream(checksum));
            out.setMethod(ZipOutputStream.DEFLATED);

            sRet += AddFilesToZip(out, fixedSrcName, "");

            out.close();
            System.out.println("checksum:                   "+checksum.getChecksum().getValue());
            sRet += "checksum:                   "+checksum.getChecksum().getValue();
            }
        catch(Exception e)
            {
            e.printStackTrace();
            }

        return(sRet);*/
    }

void Unzip(char* zipFileName, char* dstDirectory, char* sRet)
        {
          char* sTmpDir = malloc(4096);
          char* sTmpZip = malloc(4096);
          fixFileName(dstDirectory, sTmpDir);
          fixFileName(zipFileName, sTmpZip);
          char sCommand[1024];
          sprintf(sCommand,"tar -C %s -xzf %s", sTmpDir, sTmpZip);
          FILE* inputstream;
          
          inputstream = popen(sCommand, "r");
          char temp;
          strcpy(sRet, "");
          while( (temp = fgetc(inputstream)) != EOF && temp != '\0')
          {
           strncat(sRet, &temp, 1);
          }
          strcat(sRet, "No checksum in C version");
          if(pclose(inputstream) > 0)
          {
            sprintf(sRet,"%sCould not unzip files to %s", sErrorPrefix, sTmpDir);
          }
          free(sTmpDir);
          free(sTmpZip);
          /*LOOK INTO ZLIB
        char*     sRet = "";
        char*    fixedZipFileName = fixFileName(zipFileName);
        char*    fixedDstDirectory = fixFileName(dstDirectory);
        char*    dstFileName = "";
        int        nNumExtracted = 0;
        boolean bRet = false;

        try {
            final int BUFFER = 2048;
            BufferedOutputStream dest = null;
            ZipFile zipFile = new ZipFile(fixedZipFileName);
            int nNumEntries = zipFile.size();
            zipFile.close();

            FileInputStream fis = new FileInputStream(fixedZipFileName);
            CheckedInputStream checksum = new CheckedInputStream(fis, new Adler32());
            ZipInputStream zis = new ZipInputStream(new BufferedInputStream(checksum));
            ZipEntry entry;

            byte [] data = new byte[BUFFER];

            while((entry = zis.getNextEntry()) != null)
                {
                System.out.println("Extracting: " + entry);
                int count;
                if (fixedDstDirectory.length() > 0)
                    dstFileName = fixedDstDirectory + entry.getName();
                else
                    dstFileName = entry.getName();

                char* tmpDir = dstFileName.substring(0, dstFileName.lastIndexOf('/'));
                File tmpFile = new File(tmpDir);
                if (!tmpFile.exists())
                    {
                    bRet = tmpFile.mkdirs();
                    }
                else
                    bRet = true;

                if (bRet)
                    {
                    // if we aren't just creating a directory
                    if (dstFileName.lastIndexOf('/') != (dstFileName.length() - 1))
                        {
                        // write out the file
                        FileOutputStream fos = new FileOutputStream(dstFileName);
                        dest = new BufferedOutputStream(fos, BUFFER);
                        while ((count = zis.read(data, 0, BUFFER)) != -1)
                            {
                            dest.write(data, 0, count);
                            }
                        dest.flush();
                        dest.close();
                        dest = null;
                        fos.close();
                        fos = null;
                        }
                    nNumExtracted++;
                    }
                else
                    sRet += " - failed" + lineSep;
                }

            data = null;
            zis.close();
            System.out.println("Checksum:          "+checksum.getChecksum().getValue());
            sRet += "Checksum:          "+checksum.getChecksum().getValue();
            sRet += lineSep + nNumExtracted + " of " + nNumEntries + " sucessfully extracted";
            }
        catch(Exception e)
            {
            e.printStackTrace();
            }

        return(sRet);
*/        }

void StatProcess(char* string, char* sRet)
        {
          /*NOT NECESSARY FOR LINUX
        char* sRet = "";
        ActivityManager aMgr = (ActivityManager) contextWrapper.getSystemService(Activity.ACTIVITY_SERVICE);
        int    [] nPids = new int [1];

        nPids[0] = Integer.parseInt(string);

        android.os.Debug.MemoryInfo[] mi = aMgr.getProcessMemoryInfo(nPids);

        sRet  = "Dalvik Private Dirty pages         " + mi[0].dalvikPrivateDirty     + " kb\n";
        sRet += "Dalvik Proportional Set Size       " + mi[0].dalvikPss              + " kb\n";
        sRet += "Dalvik Shared Dirty pages          " + mi[0].dalvikSharedDirty      + " kb\n\n";
        sRet += "Native Private Dirty pages heap    " + mi[0].nativePrivateDirty     + " kb\n";
        sRet += "Native Proportional Set Size heap  " + mi[0].nativePss              + " kb\n";
        sRet += "Native Shared Dirty pages heap     " + mi[0].nativeSharedDirty      + " kb\n\n";
        sRet += "Other Private Dirty pages          " + mi[0].otherPrivateDirty      + " kb\n";
        sRet += "Other Proportional Set Size        " + mi[0].otherPss               + " kb\n";
        sRet += "Other Shared Dirty pages           " + mi[0].otherSharedDirty       + " kb\n\n";
        sRet += "Total Private Dirty Memory         " + mi[0].getTotalPrivateDirty() + " kb\n";
        sRet += "Total Proportional Set Size Memory " + mi[0].getTotalPss()          + " kb\n";
        sRet += "Total Shared Dirty Memory          " + mi[0].getTotalSharedDirty()  + " kb";


        return(sRet);*/
        }

void GetTestRoot(char* sRet)
        {
/*
        if (Environment.getExternalStorageState().equalsIgnoreCase(Environment.MEDIA_MOUNTED))
            {
            sRet = Environment.getExternalStorageDirectory().getAbsolutePath();
            }
        else*/
        {
        strcpy(sRet, "/testRoot");
        }

        return;//(sRet);
        }

void GetAppRoot(char* AppName, char* sRet)
        {
          /*Not sure what it does for Linux
        char* sRet = sErrorPrefix + " internal error [no context]";
        Context ctx = contextWrapper.getApplicationContext();

        if (ctx != null)
            {
            try {
                Context appCtx = ctx.createPackageContext(AppName, 0);
                ContextWrapper appCtxW = new ContextWrapper(appCtx);
                sRet = appCtxW.getPackageResourcePath();
                appCtxW = null;
                appCtx = null;
                ctx = null;
                System.gc();
                }
            catch (NameNotFoundException e)
                {
                e.printStackTrace();
                }
            }
        return(sRet);*/
        return;
        }

void isDirectory(char* sDir, char* sRet)
        {
        char* tmpDir = malloc(4096);
        fixFileName(sDir, tmpDir);
        strcat(sRet, sErrorPrefix);
        strcat(sRet, sDir);
        strcat(sRet, " does not exist");

        DIR* tmpFile = opendir(tmpDir);

        if (tmpFile != NULL) {
            closedir(tmpFile);
            strcpy(sRet, "TRUE");
        }
        else {
            FILE* tmp = fopen(tmpDir, "r");
            if( tmp != NULL )
            {
              fclose(tmp);
              strcpy(sRet, "FALSE");
            }
        }
        free(tmpDir);

        return;
        }


void changeDir(char* newDir, char* sRet)
        {
        char*    tmpDir    = malloc(4096);
        printf("newDir = %s", newDir);
        fixFileName(newDir, tmpDir);
        sprintf(sRet, "%sCouldn't change directory to %s", sErrorPrefix, tmpDir);

        char tempDirName[9999];
        strcpy(tempDirName, tmpDir);
        DIR* tmpFile = opendir((const char *)tmpDir);
        printf("tmpDirV2 = %s\n", tmpDir);
        printf("REALtmpDir = %s\n", tempDirName);

        if (tmpFile != NULL) {
          strcpy((char*)pthread_getspecific(workerKey), tmpDir);
          //sprintf(sRet, "Directory changed to %s", tmpDir);
          strcpy(sRet,"");
          printf("Directory changed to %s\n", tmpDir);
          closedir(tmpFile);
        } 

        free(tmpDir);
        return;
        }

char* HEXES = "0123456789abcdef";

void getHex( char* raw, char* sRet ) {
  int i;
  char* cooked;
  if ( raw == NULL )
  {
    return;
  }
  cooked = malloc(strlen(raw));

  for (i = 0; i< strlen(raw); i++ ) {
      cooked[i*2] = HEXES[(raw[i] & 0xF0) >> 4];
      cooked[i*2+1] = HEXES[(raw[i] & 0x0F)];
  }
  strcpy(sRet, cooked);
  free(cooked);
  return;
  }

void HashFile(char* fileName, char* sRet)
    {
        char*            sTmpFileName = malloc(4096);
        char            sCommand[1024];
        fixFileName(fileName, sTmpFileName);
        strcpy(sRet,"");
        sprintf(sCommand,"md5sum %s", sTmpFileName);
        FILE* inputstream;
        
        inputstream = popen(sCommand, "r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF && temp != ' ')
        {
         strncat(sRet, &temp, 1);
        }
        pclose(inputstream);
        free(sTmpFileName);
        
        return;
    }

void RemoveFile(char* fileName, char* sRet)
        {
        char*    sTmpFileName = malloc(4096);
        fixFileName(fileName, sTmpFileName);
        char    sCommand[1024] = "";
        sprintf(sRet, "%sCouldn't delete file %s",sErrorPrefix,sTmpFileName);
        sprintf(sCommand,"rm %s", sTmpFileName);

        if(system(sCommand)==0) {
            sprintf(sRet, "deleted %s", sTmpFileName);
        }
        free(sTmpFileName);
        return;
        }

void PruneDir(char* sDir, char* sRet)
{
  char*    sTmpDir = malloc(4096);
  fixFileName(sDir, sTmpDir);
  char    sCommand[1024] = "";
  sprintf(sRet, "%sCouldn't prune directory %s",sErrorPrefix,sDir);
  sprintf(sCommand,"rm -r %s", sTmpDir);
  if(system(sCommand)==0) {
    sprintf(sRet, "deleted %s", sDir);
  }
  free(sTmpDir);
  return;
}

void PrintDir(char* sDir, char* sRet)
      {
        char* sTmpDir = malloc(4096);
        fixFileName(sDir, sTmpDir);

        DIR* dir = opendir(sTmpDir);
        struct dirent *entry;
        char temp[1024];
        if (dir != NULL) {
            while(entry = readdir(dir)) {
                sprintf(temp, "%s\n", entry->d_name);
                if(strcmp(temp, ".\n") == 0 || strcmp(temp, "..\n") == 0)
                {
                  continue;
                }
                strcat( sRet, temp );
              }
            sRet[strlen(sRet)-1] = '\0';
            closedir(dir);
        }
        else {
            sprintf(sRet,"%s%s is not a directory", sErrorPrefix, sTmpDir);
        }
        free(sTmpDir);
        return;
    }

void Move(char* sTmpSrcFileName, char* sTmpDstFileName, char* sRet) {
        char sCommand[1024];

        char* sTmpSrc = malloc(4096);
        char* sTmpDst = malloc(4096);

        fixFileName(sTmpSrcFileName, sTmpSrc);
        fixFileName(sTmpDstFileName, sTmpDst);

        sprintf(sRet,"%sCould not move %s to %s", sErrorPrefix, sTmpSrcFileName, sTmpDstFileName);
        sprintf(sCommand,"mv %s %s", sTmpSrc, sTmpDst);

        printf("\nmv %s %s\n", sTmpSrc, sTmpDst);
        if (!system(sCommand)){
            sprintf( sRet,"%s moved to %s",sTmpSrcFileName, sTmpDstFileName);
        }
        free(sTmpSrc);
        free(sTmpDst);
        return;
    }

void CopyFile(char* sTmpSrcFileName, char* sTmpDstFileName, char* sRet) {
        char sCommand[1024];
        char* sTmpSrc = malloc(4096);
        char* sTmpDst = malloc(4096);
        fixFileName(sTmpSrcFileName, sTmpSrc);
        fixFileName(sTmpDstFileName, sTmpDst);
        sprintf(sRet,"%sCould not copy %s to %s", sErrorPrefix, sTmpSrc, sTmpDst);
        sprintf(sCommand,"cp %s %s", sTmpSrc, sTmpDst);
        if (!system(sCommand)){
            sprintf( sRet,"%s copied to %s",sTmpSrcFileName, sTmpDstFileName);
        }
        free(sTmpSrc);
        free(sTmpDst);
        return;
    }

void IsDirWritable(char* sDir, char* sRet)
    {
        char*    sTmpDir = malloc(4096);
        fixFileName(sDir, sTmpDir);
        sprintf(sRet,"%s[%s] is not a directory", sErrorPrefix, sTmpDir);

        DIR* dir = opendir(sTmpDir);

        if (dir != NULL) {
          char sCommand[1024];
          sprintf(sCommand,"test -w %s", sTmpDir);
          FILE* inputstream = popen(sCommand,"r");
          int writable;
          writable = pclose( inputstream );
          if( !writable )
          {
            sprintf(sRet, "[%s] is writeable", sTmpDir);
          }
          else
          {
            sprintf(sRet, "[%s] is not writeable", sTmpDir);
          }
        } else {
            sprintf(sRet, "%s[%s] is not a directory", sErrorPrefix, sTmpDir);
        }
        free(sTmpDir);
        return;
    }

void Push(char* fileName, int socket, long lSize, char* sRet)
    {
        char                buffer[8192];
        int                 nRead     = 1;
        long                lRead     = 0;
        char* sTmpFileName = malloc(4096);
        fixFileName(fileName, sTmpFileName);
        FILE*     dstFile;
        sprintf(sRet, "%sPush failed!", sErrorPrefix);

        dstFile = fopen(sTmpFileName, "wb");
        printf("dstFile = %p", dstFile);

            while((nRead > 0) && (lRead < lSize))
            {
                nRead = recv(socket, buffer, 8192, 0);
                if (nRead > 0) {
                    if (dstFile != NULL) {
                        fwrite(buffer, 1, nRead, dstFile);
                        fflush(dstFile);
                    }
                    lRead += (long)nRead;
                }
                else {
                  //printf("buffer = %s\n", buffer);
                }
                if(nRead != 8192 && nRead != lSize%8192)
                {
                  break;
                }
            }

            if (dstFile != NULL) {
                fflush(dstFile);
                fclose(dstFile);
            }

            if (lRead == lSize)    {
                HashFile(sTmpFileName, sRet);
            }
            else{
                  printf("lRead = %ld\n", lRead);
                  printf("lSize = %ld\n", lSize);
            }
      free(sTmpFileName);

      return;
    }

void FTPGetFile(char* sServer, char* sSrcFileName, char* sDstFileName, int outSocket, char* sRet)
        {
          /*
        byte[] buffer = new byte [4096];
        int    nRead = 0;
        long lTotalRead = 0;
        char* sRet = sErrorPrefix + "FTP Get failed for " + sSrcFileName;
        char* strRet = "";
        int    reply = 0;
        FileOutputStream outStream = null;
        char*    sTmpDstFileName = fixFileName(sDstFileName);

        FTPClient ftp = new FTPClient();
        try
            {
            ftp.connect(sServer);
            reply = ftp.getReplyCode();
            if(FTPReply.isPositiveCompletion(reply))
                {
                ftp.login("anonymous", "b@t.com");
                reply = ftp.getReplyCode();
                if(FTPReply.isPositiveCompletion(reply))
                    {
                    ftp.enterLocalPassiveMode();
                    if (ftp.setFileType(FTP.BINARY_FILE_TYPE))
                        {
                        File dstFile = new File(sTmpDstFileName);
                        outStream = new FileOutputStream(dstFile);
                        FTPFile [] ftpFiles = ftp.listFiles(sSrcFileName);
                        if (ftpFiles.length > 0)
                            {
                            long lFtpSize = ftpFiles[0].getSize();
                            if (lFtpSize <= 0)
                                lFtpSize = 1;

                            InputStream ftpIn = ftp.retrieveFileStream(sSrcFileName);
                            while ((nRead = ftpIn.read(buffer)) != -1)
                                {
                                lTotalRead += nRead;
                                outStream.write(buffer, 0, nRead);
                                strRet = "\r" + lTotalRead + " of " + lFtpSize + " bytes received " + ((lTotalRead * 100) / lFtpSize) + "% completed";
                                out.write(strRet.getBytes());
                                out.flush();
                                }
                            ftpIn.close();
                            @SuppressWarnings("unused")
                            boolean bRet = ftp.completePendingCommand();
                            outStream.flush();
                            outStream.close();
                            strRet = ftp.getReplychar*();
                            reply = ftp.getReplyCode();
                            }
                        else
                            {
                            strRet = sRet;
                            }
                        }
                    ftp.logout();
                    ftp.disconnect();
                    sRet = "\n" + strRet;
                    }
                else
                    {
                    ftp.disconnect();
                    System.err.println("FTP server refused login.");
                    }
                }
            else
                {
                ftp.disconnect();
                System.err.println("FTP server refused connection.");
                }
            }
        catch (SocketException e)
            {
            sRet = e.getMessage();
            strRet = ftp.getReplychar*();
            reply = ftp.getReplyCode();
            sRet += "\n" + strRet;
            e.printStackTrace();
            }
        catch (IOException e)
            {
            sRet = e.getMessage();
            strRet = ftp.getReplychar*();
            reply = ftp.getReplyCode();
            sRet += "\n" + strRet;
            e.printStackTrace();
            }
        return (sRet);
        */
        return;
    }


void Pull(char* fileName, long lOffset, long lLength, int outSocket, char* sRet)
        {
        char* sTmpFileName = malloc(4096);
        fixFileName(fileName, sTmpFileName);
        FILE*     srcFile;
        pthread_t rOThread;
        sprintf(sRet, "%sCould not read file %s!", sErrorPrefix, sTmpFileName);
        lLength = 0;
        srcFile = fopen(sTmpFileName, "rb");
        if(srcFile == NULL)
        {
          char temp[1024];
          sprintf(temp, "%s%s,-1\nNo such file or directory", sErrorPrefix, sTmpFileName);
          send(outSocket, temp, strlen(temp), 0);
          //strcpy(sRet, "");
        }
        else { 
          // send across name and file length 
          fseek(srcFile, 0L, SEEK_END);
          lLength = ftell(srcFile);
          rewind(srcFile);
          char temp[1024];
          sprintf(temp, "%s,%ld\n", sTmpFileName, lLength);
          send(outSocket, temp, strlen(temp), 0);

          struct ROTArgs args;
          args.outStream = srcFile;
          args.outSocket = outSocket;

          pthread_create(&rOThread, NULL, *RedirOutputThread, &args);
          pthread_join(rOThread, NULL);
          strcpy(sRet,"");
        }
      free(sTmpFileName);

      return;
    }

void Cat(char* fileName, int outSocket, char* sRet)
        {
        char*    sTmpFileName = malloc(4096);
        fixFileName(fileName, sTmpFileName);
        pthread_t rOThread;
        sprintf(sRet, "%sCould not read the file %s", sErrorPrefix, sTmpFileName);

        
          FILE* fin = fopen(sTmpFileName,"r");
          if( fin != NULL) {
            /*int tempChar;
            while((tempChar = fgetc(fin))!= EOF) {
              fputc(tempChar, out);
            }*/
            struct ROTArgs args;
            args.outStream = fin;
            args.outSocket = outSocket;
            printf("FILE* = %p, outSocket = %d\n", fin, outSocket);
            pthread_create(&rOThread, NULL, *RedirOutputThread, &args);

            pthread_join(rOThread, NULL);
            strcpy(sRet, "");
          }
        
        free(sTmpFileName);
        return;
    }

void MakeDir(char* sDir, char* sRet)
        {
        char* sTmpDir = malloc(4096);
        fixFileName(sDir, sTmpDir);
        char sCommand[1024];
        sprintf(sRet,"%sCould not make directory %s", sErrorPrefix, sTmpDir);
        sprintf(sCommand,"mkdir -p %s", sTmpDir);
        if (!system(sCommand)){
            sprintf( sRet,"Directory %s made",sTmpDir);
        }

        free(sTmpDir);
        return;
        }


    // move this to SUTAgentAndroid.java
void GetScreenInfo(char* sRet)
        {
          /*
        char* sRet = "";
        DisplayMetrics metrics = new DisplayMetrics();
        WindowManager wMgr = (WindowManager) contextWrapper.getSystemService(Context.WINDOW_SERVICE);
        wMgr.getDefaultDisplay().getMetrics(metrics);
        sRet = "X:" + metrics.widthPixels + " Y:" + metrics.heightPixels;
        */
        return;
        }


    // move this to SUTAgentAndroid.java
int*  GetScreenXY()
        {
          
            //int nRetXY[1] = {2};
           /* DisplayMetrics metrics = new DisplayMetrics();
            WindowManager wMgr = (WindowManager) contextWrapper.getSystemService(Context.WINDOW_SERVICE);
            wMgr.getDefaultDisplay().getMetrics(metrics);
            nRetXY[0] = metrics.widthPixels;
            nRetXY[1] = metrics.heightPixels;*/
            return NULL;
        }

void KillProcess(char* sProcName, int outSocket, char* sRet)
      {
        char sCommand[1024];
        int result;
        sprintf(sCommand, "kill %s", sProcName);
        result = system(sCommand);
        printf("Result of kill is %d\n", result);
        if (result > 0)
        {
          sprintf(sCommand, "killall %s", sProcName);
          result = system(sCommand);
          printf("Result of kill is %d\n", result);
          if(result > 0) {
            sprintf(sRet, "%sUnable to kill %s\n", sErrorPrefix, sProcName);
            return;
          }
        }
        strcpy(sRet, "Process Killed");
      }

int IsProcessDead(char* sProcName)
        {
        char sCommand[1024];

        sprintf(sCommand, "ps -o state %s", sProcName);

        FILE* inputstream;
        inputstream = popen(sCommand,"r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
          if(temp == 'Z')
          {
            return 0;
          }
        }

        pclose(inputstream);

        return 1;
  }

void GetProcessInfo(char* sRet)
      {
        FILE* inputstream;
        inputstream = popen("ps -a","r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
         strncat(sRet, &temp, 1);
        }
        pclose(inputstream);

        return;
      }

void GetOSInfo(char* sRet)
        {
        FILE* inputstream;
        inputstream = popen("uname -s -m -p -r","r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
         strncat(sRet, &temp, 1);
        }
        pclose(inputstream);
        //Removing newline
        sRet[strlen(sRet)-1]='\0';
        }
/*
char* GetPowerInfo()
        {
        char* sRet = "";

        sRet = "Power status:\n  AC power " + SUTAgentAndroid.sACStatus + "\n";
        sRet += "  Battery charge " + SUTAgentAndroid.sPowerStatus + "\n";
        sRet += "  Remaining charge:      " + SUTAgentAndroid.nChargeLevel + "%\n";
        sRet += "  Battery Temperature:   " + (((float)(SUTAgentAndroid.nBatteryTemp))/10) + " (c)\n";
        return (sRet);
        }

    // todo
char* GetDiskInfo(char* sPath)
        {
        char* sRet = "";
        StatFs statFS = new StatFs(sPath);

        int nBlockCount = statFS.getBlockCount();
        int nBlockSize = statFS.getBlockSize();
        int nBlocksAvail = statFS.getAvailableBlocks();
        int nBlocksFree = statFS.getFreeBlocks();

        sRet = "total:     " + (nBlockCount * nBlockSize) + "\nfree:      " + (nBlocksFree * nBlockSize) + "\navailable: " + (nBlocksAvail * nBlockSize);

        return (sRet);
        }

char* GetMemoryInfo()
        {
        char* sRet = "PA:" + GetMemoryConfig();
        return (sRet);
        }

long GetMemoryConfig()
        {
        ActivityManager aMgr = (ActivityManager) contextWrapper.getSystemService(Activity.ACTIVITY_SERVICE);
        ActivityManager.MemoryInfo outInfo = new ActivityManager.MemoryInfo();
        aMgr.getMemoryInfo(outInfo);
        long lMem = outInfo.availMem;

        return (lMem);
        }

char* UpdateCallBack(char* sFileName)
        {
        char* sRet = sErrorPrefix + "No file specified";
        char* sIP = "";
        char* sPort = "";
        int nEnd = 0;
        int nStart = 0;

        if ((sFileName == null) || (sFileName.length() == 0))
            return(sRet);

        Context ctx = contextWrapper.getApplicationContext();
        try {
            FileInputStream fis = ctx.openFileInput(sFileName);
            int nBytes = fis.available();
            if (nBytes > 0)
                {
                byte [] buffer = new byte [nBytes + 1];
                int nRead = fis.read(buffer, 0, nBytes);
                fis.close();
                ctx.deleteFile(sFileName);
                if (nRead > 0)
                    {
                    char* sBuffer = new char*(buffer);
                    nEnd = sBuffer.indexOf(',');
                    if (nEnd > 0)
                        {
                        sIP = (sBuffer.substring(0, nEnd)).trim();
                        nStart = nEnd + 1;
                        nEnd = sBuffer.indexOf('\r', nStart);
                        if (nEnd > 0)
                            {
                            sPort = (sBuffer.substring(nStart, nEnd)).trim();
                            Thread.sleep(5000);
                            sRet = RegisterTheDevice(sIP, sPort, sBuffer.substring(nEnd + 1));
                            }
                        }
                    }
                }
            }
        catch (FileNotFoundException e)
            {
            sRet = sErrorPrefix + "Nothing to do";
            }
        catch (IOException e)
            {
            sRet = sErrorPrefix + "Couldn't send info to " + sIP + ":" + sPort;
            }
        catch (InterruptedException e)
            {
            e.printStackTrace();
            }
        return(sRet);
        }
*/
void RegisterTheDevice(char* sSrvr, char* sPort, char* sData, int sSize, char* sRet)
        {
        char line[1024];
        printf("sSrvr = %s\n", sSrvr);
        printf("sPort = %s\n", sPort);
        printf("sData = %s\n", sData);
        printf("sSize = %d\n", sSize);
        printf("sRet = %s\n", sRet);
//        Debug.waitForDebugger();

        if (sSrvr != NULL && sPort != NULL && sData != NULL)
            {
                long nPort = atol(sPort);
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in address;
                address.sin_family = AF_INET;
                address.sin_port = htons(nPort);
                if(inet_aton(sSrvr, &address.sin_addr) <= 0){
                  puts("BAD IP!\n");
                }
                fd_set socketSet;
                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                if( connect(sockfd, (struct sockaddr *)&address, sizeof(address))) {
                  strcpy(sRet, "Failure to Connect to RegServer");
                  return;
                }
                FD_ZERO(&socketSet);
                FD_SET(sockfd, &socketSet);
                send(sockfd, sData, sSize, 0);
                while( select(sockfd+1, &socketSet, NULL, NULL, &timeout) == -1)
                    {
                    recv(sockfd, line, 1024, 0) ;

                    if (strlen(line) > 0)
                    {
                          int i;
                          for(i = 0; i < strlen(line); i++)
                          {
                            line[i] = tolower(line[i]);
                          }
                        
                          strcat(sRet, line);
                    }
                    else
                        {
                        puts("GOT NOTHING BACK!\n");
                        // end of stream reached
                        break;
                        }
                    }
                close(sockfd);
                        puts("GOT NOHING BACK!\n");
            }
                        puts("GOT NOTING BACK!\n");
        return;
        }
/*
char* GetInternetData(char* sHost, char* sPort, char* sURL)
        {
        char* sRet = "";
        char* sNewURL = "";
        HttpClient httpClient = new DefaultHttpClient();
        try
            {
            sNewURL = "http://" + sHost + ((sPort.length() > 0) ? (":" + sPort) : "") + sURL;

            HttpGet request = new HttpGet(sNewURL);
            HttpResponse response = httpClient.execute(request);
            int status = response.getStatusLine().getStatusCode();
            // we assume that the response body contains the error message
            if (status != HttpStatus.SC_OK)
                {
                ByteArrayOutputStream ostream = new ByteArrayOutputStream();
                response.getEntity().writeTo(ostream);
                Log.e("HTTP CLIENT", ostream.tochar*());
                }
            else
                {
                InputStream content = response.getEntity().getContent();
                byte [] data = new byte [2048];
                int nRead = content.read(data);
                sRet = new char*(data, 0, nRead);
                content.close(); // this will also close the connection
                }
            }
        catch (IllegalArgumentException e)
            {
            sRet = e.getLocalizedMessage();
            e.printStackTrace();
            }
        catch (ClientProtocolException e)
            {
            sRet = e.getLocalizedMessage();
            e.printStackTrace();
            }
        catch (IOException e)
            {
            sRet = e.getLocalizedMessage();
            e.printStackTrace();
            }

        return(sRet);
        }

char* GetTimeZone()
        {
        char*    sRet = "";
        TimeZone tz;

        tz = TimeZone.getDefault();
        Date now = new Date();
        sRet = tz.getDisplayName(tz.inDaylightTime(now), TimeZone.LONG);

        return(sRet);
        }

char* SetTimeZone(char* sTimeZone)
        {
        char*            sRet = "Unable to set timezone to " + sTimeZone;
        TimeZone         tz = null;
        AlarmManager     amgr = null;

        if ((sTimeZone.length() > 0) && (sTimeZone.startsWith("GMT")))
            {
            amgr = (AlarmManager) contextWrapper.getSystemService(Context.ALARM_SERVICE);
            if (amgr != null)
                amgr.setTimeZone(sTimeZone);
            }
        else
            {
            char* [] zoneNames = TimeZone.getAvailableIDs();
            int nNumMatches = zoneNames.length;
            int    lcv = 0;

            for (lcv = 0; lcv < nNumMatches; lcv++)
                {
                if (zoneNames[lcv].equalsIgnoreCase(sTimeZone))
                    break;
                }

            if (lcv < nNumMatches)
                {
                amgr = (AlarmManager) contextWrapper.getSystemService(Context.ALARM_SERVICE);
                if (amgr != null)
                    amgr.setTimeZone(zoneNames[lcv]);
                }
            }

        if (amgr != null)
            {
            tz = TimeZone.getDefault();
            Date now = new Date();
            sRet = tz.getDisplayName(tz.inDaylightTime(now), TimeZone.LONG);
            }

        return(sRet);
        }
*/
void GetSystemTime(char* sRet)
        {
        FILE* inputstream;
        inputstream = popen("date \"+%Y/%m/%d %H:%M:%S:000\"","r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
         strncat(sRet, &temp, 1);
        }
        pclose(inputstream);

        return;
        }

void SetSystemTime(char* sDate, char* sTime, int outSocket, char* sRet) {
    
    
    
    
    
    
    }

void GetClok(char* sRet)
        {
        long result = time(NULL);
        sprintf(sRet,"%ld", result);
        }

void GetUptime(char* sRet)
        {
        char sCommand[1024];

        sprintf(sCommand,"uptime");

        FILE* inputstream = popen(sCommand, "r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
          //fputc(temp, sRet);
          strncat(sRet, &temp, 1);
        }
        pclose(inputstream);

        return;
      }/*
        char* sRet = "";
        long lHold = 0;
        long lUptime = SystemClock.elapsedRealtime();
        int    nDays = 0;
        int    nHours = 0;
        int nMinutes = 0;
        int nSecs = 0;
        int nMilliseconds = 0;

        if (lUptime > 0)
            {
            nDays = (int)(lUptime / (24L * 60L * 60L * 1000L));
            lHold = lUptime % (24L * 60L * 60L * 1000L);
            nHours = (int)(lHold / (60L * 60L * 1000L));
            lHold %= 60L * 60L * 1000L;
            nMinutes = (int)(lHold / (60L * 1000L));
            lHold %= 60L * 1000L;
            nSecs = (int)(lHold / 1000L);
            nMilliseconds = (int)(lHold % 1000);
            sRet = "" + nDays + " days " + nHours + " hours " + nMinutes + " minutes " + nSecs + " seconds " + nMilliseconds + " ms";
            }

        return (sRet);
        }*/

/*char* NewKillProc(char* sProcId, OutputStream out)
        {
        char* sRet = "";
        char* [] theArgs = new char* [3];

        theArgs[0] = "su";
        theArgs[1] = "-c";
        theArgs[2] = "kill " + sProcId;

        try
            {
            pProc = Runtime.getRuntime().exec(theArgs);
            RedirOutputThread outThrd = new RedirOutputThread(pProc, out);
            outThrd.start();
            outThrd.join(5000);
            }
        catch (IOException e)
            {
            sRet = e.getMessage();
            e.printStackTrace();
            }
        catch (InterruptedException e)
            {
            e.printStackTrace();
            }

        return(sRet);
        }
*/
void SendPing(char* sIPAddr, int outSocket, char* sRet)
      {
        char sCommand[1024];
        pthread_t rOThread;

        sprintf(sCommand,"ping -c 3 %s", sIPAddr);

        FILE* inputstream = popen(sCommand, "r");
        struct ROTArgs args;
        args.outStream = inputstream;
        args.outSocket = outSocket;

        pthread_create(&rOThread, NULL, *RedirOutputThread, &args);

        pthread_join(rOThread, NULL);
        /*char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
          fputc(temp, out);
          //strncat(sRet, &temp, 1);
        }
        pclose(inputstream);
        */


        return;
      }
/*
char* GetTmpDir()
    {
        char*     sRet = "";
        Context ctx = contextWrapper.getApplicationContext();
        File dir = ctx.getFilesDir();
        ctx = null;
        try {
            sRet = dir.getCanonicalPath();
            }
        catch (IOException e)
            {
            e.printStackTrace();
            }
        return(sRet);
    }
*/
void PrintFileTimestamp(char* sFile, char* sRet)
      {
        char* sTmpDst = malloc(1024);
        strcpy(sRet, "Last modified: ");
        fixFileName(sFile, sTmpDst);
        char sCommand[1024];
        sprintf(sCommand, "stat -f %%Sm -t \"%%Y/%%m/%%d %%H:%%M:%%S:000\" %s", sTmpDst);
        FILE* inputstream;
        inputstream = popen(sCommand,"r");
        char temp;
        while( (temp = fgetc(inputstream)) != EOF && temp != '\n')
        {
         strncat(sRet, &temp, 1);
        }
        pclose(inputstream);
        free(sTmpDst);
      }

void GetIniData(char* sSection, char* sKey, char* sFile, char* sRet)
        {
        char sComp[1024];
        char sLine[1024];
        int bFound = 0;
        FILE* in;
        char* sTmpFileName = malloc(1024); 
        fixFileName(sFile, sTmpFileName);

        in = fopen(sTmpFileName, "r");
        sprintf(sComp, "[%s]", sSection);
        while (fgets(sLine, 1024, in) != NULL)
            {
            if (strstr(sLine,sComp) != NULL)
                {
                bFound = 1;
                break;
                }
            }

        if (bFound)
            {
            sprintf(sComp, "%s =", sKey);
            while (fgets(sLine, 1024, in) != NULL)
                {
                if (strstr(sLine, sComp) != NULL)
                    {
                      char* equals = strchr(sLine, '=');
                      if (equals != NULL)
                        {
                          strcpy(sRet, equals+2);
                          sRet[strlen(sRet)-1]='\0';
                        }
                    break;
                    }
                }
            }
        fclose(in);
        free(sTmpFileName);
        return;
        }

void RunReboot(int outSocket, char* sCallBackIP, char* sCallBackPort, char* sRet)
        {
        char sCommand[1024];
        strcpy(sCommand, "sudo reboot");

        if ((sCallBackIP != NULL) && (sCallBackPort != NULL) &&
            (strlen(sCallBackIP) > 0) && (strlen(sCallBackPort) > 0))    {
            FILE* fos = fopen("update.info", "w+");
            char sBuffer[1024];
            sprintf(sBuffer,"%s,%s,\rSystem rebooted\r", sCallBackIP, sCallBackPort);
            fputs(sBuffer, fos);
            fflush(fos);
            fclose(fos);
        }

            // Tell all of the data channels we are rebooting
            //((ASMozStub)this.contextWrapper).SendToDataChannel("Rebooting ...");

          system(sCommand);
        
          strcpy(sRet, "You really got here???");
        }
/*
char* UnInstallApp(char* sApp, OutputStream out)
        {
        char* sRet = "";
        char* [] theArgs = new char* [3];

        theArgs[0] = "su";
        theArgs[1] = "-c";
        theArgs[2] = "pm uninstall " + sApp + ";reboot;exit";

        try
            {
            pProc = Runtime.getRuntime().exec(theArgs);

            RedirOutputThread outThrd = new RedirOutputThread(pProc, out);
            outThrd.start();
            outThrd.join(60000);
            int nRet = pProc.exitValue();
            sRet = "\nuninst complete [" + nRet + "]";
            }
        catch (IOException e)
            {
            sRet = e.getMessage();
            e.printStackTrace();
            }
        catch (InterruptedException e)
            {
            e.printStackTrace();
            }

        return (sRet);
    }

char* InstallApp(char* sApp, OutputStream out)
        {
        char* sRet = "";
        char* [] theArgs = new char* [3];
        File    srcFile = new File(sApp);

        theArgs[0] = "su";
        theArgs[1] = "-c";
        theArgs[2] = "mv " + GetTmpDir() + "/" + srcFile.getName() + " /data/local/tmp/" + srcFile.getName() + ";exit";

        sRet = CopyFile(sApp, GetTmpDir() + "/" + srcFile.getName());
        try {
            out.write(sRet.getBytes());
            out.flush();
            }
        catch (IOException e1)
            {
            e1.printStackTrace();
            }

        try
            {
            pProc = Runtime.getRuntime().exec(theArgs);

            RedirOutputThread outThrd = new RedirOutputThread(pProc, out);
            outThrd.start();
            outThrd.join(90000);
            int nRet = pProc.exitValue();
            sRet = "\nmove complete [" + nRet + "]";
            try
                {
                out.write(sRet.getBytes());
                out.flush();
                }
            catch (IOException e1)
                {
                e1.printStackTrace();
                }

            theArgs[2] = "chmod 666 /data/local/tmp/" + srcFile.getName() + ";exit";
            pProc = Runtime.getRuntime().exec(theArgs);
            RedirOutputThread outThrd2 = new RedirOutputThread(pProc, out);
            outThrd2.start();
            outThrd2.join(10000);
            int nRet2 = pProc.exitValue();
            sRet = "\npermission change complete [" + nRet2 + "]\n";
            try {
                out.write(sRet.getBytes());
                out.flush();
                }
            catch (IOException e1)
                {
                e1.printStackTrace();
                }

            theArgs[2] = "pm install -r /data/local/tmp/" + srcFile.getName() + " Cleanup" + ";exit";
            pProc = Runtime.getRuntime().exec(theArgs);
            RedirOutputThread outThrd3 = new RedirOutputThread(pProc, out);
            outThrd3.start();
            outThrd3.join(60000);
            int nRet3 = pProc.exitValue();
            sRet = "\ninstallation complete [" + nRet3 + "]";
            try {
                out.write(sRet.getBytes());
                out.flush();
                }
            catch (IOException e1)
                {
                e1.printStackTrace();
                }

            theArgs[2] = "rm /data/local/tmp/" + srcFile.getName() + ";exit";
            pProc = Runtime.getRuntime().exec(theArgs);
            RedirOutputThread outThrd4 = new RedirOutputThread(pProc, out);
            outThrd4.start();
            outThrd4.join(60000);
            int nRet4 = pProc.exitValue();
            sRet = "\ntmp file removed [" + nRet4 + "]";
            try {
                out.write(sRet.getBytes());
                out.flush();
                }
            catch (IOException e1)
                {
                e1.printStackTrace();
                }
            sRet = "\nSuccess";
            }
        catch (IOException e)
            {
            sRet = e.getMessage();
            e.printStackTrace();
            }
        catch (InterruptedException e)
            {
            e.printStackTrace();
            }

        return (sRet);
        }

char* StrtUpdtOMatic(char* sPkgName, char* sPkgFileName, char* sCallBackIP, char* sCallBackPort)
        {
        char* sRet = "";

        Context ctx = contextWrapper.getApplicationContext();
        PackageManager pm = ctx.getPackageManager();

        Intent prgIntent = new Intent();
        prgIntent.setPackage("com.mozilla.watcher");

        try {
            PackageInfo pi = pm.getPackageInfo("com.mozilla.watcher", PackageManager.GET_SERVICES | PackageManager.GET_INTENT_FILTERS);
            ServiceInfo [] si = pi.services;
            for (int i = 0; i < si.length; i++)
                {
                ServiceInfo s = si[i];
                if (s.name.length() > 0)
                    {
                    prgIntent.setClassName(s.packageName, s.name);
                    break;
                    }
                }
            }
        catch (NameNotFoundException e)
            {
            e.printStackTrace();
            sRet = sErrorPrefix + "watcher is not properly installed";
            return(sRet);
            }

        prgIntent.putExtra("command", "updt");
        prgIntent.putExtra("pkgName", sPkgName);
        prgIntent.putExtra("pkgFile", sPkgFileName);
        prgIntent.putExtra("reboot", true);

        try
            {
            if ((sCallBackIP != null) && (sCallBackPort != null) &&
                (sCallBackIP.length() > 0) && (sCallBackPort.length() > 0))
                {
                FileOutputStream fos = ctx.openFileOutput("update.info", Context.MODE_WORLD_READABLE | Context.MODE_WORLD_WRITEABLE);
                char* sBuffer = sCallBackIP + "," + sCallBackPort + "\rupdate started " + sPkgName + " " + sPkgFileName + "\r";
                fos.write(sBuffer.getBytes());
                fos.flush();
                fos.close();
                fos = null;
                prgIntent.putExtra("outFile", ctx.getFilesDir() + "/update.info");
                }
            else {
                if (prgIntent.hasExtra("outFile")) {
                    System.out.println("outFile extra unset from intent");
                    prgIntent.removeExtra("outFile");
                }
            }

            ComponentName cn = contextWrapper.startService(prgIntent);
            if (cn != null)
                sRet = "exit";
            else
                sRet = sErrorPrefix + "Unable to use watcher service";
            }
        catch(ActivityNotFoundException anf)
            {
            sRet = sErrorPrefix + "Activity Not Found Exception [updt] call failed";
            anf.printStackTrace();
            }
        catch (FileNotFoundException e)
            {
            sRet = sErrorPrefix + "File creation error [updt] call failed";
            e.printStackTrace();
            }
        catch (IOException e)
            {
            sRet = sErrorPrefix + "File error [updt] call failed";
            e.printStackTrace();
            }

        ctx = null;

        return (sRet);
        }

char* StartJavaPrg(char* [] sArgs, Intent preIntent)
        {
        char* sRet = "";
        char* sArgList = "";
        char* sUrl = "";
//        char* sRedirFileName = "";
        Intent prgIntent = null;

        Context ctx = contextWrapper.getApplicationContext();
        PackageManager pm = ctx.getPackageManager();

        if (preIntent == null)
            prgIntent = new Intent();
        else
            prgIntent = preIntent;

        prgIntent.setPackage(sArgs[0]);
        prgIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        try {
            PackageInfo pi = pm.getPackageInfo(sArgs[0], PackageManager.GET_ACTIVITIES | PackageManager.GET_INTENT_FILTERS);
            ActivityInfo [] ai = pi.activities;
            for (int i = 0; i < ai.length; i++)
                {
                ActivityInfo a = ai[i];
                if (a.name.length() > 0)
                    {
                    prgIntent.setClassName(a.packageName, a.name);
                    break;
                    }
                }
            }
        catch (NameNotFoundException e)
            {
            e.printStackTrace();
            }

        if (sArgs.length > 1)
            {
            if (sArgs[0].contains("android.browser"))
                prgIntent.setAction(Intent.ACTION_VIEW);
            else
                prgIntent.setAction(Intent.ACTION_MAIN);

            if (sArgs[0].contains("fennec"))
                {
                sArgList = "";
                sUrl = "";

                for (int lcv = 1; lcv < sArgs.length; lcv++)
                    {
                    if (sArgs[lcv].contains("://"))
                        {
                        prgIntent.setAction(Intent.ACTION_VIEW);
                        sUrl = sArgs[lcv];
                        }
                    else
                        {
                        if (sArgs[lcv].equals(">"))
                            {
                            lcv++;
                            if (lcv < sArgs.length)
                                lcv++;
//                                sRedirFileName = sArgs[lcv++];
                            }
                        else
                            sArgList += " " + sArgs[lcv];
                        }
                    }

                if (sArgList.length() > 0)
                    prgIntent.putExtra("args", sArgList.trim());

                if (sUrl.length() > 0)
                    prgIntent.setData(Uri.parse(sUrl.trim()));
                }
            else
                {
                for (int lcv = 1; lcv < sArgs.length; lcv++)
                    sArgList += " " + sArgs[lcv];

                prgIntent.setData(Uri.parse(sArgList.trim()));
                }
            }
        else
            {
            prgIntent.setAction(Intent.ACTION_MAIN);
            }

        try
            {
            contextWrapper.startActivity(prgIntent);
            }
        catch(ActivityNotFoundException anf)
            {
            anf.printStackTrace();
            }

        ctx = null;
        return (sRet);
        }
*/
void StartPrg(char** progArray, int progLen, int outSocket, char* sRet)
      {
        char sCommand[1024] = "";
        int i;
        pthread_t rOThread;
        for(i = 0; i < progLen; i++)
        {
          strcat( sCommand, progArray[i] );
          strcat( sCommand, " ");
        }
        strcat( sCommand, " 2>&1");

        FILE* inputstream = popen(sCommand, "r");
        /*char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
          fputc(temp, out);
          //strncat(sRet, &temp, 1);
        }*/
        struct ROTArgs args;
        args.outStream = inputstream;
        args.outSocket = outSocket;

        pthread_create(&rOThread, NULL, *RedirOutputThread, &args);

        pthread_join(rOThread, NULL);

        int nRetCode = pclose(inputstream);
        sprintf(sRet, "return code [%d]", nRetCode);
      }

void StartPrg2(char** progArray, int progLen, int outSocket, char* sRet)
      {
        int i;
        pthread_t rOThread;
        char sCommand[1024];
        char sEnvString[1024];
        strcpy(sEnvString, progArray[0]);

        if (strchr(sEnvString,'=') == NULL && strlen(sEnvString) > 0)
        {
          StartPrg(progArray, progLen, outSocket, sRet);
          return;
        }

        replace_str(sEnvString, ",", " ");
        strcpy(sCommand, sEnvString);
        strcat(sCommand, " ");
        for( i = 1; i < progLen; i++)
        {
          strcat(sCommand, progArray[i]);
          strcat(sCommand, " ");
        }
        strcat( sCommand, " 2>&1");
        FILE* inputstream = popen(sCommand, "r");
        /*char temp;
        while( (temp = fgetc(inputstream)) != EOF)
        {
          fputc(temp, out);
          //strncat(sRet, &temp, 1);
        }
        fflush(out);*/
        struct ROTArgs args;
        args.outStream = inputstream;
        args.outSocket = outSocket;

        pthread_create(&rOThread, NULL, *RedirOutputThread, &args);

        pthread_join(rOThread, NULL);
        int nRetCode = pclose(inputstream);
        sprintf(sRet, "return code [%d]", nRetCode);
      }

void PrintUsage(char* sRet)
{
  strcat(sRet, "run [cmdline]                - start program no wait\n");
  strcat(sRet, "exec [env pairs] [cmdline]   - start program no wait optionally pass env\n");
  strcat(sRet, "                               key=value pairs (comma separated)\n");
  strcat(sRet, "kill [program name]          - kill program no path\n");
  strcat(sRet, "killall                     *- kill all processes started\n");
  strcat(sRet, "ps                           - list of running processes\n");
  strcat(sRet, "info                         - list of device info\n");
  strcat(sRet, "        [os]                 - os version for device\n");
  strcat(sRet, "        [id]                *- unique identifier for device\n");
  strcat(sRet, "        [uptime]             - uptime for device\n");
  strcat(sRet, "        [systime]            - current system time\n");
  strcat(sRet, "        [screen]            *- width, height and bits per pixel for device\n");
  strcat(sRet, "        [memory]            *- physical, free, available, storage memory\n");
  strcat(sRet, "                               for device\n");
  strcat(sRet, "        [processes]          - list of running processes see 'ps'\n");
  strcat(sRet, "deadman timeout             *- set the duration for the deadman timer\n");
  strcat(sRet, "alrt [on/off]               *- start or stop sysalert behavior\n");
  strcat(sRet, "disk [arg]                  *- prints disk space info\n");
  strcat(sRet, "cp file1 file2               - copy file1 to file2\n");
  strcat(sRet, "time file                    - timestamp for file\n");
  strcat(sRet, "hash file                    - generate hash for file\n");
  strcat(sRet, "cd directory                 - change cwd\n");
  strcat(sRet, "cat file                     - cat file\n");
  strcat(sRet, "cwd                          - display cwd\n");
  strcat(sRet, "mv file1 file2               - move file1 to file2\n");
  strcat(sRet, "push filename                - push file to device\n");
  strcat(sRet, "rm file                      - delete file\n");
  strcat(sRet, "rmdr directory               - delete directory even if not empty\n");
  strcat(sRet, "mkdr directory               - create directory\n");
  strcat(sRet, "dirw directory               - tests whether the directory is writable\n");
  strcat(sRet, "isdir directory              - test whether the directory exists\n");
  strcat(sRet, "stat processid              *- stat process\n");
  strcat(sRet, "dead processid               - print whether the process is alive or hung\n");
  strcat(sRet, "mems                        *- dump memory stats\n");
  strcat(sRet, "ls                           - print directory\n");
  strcat(sRet, "tmpd                        *- print temp directory\n");
  strcat(sRet, "ping [hostname/ipaddr]       - ping a network device\n");
  strcat(sRet, "unzp zipfile destdir         - unzip the zipfile into the destination dir\n");
  strcat(sRet, "zip zipfile src              - zip the source file/dir into zipfile\n");
  strcat(sRet, "rebt                         - reboot device\n");
  strcat(sRet, "inst /path/filename.apk     *- install the referenced apk file\n");
  strcat(sRet, "uninst packagename          *- uninstall the referenced package\n");
  strcat(sRet, "updt pkgname pkgfile        *- unpdate the referenced package\n");
  strcat(sRet, "clok                         - the current device time expressed as the");
  strcat(sRet, "                               number of millisecs since epoch\n");
  strcat(sRet, "settime date time            - sets the device date and time\n");
  strcat(sRet, "                               (YYYY/MM/DD HH:MM:SS)\n");
  strcat(sRet, "tzset timezone              *- sets the device timezone format is\n");
  strcat(sRet, "                               GMTxhh:mm x = +/- or a recognized Olsen string\n");
  strcat(sRet, "tzget                       *- returns the current timezone set on the device\n");
  strcat(sRet, "rebt                         - reboot device\n");
  strcat(sRet, "quit                         - disconnect SUTAgent\n");
  strcat(sRet, "exit                         - close SUTAgent\n");
  strcat(sRet, "ver                          - SUTAgent version\n");
  strcat(sRet, "help                         - you're reading it");
  strcat(sRet, "  *- indicates not available ^");
}
/*
int main(int argc, char *argv[])
{
  char commandstr[1024] = "";
  int i = 0;
  for(i = 1; i < argc;i++) {
    //printf("\n%s", argv[i]);
    strcat(commandstr, argv[i]);
    strcat(commandstr," ");
  }
  //puts(commandstr);
  //puts("\n");
  puts((processCommand(commandstr, NULL,stdout)));
}*/
