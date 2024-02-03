#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <netdb.h>
#include <cstring>
#include <map>
#include <algorithm>
#include <sstream>
#include <unistd.h>
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

	typedef std::map<std::string, std::string> headers_t ;

public:
	std::string 		method ;
	std::string			uri ;
	std::string			httpVersion ;

	headers_t			headers ;
    std::string 		body ;

	bool				isParsed ;
	std::string			response ;
	bool				isFinished ;

	Request( const int& socketfd, const Server& owner, const struct sockaddr& in_addr ) ;
	Request( const Request& rhs ) ;
	~Request( void ) ;

	void parse( void ) ;

	// Getters
	const int&          			getSocketFD( void ) const ;
	const Server& 					getServer( void ) const ;

} ;

std::ostream& operator<<( std::ostream& os, const Request& server ) ;

#endif
