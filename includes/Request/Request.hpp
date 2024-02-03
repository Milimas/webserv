#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <netdb.h>
#include <cstring>
#include "../Server.hpp"

class Request
{
public:
private:
    const Server&   owner ;
	const int       socketfd ;
	struct sockaddr in_addr ;

	Request& operator=( const Request& rhs ) ;
	Request( void ) ;

public:
	Location::Method_t 	method ;
	std::string			uri ;
	std::string 		host ;
	bool				isConnectionClose ;
    std::string 		requestBody ;

	bool				isRequestFinished ;
	std::string			response ;
	bool				isReponseFinished ;

	Request( const int& socketfd, const Server& owner, const struct sockaddr& in_addr ) ;
	Request( const Request& rhs ) ;
	~Request( void ) ;

	// Getters
	const int&          			getSocketFD( void ) const ;
	const Server& 					getServer( void ) const ;

} ;

std::ostream& operator<<( std::ostream& os, const Request& server ) ;

#endif
