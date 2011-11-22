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

#define FILESDIR "/"
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>
#include "doCommand.h"
#include "SUTAgent.h"
#include "RunCmdThread.h"
#include "RunDataThread.h"

    /*public static final int START_PRG = 1959;
    MenuItem mExitMenuItem;
    Timer timer = null;

    public boolean onCreateOptionsMenu(Menu menu)
        {
        mExitMenuItem = menu.add("Exit");
        mExitMenuItem.setIcon(android.R.drawable.ic_menu_close_clear_cancel);
        return super.onCreateOptionsMenu(menu);
        }

    public boolean onMenuItemSelected(int featureId, MenuItem item)
        {
        if (item == mExitMenuItem)
            {
            finish();
            }
        return super.onMenuItemSelected(featureId, item);
        }

    public static String getRegSvrIPAddr()
        {
        return(RegSvrIPAddr);
        }
*/

// NEXT THREE URL DECODING FUNCTIONS TAKEN FROM:
// http://www.geekhideout.com/urlcode.shtml

 
/* Converts a hex character to its integer value */
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
      return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
    char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
    while (*pstr) {
      if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~' || *pstr == '=' || *pstr == '&') 
        *pbuf++ = *pstr;
      else if (*pstr == ' ') 
        *pbuf++ = '+';
      else 
        *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
      pstr++;
    }
    *pbuf = '\0';
    return buf;
}
    void getMAC(char* sRet)
      {/*
        ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];
        if (getifaddrs(&ifaddr) == -1) {
          perror("getifaddrs");
        }
        / * Walk through linked list, maintaining head pointer so we
        can free list later * /
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
          if (ifa->ifa_addr == NULL)
            continue;
          family = ifa->ifa_addr->sa_family;
          if(family == AF_INET || AF_INET6) {
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET)?sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6),
             host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
             if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                 return NULL;

          }
          / * Display interface name and family (including symbolic
          form of the latter for the common families) * /
          printf("%s  address family: %d%s\n",
          ifa->ifa_name, family,
          (family == AF_PACKET) ? " (AF_PACKET)" :
          (family == AF_INET) ?   " (AF_INET)" :
          (family == AF_INET6) ?  " (AF_INET6)" : "");
          / * For an AF_INET* interface address, display the address * /
          if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
            (family == AF_INET) ? sizeof(struct sockaddr_in) :
            sizeof(struct sockaddr_in6),
            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
              printf("getnameinfo() failed: %s\n", gai_strerror(s));
              exit(EXIT_FAILURE);
            }
            printf("\taddress: <%s>\n", host);
          }
        }
        freeifaddrs(ifaddr);
        exit(EXIT_SUCCESS);*/
        strcpy(sRet, "DE:AD:DE:AD");
      }


    /** Called when the activity is first created. */
    int main(int argc, char** argv)
        {
        char sIniFile[1024] = "";
        //setContentView(R.layout.main);

        //fixScreenOrientation();
        if(argc > 1)
        {
          strcpy(sIniFile, argv[1]);
        } 
        else {
	  strcat(sIniFile, "/SUTAgent.ini");
        }


        int CmdSockFD = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in cmdAddress;
        cmdAddress.sin_family = AF_INET;
        cmdAddress.sin_port = htons(20701);
        cmdAddress.sin_addr.s_addr = INADDR_ANY;
        pthread_t cmdThread;
        if( bind(CmdSockFD, (struct sockaddr *)&cmdAddress, sizeof(cmdAddress))==0) 
        {
          pthread_create(&cmdThread, NULL, *RunCmdThread, CmdSockFD);
        }
        else {
          puts("Cmd Port currently in use.\n");
          return 1;
        }

        int DataSockFD = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in dataAddress;
        dataAddress.sin_family = AF_INET;
        dataAddress.sin_port = htons(20700);
        dataAddress.sin_addr.s_addr = INADDR_ANY;
        pthread_t dataThread;
        if( bind(DataSockFD, (struct sockaddr *)&dataAddress, sizeof(dataAddress))==0) 
        {
          pthread_create(&dataThread, NULL, *RunDataThread, DataSockFD);
        }
        else {
          puts("Data Port currently in use.\n");
          return 1;
        }
        // Get configuration settings from "ini" file
        //FILE dir = getFilesDir();
        //File iniFile = new File(dir, "SUTAgent.ini");
        //getcwd(sIniFile, 1024);
        printf("sIniFile = %s\n", sIniFile);

        GetIniData("Registration Server", "IPAddr", sIniFile, RegSvrIPAddr) ;
        GetIniData("Registration Server", "PORT", sIniFile, RegSvrIPPort );
        GetIniData("Registration Server", "HARDWARE", sIniFile, HardwareID);
        GetIniData("Registration Server", "POOL", sIniFile, Pool );
        GetIniData("NTP Server", "IPAddr", sIniFile, sNTPServer );

        //tv = (TextView) this.findViewById(R.id.Textview01);

        //if (getLocalIpAddress() == NULL)
        //    setUpNetwork(sIniFile);

        //WifiInfo wifi;
        //WifiManager wifiMan = (WifiManager)getSystemService(Context.WIFI_SERVICE);
        //String macAddress = "Unknown";
        /*if (wifiMan != null)
            {
            wifi = wifiMan.getConnectionInfo();
            if (wifi != null)
                {
                macAddress = wifi.getMacAddress();
                if (macAddress != null)
                    sUniqueID = macAddress;
                }
            }
*/
        /*struct ifreq ifr; 
        ioctl(sock, SIOCGIFHWADDR, &ifr);
        unsigned char mac_address[6];
        memcpy(mac_address, ifr.ifr_addr.sa_data, 6);
        printf("MAC = %s", mac_address);
        */
        char mac[18];
        char IP[1024];
        getLocalIpAddress(IP, mac);
        // strcpy(mac, "DE:AD:DE:AD");
        //strcpy(IP, "127.0.0.1");
        printf("IP = %s\n",IP);
        printf("MAC = %s\n", mac);
        strcpy(uniqueID, mac);

        /*Toast.makeText(getApplication().getApplicationContext(), "SUTAgent [" + sLocalIPAddr + "] ...", Toast.LENGTH_LONG).show();
*/
        
        char sConfig[1024]="";
        char osInfo[1024]="";
        sprintf(sConfig, "Unique ID: %s%s", mac, lineSep);
        sprintf(sConfig, "%sOS Info%s", sConfig, lineSep);
        GetOSInfo(osInfo);
        sprintf(sConfig, "%s\t%s%s",sConfig, osInfo, lineSep);
        sprintf(sConfig, "%sScreen Info%s",sConfig, lineSep);
        // int [] xy = dc.GetScreenXY();
        sprintf(sConfig, "%s\t Width: 0%s",sConfig, lineSep);
        sprintf(sConfig, "%s\t Height: 0%s",sConfig, lineSep);
        sprintf(sConfig, "%sMemory Info:%s",sConfig, lineSep);
        sprintf(sConfig, "%s\t 0MemoryInfo%s",sConfig, lineSep);
        sprintf(sConfig, "%sNetwork Info: 0%s",sConfig, lineSep);
        sprintf(sConfig, "%s\tMac Address: %s%s",sConfig, mac, lineSep);
        sprintf(sConfig, "%s\tIP Address: %s%s",sConfig, IP, lineSep);

        printf("%s",sConfig);
        //displayStatus(sConfig);

        sprintf(sRegString, "NAME=%s", mac);
        sprintf(sRegString,"%s&IPADDR=%s",sRegString, IP);
        sprintf(sRegString,"%s&CMDPORT=%d",sRegString, 20701);
        sprintf(sRegString,"%s&DATAPORT=%d",sRegString,20700);
        sprintf(sRegString,"%s&OS=%s",sRegString,osInfo);
        sprintf(sRegString,"%s&SCRNWIDTH=0",sRegString);
        sprintf(sRegString,"%s&SCRNHEIGHT=0",sRegString);
        sprintf(sRegString,"%s&BPP=8",sRegString);
        sprintf(sRegString,"%s&MEMORY=0",sRegString);
        sprintf(sRegString,"%s&HARDWARE=%s",sRegString,HardwareID);
        sprintf(sRegString,"%s&POOL=%s",sRegString,Pool);

        char* buf = url_encode(sRegString);
        strcpy(sRegString, buf);
        free(buf);
        //String sTemp = Uri.encode(sRegString,"=&");
        char temp[1024] = "";
        strcat(temp, "register ");
        strcat(temp, sRegString);
        strcpy(sRegString, temp);
        //sprintf(sRegString,"register %s", sRegString);

        //!bNetworkingStarted)
        
            //Thread thread = new Thread(null, doStartService, "StartServiceBkgnd");
            //thread.start();
            //bNetworkingStarted = true;

//            Thread thread2 = new Thread(null, doRegisterDevice, "RegisterDeviceBkgnd");
        pthread_t thread2;
        pthread_create(&thread2, NULL, doRegisterDevice, NULL);
        pthread_join(thread2, NULL);
            //thread2.start();
       

        pthread_join(cmdThread, NULL);
        puts("Exiting....");
        DataAllStopListening();
        pthread_join(dataThread, NULL);

        //monitorBatteryState();

        // If we are returning from an update let'em know we're back
        //Thread thread3 = new Thread(null, doUpdateCallback, "UpdateCallbackBkgnd");
        //thread3.start();

        //if (strlen(SUTAgentC.sNTPServer) > 0) {
            //Thread thread4 = new Thread(null, doSetClock, "SetClockBkgrnd");
            //thread4.start();
        //}

        //final Button goButton = (Button) findViewById(R.id.Button01);
        //goButton.setOnClickListener(new OnClickListener() {
        //    public void onClick(View v) {
        //        finish();
        //        }
        //    });
        }
