/***************************************************************************
 *   This file is part of "libaiml"                                        *
 *   Copyright (C) 2005 by V01D                                            *
 *                                                                         *
 *   "libaiml" is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   "libaiml" is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with "libaiml"; if not, write to the                            *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <std_utils/std_util.h>
#include <fstream>
#include <iostream>
#include "tlib_log.h"

#include "../src/aiml.h"
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;
using std_util::strip;
using namespace aiml;

class cServerAppCallbacks : public cInterpreterCallbacks {
  public:
    void onAimlLoad(const std::string& filename) {
      cout << "Loaded " << filename << endl;
    }
};

int main(int argc, char* argv[]) {

  char szServerAddr[32] ;
  short nServerPort = 9527;
  int ch;
  
  while((ch = getopt(argc,argv,"a:p:"))!= -1)
  {
    switch(ch)
    {
      case 'a': 
        strncpy(szServerAddr,optarg,sizeof(szServerAddr)-1);
        break;
      case 'p': 
        nServerPort = atoi(optarg);
        break;
      default: 
        printf("useage  -a serveraddr -p port\n");
        return -2;
    }
  }


  TLib_Log_LogInit("server_aiml", 10000000, 5, 1); 

  cInterpreter* interpreter = cInterpreter::newInterpreter();

  int ret = 0;

  int fd = 0;
  sockaddr_in   server_addr, client_addr;
  fd = socket(AF_INET,SOCK_DGRAM,0);
  if(fd < 0)
  {
    TLib_Log_LogMsg("Init socket failed\n");
    return -1;
  }

  memset(&server_addr,0,sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(szServerAddr);
  server_addr.sin_port = htons(nServerPort);

  ret = bind(fd,(sockaddr*)&server_addr,sizeof(server_addr));
  if (ret < 0)
  {
    TLib_Log_LogMsg("bind addr and port on %s:%d failed.",szServerAddr,nServerPort);
    return -3;
  }


  // exceptions are used because returning in the middle of the program wouldn't let 'interpreter' be freed
  try {
    cServerAppCallbacks myCallbacks;
    interpreter->registerCallbacks(&myCallbacks);
    
    TLib_Log_LogMsg("Initializing interpreter...\n");
    if (!interpreter->initialize("libaiml.xml")) throw 1;
    
    string result;
    std::list<cMatchLog> log;

    while (true) {
      char szMsg[1024] ;
      memset(szMsg,0,sizeof(szMsg));

      int len = sizeof(client_addr);
      int n = recvfrom(fd,szMsg,sizeof(szMsg)-1,0,(sockaddr*)&client_addr,(socklen_t*)&len);
      if(n < 0)
      {
        TLib_Log_LogMsg("recv failed of %d\n",n);
        continue;
      }

      //get username and msg content
      char szUserName[32];
      char szContent[1024];
      memset(szUserName,0,sizeof(szUserName));
      memset(szContent,0,sizeof(szContent));
      memcpy(szUserName,szMsg,sizeof(szUserName));// first 32 bytes means user name
      szUserName[sizeof(szUserName)-1] = 0;
      
      strncpy(szContent,&szMsg[sizeof(szUserName)],sizeof(szContent)-1);

      /** remove the last parameter to avoid logging the match **/
      //if (!interpreter->respond(line, "localhost", result, &log)) throw 3;
      int res_ret = interpreter->respond(szContent,szUserName,result,&log);
      if(res_ret == 0)
      {
        //don't know what to say
        strcpy(szMsg,"不知道你在说什么。。。");
      }else{
          strcpy(szMsg,result.c_str());

      }

      TLib_Log_LogMsg("%s:%s",szUserName,szContent);
      TLib_Log_LogMsg("Computer:%s",szMsg);
      n = sendto(fd,szMsg,strlen(szMsg),0,(sockaddr*)&client_addr,len);
      if(n < 0)
      {
        TLib_Log_LogMsg("Send msg faild of %d",n);
      }
      
      cout << "Bot: " << strip(result) << endl;
      cout << "Match path:" << endl;
      for(list<cMatchLog>::const_iterator it_outter = log.begin(); it_outter != log.end(); ++it_outter) {
        cout << "\tpattern:\t";
        for (list<string>::const_iterator it = it_outter->pattern.begin(); it != it_outter->pattern.end(); ++it) { cout << "[" << *it << "] "; }
        cout << endl << "\tthat:\t\t";
        for (list<string>::const_iterator it = it_outter->that.begin(); it != it_outter->that.end(); ++it) { cout << "[" << *it << "] "; }
        cout << endl << "\ttopic:\t\t";
        for (list<string>::const_iterator it = it_outter->topic.begin(); it != it_outter->topic.end(); ++it) { cout << "[" << *it << "] "; }
        cout << endl << endl;
      }
    }
  
    /** Uncomment this line out and you'll see that the bot will no longer remember user vars **/
    //interpreter->unregisterUser("localhost");
  }
  catch(int _ret) {
    cout << "ERROR: " << interpreter->getErrorStr(interpreter->getError()) << " (" << interpreter->getError() << ")" << endl;
    if (!interpreter->getRuntimeErrorStr().empty()) cout << "Runtime Error: " << interpreter->getRuntimeErrorStr() << endl;
    ret = _ret;
  }

  delete interpreter;
  // the above is equivalent to cInterpreter::freeInterpreter(interpreter);
  return ret;
}
