/*
 *  Hans - IP over ICMP
 *  Copyright (C) 2009 Friedrich Schöller <hans@schoeller.se>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "client.h"
#include "server.h"
#include "exception.h"
#include "config.h"
#include "utility.h"

#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <syslog.h>
#include <stdlib.h>

using namespace std;

const Worker::TunnelHeader::Magic Client::magic("hanc");

Client::Client(int tunnelMtu, const char *deviceName, uint32_t serverIp,
               int maxPolls, const char *passphrase, uid_t uid, gid_t gid,
               bool changeEchoId, bool changeEchoSeq, uint32_t desiredIp)
: Worker(tunnelMtu, deviceName, false, uid, gid), auth(passphrase)
{
	printf("new client started\n");
    this->serverIp = serverIp;
    this->clientIp = INADDR_NONE;
    this->desiredIp = desiredIp;
    this->maxPolls = maxPolls;
    this->nextEchoId = Utility::rand();
    this->changeEchoId = changeEchoId;
    this->changeEchoSeq = changeEchoSeq;
    this->nextEchoSequence = Utility::rand();

    state = STATE_CLOSED;
	printf("new client finish\n");
}

Client::~Client()
{

}

void Client::sendConnectionRequest()
{
	printf("client send request\n");
    Server::ClientConnectData *connectData = (Server::ClientConnectData *)echoSendPayloadBuffer();
    connectData->maxPolls = maxPolls;
    connectData->desiredIp = desiredIp;

    syslog(LOG_DEBUG, "sending connection request");

    sendEchoToServer(TunnelHeader::TYPE_CONNECTION_REQUEST, sizeof(Server::ClientConnectData));

    state = STATE_CONNECTION_REQUEST_SENT;
    setTimeout(5000);
}

void Client::sendChallengeResponse(int dataLength)
{
    if (dataLength != CHALLENGE_SIZE)
        throw Exception("invalid challenge received");

    state = STATE_CHALLENGE_RESPONSE_SENT;

    syslog(LOG_DEBUG, "sending challenge response");

    vector<char> challenge;
    challenge.resize(dataLength);
    memcpy(&challenge[0], echoReceivePayloadBuffer(), dataLength);

    Auth::Response response = auth.getResponse(challenge);

    memcpy(echoSendPayloadBuffer(), (char *)&response, sizeof(Auth::Response));
    sendEchoToServer(TunnelHeader::TYPE_CHALLENGE_RESPONSE, sizeof(Auth::Response));

    setTimeout(5000);
}

bool Client::handleEchoData(const TunnelHeader &header, int dataLength, uint32_t realIp, bool reply, uint16_t id, uint16_t seq)
{
	char cmdline[512];
	printf("client handle echo data\n");
    if (realIp != serverIp || !reply)
        return false;

    if (header.magic != Server::magic)
        return false;

    switch (header.type)
    {
        case TunnelHeader::TYPE_RESET_CONNECTION:
            syslog(LOG_DEBUG, "reset reveiced");

            sendConnectionRequest();
            return true;
        case TunnelHeader::TYPE_SERVER_FULL:
            if (state == STATE_CONNECTION_REQUEST_SENT)
            {
                throw Exception("server full");
            }
            break;
        case TunnelHeader::TYPE_CHALLENGE:
            if (state == STATE_CONNECTION_REQUEST_SENT)
            {
                syslog(LOG_DEBUG, "challenge received");
                sendChallengeResponse(dataLength);
                return true;
            }
            break;
        case TunnelHeader::TYPE_CONNECTION_ACCEPT:
            if (state == STATE_CHALLENGE_RESPONSE_SENT)
            {
                if (dataLength != sizeof(uint32_t))
                {
                    throw Exception("invalid ip received");
					printf("invalid ip received\n");
                    return true;
                }

                syslog(LOG_INFO, "connection established");
				printf("connection established\n");

                uint32_t ip = ntohl(*(uint32_t *)echoReceivePayloadBuffer());
                if (ip != clientIp)
                {
                    if (privilegesDropped)
                        throw Exception("could not get the same ip address, so root privileges are required to change it");

                    clientIp = ip;
                    desiredIp = ip;
                    tun->setIp(ip, (ip & 0xffffff00) + 1, false);
					printf("client set ip\n");
					//设置客户端路由表---废弃
					//route add -host 182.254.245.209 gw 192.168.1.253
					//route add default gw 10.1.2.1
					//route del default gw 192.168.1.253
					
					//echo 60 tun0 >> /data/misc/net/rt_tables
					//ip rule add prio 100 from all lookup tun0
					//ip route add table tun0 182.254.245.209  via 192.168.1.253 dev wlan0
					//ip route add table tun0 182.254.245.209 via `getprop dhcp.wlan0.gateway` dev wlan0
					//ip route add table tun0 10.1.2.1 dev tun0
					//ip route add table tun0 0.0.0.0/1 via 10.1.2.1 dev tun0
					//ip route add table tun0 128.0.0.0/1 via 10.1.2.1 dev tun0
					
					snprintf(cmdline, sizeof(cmdline), "echo 60 tun0 >> /data/misc/net/rt_tables");
					system("echo 60 tun0 >> /data/misc/net/rt_tables");
					printf("echo 60 tun0 >> /data/misc/net/rt_tables\n");
					
					snprintf(cmdline, sizeof(cmdline), "ip rule add prio 100 from all lookup tun0");
					system("ip rule add prio 100 from all lookup tun0");
					printf("ip rule add prio 100 from all lookup tun0\n");
					
					string ips = Utility::formatIp(serverIp);
					snprintf(cmdline, sizeof(cmdline), "ip route add table tun0 %s via `getprop net.rmnet0.gw` dev rmnet0", ips.c_str());
					//snprintf(cmdline, sizeof(cmdline), "ip route add table tun0 %lu via `getprop dhcp.wlan0.gateway` dev wlan0", serverIp);
					system(cmdline);
					printf("ip route add table tun0 %s via `getprop net.rmnet0.gw` dev rmnet0\n", ips.c_str());
					
					snprintf(cmdline, sizeof(cmdline), "ip route add table tun0 10.1.2.1 dev tun0");
					system("ip route add table tun0 10.1.2.1 dev tun0");
					printf("ip route add table tun0 10.1.2.1 dev tun0\n");
					
					snprintf(cmdline, sizeof(cmdline), "ip route add table tun0 0.0.0.0/1 via 10.1.2.1 dev tun0");
					system("ip route add table tun0 0.0.0.0/1 via 10.1.2.1 dev tun0");
					printf("ip route add table tun0 0.0.0.0/1 via 10.1.2.1 dev tun0\n");
					
					snprintf(cmdline, sizeof(cmdline), "ip route add table tun0 128.0.0.0/1 via 10.1.2.1 dev tun0");
					system("ip route add table tun0 128.0.0.0/1 via 10.1.2.1 dev tun0");
					printf("ip route add table tun0 128.0.0.0/1 via 10.1.2.1 dev tun0\n");
					
					/**
					snprintf(cmdline, sizeof(cmdline), "busybox route add -host 182.254.245.209 gw 192.168.1.253");
					if (system(cmdline) != 0)
					{
						syslog(LOG_ERR, "could not add route");
						printf("could not add route\n");
					}else{
						printf("busybox route add -host 182.254.245.209 gw 192.168.1.253");
					}
					snprintf(cmdline, sizeof(cmdline), "busybox route add default gw 10.1.2.1");
					if (system(cmdline) != 0)
					{
						syslog(LOG_ERR, "could not add route");
						printf("could not add route\n");
					}else{
						printf("busybox route add default gw 10.1.2.1\n");
					}
					snprintf(cmdline, sizeof(cmdline), "busybox route del default gw 192.168.1.253");
					if (system(cmdline) != 0)
					{
						syslog(LOG_ERR, "could not del route");
						printf("could not del route\n");
					}else{
						printf("busybox route del default gw 192.168.1.253\n");
					}
					**/
					
                }
                state = STATE_ESTABLISHED;

                dropPrivileges();
                startPolling();

                return true;
            }
            break;
        case TunnelHeader::TYPE_CHALLENGE_ERROR:
            if (state == STATE_CHALLENGE_RESPONSE_SENT)
            {
                throw Exception("password error");
            }
            break;
        case TunnelHeader::TYPE_DATA:
            if (state == STATE_ESTABLISHED)
            {
                handleDataFromServer(dataLength);
                return true;
            }
            break;
    }

    syslog(LOG_DEBUG, "invalid packet type: %d, state: %d", header.type, state);

    return true;
}