/*
    private class UpdateStatus implements Runnable {
        public String sText = "";

        UpdateStatus(String sStatus) {
            sText = sStatus;
        }

        public void run() {
            displayStatus(sText);
        }
    }

    public synchronized void displayStatus(String sStatus) {
        String sTVText = (String) tv.getText();
        sTVText += sStatus;
        tv.setText(sTVText);
    }

    public void fixScreenOrientation()
        {
        setRequestedOrientation((getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) ?
                                ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        }

    protected void onActivityResult(int requestCode, int resultCode, Intent data)
        {
        if (requestCode == START_PRG)
            {
               Toast.makeText(getApplication().getApplicationContext(), "SUTAgent startprg finished ...", Toast.LENGTH_LONG).show();
            }
        }

    @Override
    public void onDestroy()
        {
        super.onDestroy();
        if (isFinishing())
            {
            Intent listenerSvc = new Intent(this, ASMozStub.class);
            listenerSvc.setAction("com.mozilla.SUTAgentC.service.LISTENER_SERVICE");
            stopService(listenerSvc);
            bNetworkingStarted = false;

            unregisterReceiver(battReceiver);

            if (wl != null)
                wl.release();

            System.exit(0);
            }
        }

    private void monitorBatteryState()
        {
        battReceiver = new BroadcastReceiver()
            {
            public void onReceive(Context context, Intent intent)
                {
                StringBuilder sb = new StringBuilder();

                int rawlevel = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1); // charge level from 0 to scale inclusive
                int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, -1); // Max value for charge level
                int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
                int health = intent.getIntExtra(BatteryManager.EXTRA_HEALTH, -1);
                boolean present = intent.getBooleanExtra(BatteryManager.EXTRA_PRESENT, false);
                int plugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1); //0 if the device is not plugged in; 1 if plugged into an AC power adapter; 2 if plugged in via USB.
//                int voltage = intent.getIntExtra(BatteryManager.EXTRA_VOLTAGE, -1); // voltage in millivolts
                nBatteryTemp = intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1); // current battery temperature in tenths of a degree Centigrade
//                String technology = intent.getStringExtra(BatteryManager.EXTRA_TECHNOLOGY);

                nChargeLevel = -1;  // percentage, or -1 for unknown
                if (rawlevel >= 0 && scale > 0)
                    {
                    nChargeLevel = (rawlevel * 100) / scale;
                    }

                if (plugged > 0)
                    sACStatus = "ONLINE";
                else
                    sACStatus = "OFFLINE";

                if (present == false)
                    sb.append("NO BATTERY");
                else
                    {
                    if (nChargeLevel < 10)
                        sb.append("Critical");
                    else if (nChargeLevel < 33)
                        sb.append("LOW");
                    else if (nChargeLevel > 80)
                        sb.append("HIGH");
                    }

                if (BatteryManager.BATTERY_HEALTH_OVERHEAT == health)
                    {
                    sb.append("Overheated ");
                    sb.append((((float)(nBatteryTemp))/10));
                    sb.append("(C)");
                    }
                else
                    {
                    switch(status)
                        {
                        case BatteryManager.BATTERY_STATUS_UNKNOWN:
                            // old emulator; maybe also when plugged in with no battery
                            if (present == true)
                                sb.append(" UNKNOWN");
                            break;
                        case BatteryManager.BATTERY_STATUS_CHARGING:
                            sb.append(" CHARGING");
                            break;
                        case BatteryManager.BATTERY_STATUS_DISCHARGING:
                            sb.append(" DISCHARGING");
                            break;
                        case BatteryManager.BATTERY_STATUS_NOT_CHARGING:
                            sb.append(" NOTCHARGING");
                            break;
                        case BatteryManager.BATTERY_STATUS_FULL:
                            sb.append(" FULL");
                            break;
                        default:
                            if (present == true)
                                sb.append("Unknown");
                            break;
                        }
                    }

                sPowerStatus = sb.toString();
                }
            };

        IntentFilter battFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(battReceiver, battFilter);
        }
*/
   int setUpNetwork(char* sIniFile)
        {
  /*      char* ssid;
        char* auth;
        char* encr;
        char* key;
        char* eap;
        char* adhoc;
        int    bRet = 0;
        int    lcv    = 0;
        int    lcv2 = 0;
        WifiManager wifi = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        WifiConfiguration wc = new WifiConfiguration();

        String ssid = GetIniData("Network Settings", "SSID", sIniFile);
        String auth = tmpdc.GetIniData("Network Settings", "AUTH", sIniFile);
        String encr = tmpdc.GetIniData("Network Settings", "ENCR", sIniFile);
        String key = tmpdc.GetIniData("Network Settings", "KEY", sIniFile);
        String eap = tmpdc.GetIniData("Network Settings", "EAP", sIniFile);
        String adhoc = tmpdc.GetIniData("Network Settings", "ADHOC", sIniFile);

        Toast.makeText(getApplication().getApplicationContext(), "Starting and configuring network", Toast.LENGTH_LONG).show();
// *
        ContentResolver cr = getContentResolver();
        int nRet;
        try {
            nRet = Settings.System.getInt(cr, Settings.System.WIFI_USE_STATIC_IP);
            String foo2 = "" + nRet;
        } catch (SettingNotFoundException e1) {
            e1.printStackTrace();
        }
* //
        wc.SSID = "\"" + ssid + "\"";
//        wc.SSID = "\"Mozilla-G\"";
//        wc.SSID = "\"Mozilla\"";

        if (auth.contentEquals("wpa2"))
            {
            wc.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
            wc.preSharedKey  = null;
            }

        if (encr.contentEquals("aes"))
            {
            wc.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
            wc.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
            }

        if (eap.contentEquals("peap"))
            {
            wc.eap.setValue("PEAP");
            wc.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
            wc.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.IEEE8021X);
            }

        wc.hiddenSSID = false;
        wc.status = WifiConfiguration.Status.ENABLED;

        wc.password.setValue("\"password\"");
        wc.identity.setValue("\"bmoss@mozilla.com\"");

        if (!wifi.isWifiEnabled())
            wifi.setWifiEnabled(true);

        while(wifi.getWifiState() != WifiManager.WIFI_STATE_ENABLED)
            {
            Thread.yield();
            if (++lcv > 10000)
                return(bRet);
            }

        wl = wifi.createWifiLock(WifiManager.WIFI_MODE_FULL, "SUTAgent");
        if (wl != null)
            wl.acquire();

        WifiConfiguration    foo = null;
        int                    nNetworkID = -1;

        List<WifiConfiguration> connsLst =  wifi.getConfiguredNetworks();
        int nConns = connsLst.size();
        for (int i = 0; i < nConns; i++)
            {

            foo = connsLst.get(i);
            if (foo.SSID.equalsIgnoreCase(wc.SSID))
                {
                nNetworkID = foo.networkId;
                wc.networkId = foo.networkId;
                break;
                }
            }

        int res;

        if (nNetworkID != -1)
            {
            res = wifi.updateNetwork(wc);
            }
        else
            {
            res = wifi.addNetwork(wc);
            }

        Log.d("WifiPreference", "add Network returned " + res );

        boolean b = wifi.enableNetwork(res, true);
        Log.d("WifiPreference", "enableNetwork returned " + b );

        wifi.saveConfiguration();

        WifiInfo wi = wifi.getConnectionInfo();
        SupplicantState ss = wi.getSupplicantState();

        lcv = 0;
        lcv2 = 0;

        while (ss.compareTo(SupplicantState.COMPLETED) != 0)
            {
            try {
                Thread.sleep(1000);
                }
            catch (InterruptedException e)
                {
                e.printStackTrace();
                }

            if (wi != null)
                wi = null;
            if (ss != null)
                ss = null;
            wi = wifi.getConnectionInfo();
            ss = wi.getSupplicantState();
            if (++lcv > 60)
                {
                if (++lcv2 > 5)
                    {
                    Toast.makeText(getApplication().getApplicationContext(), "Unable to start and configure network", Toast.LENGTH_LONG).show();
                    return(bRet);
                    }
                else
                    {
                    Toast.makeText(getApplication().getApplicationContext(), "Resetting wifi interface", Toast.LENGTH_LONG).show();
                    if (wl != null)
                        wl.release();
                    wifi.setWifiEnabled(false);
                    while(wifi.getWifiState() != WifiManager.WIFI_STATE_DISABLED)
                        {
                        Thread.yield();
                        }

                    wifi.setWifiEnabled(true);
                    while(wifi.getWifiState() != WifiManager.WIFI_STATE_ENABLED)
                        {
                        Thread.yield();
                        }
                    b = wifi.enableNetwork(res, true);
                    Log.d("WifiPreference", "enableNetwork returned " + b );
                    if (wl != null)
                        wl.acquire();
                    lcv = 0;
                    }
                }
            }

        lcv = 0;
        while(getLocalIpAddress() == null)
            {
            if (++lcv > 10000)
                return(bRet);
            }

        Toast.makeText(getApplication().getApplicationContext(), "Network started and configured", Toast.LENGTH_LONG).show();
        bRet = true;
*/
        return 0;
        }
