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

#ifndef DOCOMMAND_H
#define DOCOMMAND_H

enum Command
    {
    RUN,
    EXEC,
    ENVRUN,
    KILL,
    PS,
    DEVINFO,
    OS,
    ID,
    UPTIME,
    SETTIME,
    SYSTIME,
    SCREEN,
    MEMORY,
    POWER,
    PROCESS,
    GETAPPROOT,
    TESTROOT,
    ALRT,
    DISK,
    CP,
    TIME,
    HASH,
    CD,
    CAT,
    CWD,
    MV,
    PUSH,
    PULL,
    RM,
    PRUNE,
    MKDR,
    DIRWRITABLE,
    ISDIR,
    DEAD,
    MEMS,
    LS,
    TMPD,
    PING,
    REBT,
    UNZP,
    ZIP,
    CLOK,
    STAT,
    QUIT,
    EXIT,
    HELP,
    FTPG,
    FTPP,
    INST,
    UPDT,
    UNINST,
    TEST,
    DBG,
    TRACE,
    VER,
    TZGET,
    TZSET,
    ADB,
    UNKNOWN
};

static char* commands[59] = { 
    "run",
    "exec",
    "envrun",
    "kill",
    "ps",
    "info",
    "os",
    "id",
    "uptime",
    "settime",
    "systime",
    "screen",
    "memory",
    "power",
    "process",
    "getapproot",
    "testroot",
    "alrt",
    "disk",
    "cp",
    "time",
    "hash",
    "cd",
    "cat",
    "cwd",
    "mv",
    "push",
    "pull",
    "rm",
    "rmdr",
    "mkdr",
    "dirw",
    "isdir",
    "dead",
    "mems",
    "ls",
    "tmpd",
    "ping",
    "rebt",
    "unzp",
    "zip",
    "clok",
    "stat",
    "quit",
    "exit",
    "help",
    "ftpg",
    "ftpp",
    "inst",
    "updt",
    "uninst",
    "test",
    "dbg",
    "trace",
    "ver",
    "tzget",
    "tzset",
    "adb",
    "unknown"
};

int getCmd(char* sCmd);

void processCommand(char* theCmdLine, int socket, char* strReturn);

void SendNotification(char* tickerText, char* expandedText);

void CancelNotification();

void StartAlert(char* sTitle, char* sMsg);

void StopAlert();

char** parseCmdLine2(char* theCmdLine, int* argc);

void fixFileName(char* fileName, char* result);

void  Zip(char* zipFileName, char* srcName, char* sRet);

void  Unzip(char* zipFileName, char* dstDirectory, char* sRet);

void StatProcess(char* string, char* sRet);

void  GetTestRoot(char* sRet);

void  GetAppRoot(char* AppName, char* sRet);

void  isDirectory(char* sDir, char* sRet);

void changeDir(char* newDir, char* sRet);

void getHex( char* raw, char* sRet );

void HashFile(char* fileName, char* sRet);

void RemoveFile(char* fileName, char* sRet);

void PruneDir(char* sDir, char* sRet);

void PrintDir(char* sDir, char* sRet);

void Move(char* sTmpSrcFileName, char* sTmpDstFileName, char* sRet);

void CopyFile(char* sTmpSrcFileName, char* sTmpDstFileName, char* sRet);

void IsDirWritable(char* sDir, char* sRet);

void Push(char* fileName, int socket, long lSize, char* sRet);

void FTPGetFile(char* sServer, char* sSrcFileName, char* sDstFileName, int outSocket, char* sRet);
    
void Pull(char* fileName, long lOffset, long lLength, int outSocket, char* sRet);

void Cat(char* fileName, int outSocket, char* sRet);

void MakeDir(char* sDir, char* sRet);
    
void GetScreenInfo(char* sRet);
    
int*  GetScreenXY();
/*    
char* SetADB(char* sWhat);
*/
void KillProcess(char* sProcName, int outSocket, char* sRet);

int IsProcessDead(char* sProcName);

void  GetProcessInfo(char* sRet);

void GetOSInfo(char* sRet);
/*
char* GetDiskInfo(char* sPath);

char* GetMemoryInfo();

long GetMemoryConfig();

char* UpdateCallBack(char* sFileName);
*/
void RegisterTheDevice(char* sSrvr, char* sPort, char* sData, int sSize, char* sRet);
/*
char* GetInternetData(char* sHost, char* sPort, char* sURL);

char* GetTimeZone();

char* SetTimeZone(char* sTimeZone);
*/
void GetSystemTime(char* sRet);

void SetSystemTime(char* sDate, char* sTime, int outSocket, char* sRet);

void GetClok(char* sRet);

void GetUptime(char* sRet);
/*
char* NewKillProc(char* sProcId, ostream out);
*/
void SendPing(char* sIPAddr, int outSocket, char* sRet);
/*
char* GetTmpDir();
*/
void PrintFileTimestamp(char* sFile, char* sRet);

void GetIniData(char* sSection, char* sKey, char* sFile, char* sRet);

void RunReboot(int outSocket, char* sCallBackIP, char* sCallBackPort, char* sRet);
/*
char* UnInstallApp(char* sApp, ostream out);

char* InstallApp(char* sApp, ostream out);

char* StrtUpdtOMatic(char* sPkgName, char* sPkgFileName, char* sCallBackIP, char* sCallBackPort);

char* StartJavaPrg(char* [] sArgs, Intent preIntent);
*/
void StartPrg(char** progArray, int progLen, int outSocket, char* sRet);

void StartPrg2(char** progArray, int progLen, int outSocket, char* sRet);

void PrintUsage(char* sRet);

#endif
