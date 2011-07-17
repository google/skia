#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "SkSockets.h"
#include "SkData.h"

SkSocket::SkSocket() {
    fMaxfd = 0;
    FD_ZERO(&fMasterSet);
    fConnected = false;
    fReady = false;
    fReadSuspended = false;
    fWriteSuspended = false;
    fSockfd = this->createSocket();
    fTimeout.tv_sec  = 0;
    fTimeout.tv_usec = 0;
}

SkSocket::~SkSocket() {
    this->closeSocket(fSockfd);
}

int SkSocket::createSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        //SkDebugf("ERROR opening socket\n");
        return -1;
    }
#ifdef NONBLOCKING_SOCKETS
    this->setNonBlocking(sockfd);
#endif
    //SkDebugf("Opened fd:%d\n", sockfd);
    fReady = true;
    return sockfd;
}

void SkSocket::closeSocket(int sockfd) {
    if (!fReady)
        return;
    
    //SkDebugf("Closed fd:%d\n", sockfd);
    close(sockfd);
    
    if (FD_ISSET(sockfd, &fMasterSet)) {
        FD_CLR(sockfd, &fMasterSet);
        if (sockfd >= fMaxfd) {
            while (FD_ISSET(fMaxfd, &fMasterSet) == false && fMaxfd > 0)
                fMaxfd -= 1;
        }
    }
    if (0 == fMaxfd) {
        fConnected = false;
        //SkDebugf("all connections closed\n");
    }
}

void SkSocket::onFailedConnection(int sockfd) {
    this->closeSocket(sockfd);
}

void SkSocket::setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void SkSocket::addToMasterSet(int sockfd) {
    FD_SET(sockfd, &fMasterSet);
    if (sockfd > fMaxfd)
        fMaxfd = sockfd;
}

int SkSocket::readPacket(void (*onRead)(const void*, size_t, int, DataType, 
                                        void*), void* context) {
    if (!fConnected || !fReady || NULL == onRead || fReadSuspended)
        return -1;

    int totalBytesRead = 0;

    char packet[PACKET_SIZE];
    for (int i = 0; i <= fMaxfd; ++i) {
        if (!FD_ISSET (i, &fMasterSet))
            continue;
        
        memset(packet, 0, PACKET_SIZE);
        SkDynamicMemoryWStream stream;
        int attempts = 0;
        bool failure = false;
        int bytesReadInTransfer = 0;
        int bytesReadInPacket = 0;
        header h;
        h.done = false;
        h.bytes = 0;
        while (!h.done && fConnected && !failure) {
            int retval = read(i, packet + bytesReadInPacket,
                              PACKET_SIZE - bytesReadInPacket);

            ++attempts;
            if (retval < 0) {
#ifdef NONBLOCKING_SOCKETS
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    if (bytesReadInPacket > 0 || bytesReadInTransfer > 0)
                        continue; //incomplete packet or frame, keep tring
                    else
                        break; //nothing to read
                }
#endif
                //SkDebugf("Read() failed with error: %s\n", strerror(errno));
                failure = true;
                break;
            }

            if (retval == 0) {
                //SkDebugf("Peer closed connection or connection failed\n");
                failure = true;
                break;
            }

            SkASSERT(retval > 0);
            bytesReadInPacket += retval;
            if (bytesReadInPacket < PACKET_SIZE) {
                //SkDebugf("Read %d/%d\n", bytesReadInPacket, PACKET_SIZE);
                continue; //incomplete packet, keep trying
            }

            SkASSERT((bytesReadInPacket == PACKET_SIZE) && !failure);
            memcpy(&h.done, packet, sizeof(bool));
            memcpy(&h.bytes, packet + sizeof(bool), sizeof(int));
            memcpy(&h.type, packet + sizeof(bool) + sizeof(int), sizeof(DataType));
            if (h.bytes > CONTENT_SIZE || h.bytes <= 0) {
                //SkDebugf("bad packet\n");
                failure = true;
                break;
            }
            //SkDebugf("read packet(done:%d, bytes:%d) from fd:%d in %d attempts\n",
            //         h.done, h.bytes, fSockfd, attempts);
            stream.write(packet + HEADER_SIZE, h.bytes);\
            bytesReadInPacket = 0;
            attempts = 0;
            bytesReadInTransfer += h.bytes;
        }

        if (failure) {
            onRead(NULL, 0, i, h.type, context);
            this->onFailedConnection(i);
            continue;
        }
        
        if (bytesReadInTransfer > 0) {
            SkData* data = stream.copyToData();
            SkASSERT(data->size() == bytesReadInTransfer);
            onRead(data->data(), data->size(), i, h.type, context);
            data->unref();
            
            totalBytesRead += bytesReadInTransfer;
        }
    }
    return totalBytesRead;
}