/*
    // If there is an update.info file callback the server and send the status
    private Runnable doUpdateCallback = new Runnable() {
        public void run() {
            DoCommand dc = new DoCommand(getApplication());
            String sRet = dc.UpdateCallBack("update.info");
            if (sRet.length() > 0) {
                if (sRet.contains("ok")) {
                    sRet = "Callback Server contacted successfully" + lineSep;
                } else if (sRet.contains("Nothing to do")) {
                    sRet = "";
                } else {
                    sRet = "Callback Server NOT contacted successfully" + lineSep;
                }
            }
            if (sRet.length() > 0)
                mHandler.post(new UpdateStatus(sRet));
            dc = null;
        }
    };

    private Runnable doSetClock = new Runnable() {
        public void run() {
            String sRet = "";

            DoCommand dc = new DoCommand(getApplication());

            sRet = dc.SetSystemTime(sNTPServer, null, null);

               mHandler.post(new UpdateStatus(sRet));

            dc = null;
        }
    };
*/
// registers with the reg server defined in the SUTAgent.ini file
void doRegisterDevice() {
  printf("RegSvrIPAddr = %s\n", RegSvrIPAddr);
  char sRet[1024] = "";
    if (strlen(RegSvrIPAddr) > 0) {
        char sRegRet[1024];
        RegisterTheDevice(RegSvrIPAddr, RegSvrIPPort, sRegString, strlen(sRegString), sRegRet);
        if (strstr(sRegRet, "ok") != NULL) {
            sprintf(sRet, "%sRegistered with testserver%s", sRet, lineSep);
            sprintf(sRet, "%s\tIPAddress: %s%s", sRet, RegSvrIPAddr, lineSep);
            if (strlen(RegSvrIPPort) > 0) {
                sprintf(sRet, "%s\tPort: %s%s",sRet, RegSvrIPPort, lineSep);
            }
        } else {
            sprintf(sRet, "%sNot registered with testserver%s",sRet, lineSep);
        }
    } else {
        sprintf(sRet, "%sNot registered with testserver%s",sRet, lineSep);
    }
    return;

}
/*
    // this starts the listener service for the command and data channels
    private Runnable doStartService = new Runnable()
        {
        public void run()
            {
            Intent listenerService = new Intent();
            listenerService.setAction("com.mozilla.SUTAgentC.service.LISTENER_SERVICE");
            startService(listenerService);
            }
        };
*/

//Function taken from http://stackoverflow.com/questions/212528/linux-c-get-the-ip-address-of-local-computer
    void getLocalIpAddress(char* sRet, char* mac)
      {
        struct ifaddrs* ifAddrStruct=NULL;
        struct ifaddrs* ifa=NULL;
        void* tmpAddrPtr=NULL;
        char* ifa_name=NULL;
        getifaddrs(&ifAddrStruct);
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
          if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            ifa_name = ifa->ifa_name;
            strcpy(sRet, addressBuffer);
          } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            ifa_name = ifa->ifa_name;
            strcpy(sRet, addressBuffer);
          } 
        }
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
          int family = ifa->ifa_addr->sa_family;
          /* removed family == AF_LINK from if statement*/
          if(strcmp(ifa_name, ifa->ifa_name) == 0){
            unsigned char* ptr = (unsigned char*)(((struct sockaddr *)ifa->ifa_addr)->sa_data);
            ptr+=9;
            sprintf( mac, "%02x:%02x:%02x:%02x:%02x:%02x\n", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4),*(ptr+5));
          }
        }

        if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
        return;
      }
