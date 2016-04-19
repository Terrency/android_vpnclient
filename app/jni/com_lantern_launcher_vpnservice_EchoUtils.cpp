#include "com_lantern_launcher_vpnservice_EchoUtils.h"
#include "hans/client.h"
#include "hans/server.h"
#include "hans/exception.h"

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <netdb.h>
// #include <uuid/uuid.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>


#ifdef __cplusplus
extern "C" {
#endif

static Worker *worker = NULL;

void Java_com_lantern_launcher_vpnservice_EchoUtils_connectToServer
  (JNIEnv *env, jclass jc, jstring svrip, jstring mtus){
	  
	const char *serverName = "172.16.130.46";
	//(*env)->GetStringUTFChars(env, svrip, 0);
    const char *userName = NULL;
    const char *password = "";
    const char *device = NULL;
    bool isServer = false;
    bool isClient = false;
    bool foreground = false;
    int mtu = 1500;
    int maxPolls = 10;
    uint32_t network = INADDR_NONE;
    uint32_t clientIp = INADDR_NONE;
    bool answerPing = false;
    uid_t uid = 0;
    gid_t gid = 0;
    bool changeEchoId = false;
    bool changeEchoSeq = false;
    bool verbose = false;
	
	uint32_t serverIp = inet_addr(serverName);
  
            if (serverIp == INADDR_NONE)
            {
                struct hostent* he = gethostbyname(serverName);
                if (!he)
                {
                    syslog(LOG_ERR, "gethostbyname: %s", hstrerror(h_errno));
                }

                serverIp = *(uint32_t *)he->h_addr;
            }

			
    worker = new Client(mtu, device, ntohl(serverIp), maxPolls, password, uid, gid, changeEchoId, changeEchoSeq, clientIp);
	
	if (!foreground)
        {
            syslog(LOG_INFO, "detaching from terminal");
            daemon(0, 0);
        }

    worker->run();
	
}

#ifdef __cplusplus
}
#endif