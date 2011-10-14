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

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include "RedirOutputThread.h"

void RedirOutputThread(void* threadArgs)
{
        FILE* outStream = ((struct ROTArgs*)threadArgs)->outStream;
        int outSocket = ((struct ROTArgs*)threadArgs)->outSocket;
        int bStillRunning = 1;
        int nBytesRead = 0;
        long lBytesSent = 0;
        char buffer[1024];
        printf("Entered!\n");
        printf("FILE* = %p, outSocket = %d\n", outStream, outSocket);

        while (bStillRunning)
            {
                if( !feof(outStream))// && tChar != '\0')
                    {

                      nBytesRead = fread(buffer, 1, 1024, outStream);
                      //String sRep = new String(buffer,0,nBytesRead).replace("\n", "\r\n");
                      if (outSocket != 0)
                          {
                            int result = send(outSocket, buffer, nBytesRead, 0);
                            if(result == -1)
                            {
                              printf("Sending Failed!");
                              bStillRunning = 0;
                            }
                            else {
                              lBytesSent += result;
                            }
                          //pOut.print(sRep);
                          //pOut.flush();
                          }
                      strcpy(buffer, "");
                    }
                else{
                 bStillRunning = 0;
                }

                //bStillRunning = (IsProcRunning(pProc) || (sutOut.available() > 0) || (sutErr.available() > 0));
            }

        //pProc.destroy();
        //System.gc();
        fclose(outStream);
}
/*
    private boolean IsProcRunning(Process pProc)
        {
        boolean bRet = false;

        try
            {
            nExitCode = pProc.exitValue();
            }
        catch (IllegalThreadStateException z)
            {
            nExitCode = -1;
            bRet = true;
            }

        return(bRet);
        }
    }*/
