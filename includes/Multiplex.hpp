#ifndef MULTIPLEX_HPP
#define MULTIPLEX_HPP

#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <string>
#include <netdb.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>

#include "SocketManager.hpp"
#include "Server.hpp"
#include "Request/Request.hpp"

#define RESETTEXT  "\x1B[0m" // Set all colors back to normal.
#define FOREBLK  "\x1B[30m" // Black
#define FORERED  "\x1B[31m" // Red
#define FOREGRN  "\x1B[32m" // Green
#define FOREYEL  "\x1B[33m" // Yellow
#define FOREBLU  "\x1B[34m" // Blue
#define FOREMAG  "\x1B[35m" // Magenta
#define FORECYN  "\x1B[36m" // Cyan
#define FOREWHT  "\x1B[37m" // White

#define R_SIZE 1024 // Read Buffer Size

class Multiplex
{
public:
	typedef std::vector<Server> servers_t ;
	typedef std::map<int, Server> listeners_t ;
	typedef struct epoll_event epoll_event_t ;
    typedef std::map<int, Request> requests_t ;
	
private:
	static SOCKET 			epollFD ;
	static listeners_t		listeners ;
	static requests_t		requests ;
    static epoll_event_t 	events[SOMAXCONN];

	Multiplex( void ) ;
	Multiplex( const Multiplex& rhs ) ;
	Multiplex& operator=( const Multiplex& rhs ) ;
	~Multiplex( void ) ;
public:
    // static void setServers( const servers_t& servers ) ;
    static void start( void ) ;
    static void setup( const servers_t& _servers ) ;
	static void newConnection( SOCKET fd ) ;
	static void handleRequest( Request& request ) ;

	// Getters

	// Setters
} ;

std::ostream& operator<<( std::ostream& os, const Multiplex& server ) ;

#endif
