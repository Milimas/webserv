#ifndef SOCKET_MANAGER_HPP
#define SOCKET_MANAGER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

class SocketManager
{
private:

    SocketManager( void ) ;
    ~SocketManager( void ) ;
    SocketManager( const SocketManager& rhs ) ;
    const SocketManager& operator=( const SocketManager& rhs ) ;
    
public:
    static int epollFD ;
    static int createSocket( const char *host, const char *port, int ai_family = AF_INET, int ai_socktype = SOCK_STREAM, int ai_flags = AI_PASSIVE) ;
    static int makeSocketNonBlocking( int sfd ) ;
    static int startListening( int sfd ) ;
    static int createEpoll( void ) ;
    static int epollCtlSocket( int sfd, int op, uint32_t _events = EPOLLIN | EPOLLOUT ) ;

};


#endif