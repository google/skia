/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNetIO_DEFINED
#define SkNetIO_DEFINED

#include <netinet/in.h>
#include <sys/socket.h>
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"

/* PACKET and HEADER Format */
#define PACKET_SIZE 1024
#define HEADER_SIZE 20
#define CONTENT_SIZE 1004

#define DEFAULT_PORT 15555
#define MAX_WAITING_CLIENTS 3
#define NONBLOCKING_SOCKETS

class SkSocket {
public:
    SkSocket();
    virtual ~SkSocket();

    enum State {
        kError_state,
        kBegin_state,
        kIncomplete_state,
        kDone_state
    };

    enum DataType {
        kPipeAppend_type,
        kPipeReplace_type,
        kString_type,
        kInt_type
    };

    bool isConnected() { return fConnected; }
    /**
     * Write data to the socket. Data is a pointer to the beginning of the data
     * to be sent and dataSize specifies the number of bytes to send. This
     * method will spread the data across multiple packets if the data can't all
     * fit in a single packet. The method will write all the data to each of the
     * socket's open connections until all the bytes have been successfully sent
     * and return total the number of bytes written to all clients, unless there
     * was an error during the transfer, in which case the method returns -1.
     * For blocking sockets, write will block indefinitely if the socket at the
     * other end of the connection doesn't receive any data.
     * NOTE: This method guarantees that all of the data will be sent unless
     * there was an error, so it may block temporarily when the write buffer is
     * full
     */
    int writePacket(void* data, size_t size, DataType type = kPipeAppend_type);

    /**
     * Read a logical packet from socket. The data read will be stored
     * sequentially in the dataArray. This method will keep running until all
     * the data in a logical chunk has been read (assembling multiple partial
     * packets if necessary) and return the number of bytes successfully read,
     * unless there was an error, in which case the method returns -1. \For
     * nonblocking sockets, read will return 0 if there's nothing to read. For
     * blocking sockets, read will block indefinitely if the socket doesn't
     * receive any data.
     * NOTE: This method guarantees that all the data in a logical packet will
     * be read so it may block temporarily if it's waiting for parts of a
     * packet
     */
    int readPacket(void (*onRead)(int cid, const void* data, size_t size,
                                  DataType type, void*), void* context);

    /**
     * Suspend network transfers until resume() is called. Leaves all
     * connections in tact.
     */
    void suspendAll() { fReadSuspended = fWriteSuspended = true; }
    /**
     * Resume all network transfers.
     */
    void resumeAll() { fReadSuspended = fWriteSuspended = false; }
    /**
     * Other helper functions
     */
    void suspendRead() { fReadSuspended = true; }
    void resumeRead() { fReadSuspended = false; }
    void suspendWrite()  { fWriteSuspended = true; }
    void resumeWrite()  { fWriteSuspended = false; }

protected:
    struct header {
        bool        done;
        int         bytes;
        DataType    type;
    };

    /**
     * Create a socket and return its file descriptor. Returns -1 on failure
     */
    int createSocket();

    /**
     * Close the socket specified by the socket file descriptor argument. Will
     * update fMaxfd and working set properly
     */
    void closeSocket(int sockfd);

    /**
     * Called when a broken or terminated connection has been detected. Closes
     * the socket file descriptor and removes it from the master set by default.
     * Override to handle broken connections differently
     */
    virtual void onFailedConnection(int sockfd);

    /**
     * Set the socket specified by the socket file descriptor as nonblocking
     */
    void setNonBlocking(int sockfd);

    /**
     * Add the socket specified by the socket file descriptor to the master
     * file descriptor set, which is used to in the select() to detect new data
     * or connections
     */
    void addToMasterSet(int sockfd);

    bool    fConnected;
    bool    fReady;
    bool    fReadSuspended;
    bool    fWriteSuspended;
    int     fMaxfd;
    int     fPort;
    int     fSockfd;

    /**
     * fMasterSet contains all the file descriptors to be used for read/write.
     * For clients, this only contains the client socket. For servers, this
     * contains all the file descriptors associated with established connections
     * to clients
     */
    fd_set  fMasterSet;
};

/*
 * TCP server. Can accept simultaneous connections to multiple SkTCPClients and
 * read/write data back and forth using read/writePacket calls. Port number can
 * be specified, but make sure that client/server use the same port
 */
class SkTCPServer : public SkSocket {
public:
    SkTCPServer(int port = DEFAULT_PORT);
    virtual ~SkTCPServer();

    /**
     * Accept any incoming connections to the server, will accept 1 connection
     * at a time. Returns -1 on error. For blocking sockets, this method will
     * block until a client calls connectToServer()
     */
    int acceptConnections();

    /**
     * Disconnect all connections to clients. Returns -1 on error
     */
    int disconnectAll();
private:
    typedef SkSocket INHERITED;
};

/*
 * TCP client. Will connect to the server specified in the constructor. If a
 * port number is specified, make sure that it's the same as the port number on
 * the server
 */
class SkTCPClient : public SkSocket {
public:
    SkTCPClient(const char* hostname, int port = DEFAULT_PORT);

    /**
     * Connect to server. Returns -1 on error or failure. Call this to connect
     * or reconnect to the server. For blocking sockets, this method will block
     * until the connection is accepted by the server.
     */
    int connectToServer();
protected:
    /**
     * Client needs to recreate the socket when a connection is broken because
     * connect can only be called successfully once.
     */
    virtual void onFailedConnection(int sockfd);
private:
    sockaddr_in fServerAddr;
    typedef SkSocket INHERITED;
};

#endif