void Client::sendEchoToServer(int type, int dataLength)
{
    if (maxPolls == 0 && state == STATE_ESTABLISHED)
        setTimeout(KEEP_ALIVE_INTERVAL);

    sendEcho(magic, type, dataLength, serverIp, false, nextEchoId, nextEchoSequence);

    if (changeEchoId)
        nextEchoId = nextEchoId + 38543; // some random prime
    if (changeEchoSeq)
        nextEchoSequence = nextEchoSequence + 38543; // some random prime
}

void Client::startPolling()
{
    if (maxPolls == 0)
    {
        setTimeout(KEEP_ALIVE_INTERVAL);
    }
    else
    {
        for (int i = 0; i < maxPolls; i++)
            sendEchoToServer(TunnelHeader::TYPE_POLL, 0);
        setTimeout(POLL_INTERVAL);
    }
}

void Client::handleDataFromServer(int dataLength)
{
    if (dataLength == 0)
    {
        syslog(LOG_WARNING, "received empty data packet");
		printf("received empty data packet\n");
        return;
    }

    sendToTun(dataLength);

    if (maxPolls != 0)
        sendEchoToServer(TunnelHeader::TYPE_POLL, 0);
}

void Client::handleTunData(int dataLength, uint32_t sourceIp, uint32_t destIp)
{
    if (state != STATE_ESTABLISHED)
        return;

    sendEchoToServer(TunnelHeader::TYPE_DATA, dataLength);
}

void Client::handleTimeout()
{
    switch (state)
    {
        case STATE_CONNECTION_REQUEST_SENT:
        case STATE_CHALLENGE_RESPONSE_SENT:
            sendConnectionRequest();
            break;

        case STATE_ESTABLISHED:
            sendEchoToServer(TunnelHeader::TYPE_POLL, 0);
            setTimeout(maxPolls == 0 ? KEEP_ALIVE_INTERVAL : POLL_INTERVAL);
            break;
        case STATE_CLOSED:
            break;
    }
}

void Client::run()
{
    now = Time::now();
	printf("client::run\n");
    sendConnectionRequest();

    Worker::run();
}