int SkSocket::writePacket(void* data, size_t size, DataType type) {
    if (size < 0|| NULL == data || !fConnected || !fReady || fWriteSuspended)
        return -1;

    int totalBytesWritten = 0;
    header h;
    char packet[PACKET_SIZE];
    for (int i = 0; i <= fMaxfd; ++i) {
        if (!FD_ISSET (i, &fMasterSet))
            continue;

        //Don't signal broken pipe
        setsockopt(i, SOL_SOCKET, SO_NOSIGPIPE, (void*)1, sizeof(int));
        int bytesWrittenInTransfer = 0;
        int bytesWrittenInPacket = 0;
        int attempts = 0;
        bool failure = false;
        while (bytesWrittenInTransfer < size && fConnected && !failure) {
            memset(packet, 0, PACKET_SIZE);
            h.done = (size - bytesWrittenInTransfer <= CONTENT_SIZE);
            h.bytes = (h.done) ? size - bytesWrittenInTransfer : CONTENT_SIZE;
            h.type = type;
            memcpy(packet, &h.done, sizeof(bool));
            memcpy(packet + sizeof(bool), &h.bytes, sizeof(int));
            memcpy(packet + sizeof(bool) + sizeof(int), &h.type, sizeof(DataType));
            memcpy(packet + HEADER_SIZE, (char*)data + bytesWrittenInTransfer,
                   h.bytes);

            int retval = write(i, packet + bytesWrittenInPacket,
                               PACKET_SIZE - bytesWrittenInPacket);
            attempts++;

            if (retval < 0) {
                if (errno == EPIPE) {
                    //SkDebugf("broken pipe, client closed connection");
                    failure = true;
                    break;
                }
#ifdef NONBLOCKING_SOCKETS
                else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    if (bytesWrittenInPacket > 0 || bytesWrittenInTransfer > 0)
                        continue; //incomplete packet or frame, keep tring
                    else
                        break; //client not available, skip current transfer
                }
#endif
                else {
                    //SkDebugf("write(%d) failed with error:%s\n", i,
                    //         strerror(errno));
                    failure = true;
                    break;
                }
            }

            bytesWrittenInPacket += retval;
            if (bytesWrittenInPacket < PACKET_SIZE) {
                //SkDebugf("Wrote %d/%d\n", bytesWrittenInPacket, PACKET_SIZE);
                continue; //incomplete packet, keep tring
            }

            SkASSERT(bytesWrittenInPacket == PACKET_SIZE);
            //SkDebugf("wrote to packet(done:%d, bytes:%d) to fd:%d in %d tries\n",
            //         h.done, h.bytes, i, attempts);
            bytesWrittenInTransfer += h.bytes;
            bytesWrittenInPacket = 0;
            attempts = 0;
        }

        if (failure) {
            //SkDebugf("Failed to write to fd:%d, terminating connection\n", i);
            this->onFailedConnection(i);
        }

        totalBytesWritten += bytesWrittenInTransfer;
    }
    return totalBytesWritten;
}
////////////////////////////////////////////////////////////////////////////////
SkTCPServer::SkTCPServer(int port) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(fSockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        //SkDebugf("ERROR on binding\n");
        fReady = false;
    }
}

SkTCPServer::~SkTCPServer() {
    this->disconnectAllConnections();
}

int SkTCPServer::acceptIncomingConnections() {
    if (!fReady)
        return -1;
//    if (fConnected)
//        return 0;

    listen(fSockfd, MAX_CLIENTS);
    ////SkDebugf("Accepting Incoming connections\n");
    int newfd;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
#ifdef NONBLOCKING_SOCKETS
        fd_set workingSet;
        FD_ZERO(&workingSet);
        FD_SET(fSockfd, &workingSet);
        int sel = select(fSockfd + 1, &workingSet, NULL, NULL, &fTimeout);
        if (sel < 0) {
            //SkDebugf("select() failed with error %s\n", strerror(errno));
            continue;
        }
        if (sel == 0) //select() timed out
            continue;
#endif
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        newfd = accept(fSockfd, (struct sockaddr*)&clientAddr, &clientLen);
        if (newfd< 0) {
            //SkDebugf("accept() failed with error %s\n", strerror(errno));
            continue;
        }
        //SkDebugf("New incoming connection - %d\n", newfd);
        fConnected = true;
#ifdef NONBLOCKING_SOCKETS
        this->setNonBlocking(newfd);
#endif
        this->addToMasterSet(newfd);
    }
    return 0;
}


int SkTCPServer::disconnectAllConnections() {
    ////SkDebugf("disconnecting server\n");
    if (!fConnected || !fReady)
        return -1;
    for (int i = 0; i <= fMaxfd; ++i)
    {
        if (FD_ISSET(i, &fMasterSet))
            this->closeSocket(i);
    }
    fConnected = false;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
SkTCPClient::SkTCPClient(const char* hostname, int port) {
    //Add fSockfd since the client will be using it to read/write
    this->addToMasterSet(fSockfd);
    
    hostent* server = gethostbyname(hostname);
    if (server) {
        fServerAddr.sin_family = AF_INET;
        memcpy((char*)&fServerAddr.sin_addr.s_addr, (char*)server->h_addr,
               server->h_length);
        fServerAddr.sin_port = htons(port);
    }
    else {
        //SkDebugf("ERROR, no such host\n");
        fReady = false;
    }
}

void SkTCPClient::onFailedConnection(int sockfd) {
    SkASSERT(sockfd == fSockfd);
    this->closeSocket(fSockfd);
    fSockfd = this->createSocket();
    //Add fSockfd since the client will be using it to read/write
    this->addToMasterSet(fSockfd);
}

int SkTCPClient::connectToServer() {
    if (!fReady)
        return -1;
    if (fConnected)
        return 0;

    int conn = connect(fSockfd, (sockaddr*)&fServerAddr, sizeof(fServerAddr));
    if (conn < 0) {
#ifdef NONBLOCKING_SOCKETS
        if (errno == EINPROGRESS || errno == EALREADY)
            return conn;
#endif
        if (errno != EISCONN) {
            //SkDebugf("error: %s\n", strerror(errno));
            this->onFailedConnection(fSockfd);
            return conn;
        }
    }
    fConnected = true;
    //SkDebugf("Succesfully reached server\n");
    return 0;
}
