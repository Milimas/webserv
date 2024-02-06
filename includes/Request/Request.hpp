#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <netdb.h>
#include <cstring>
#include <map>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <string>
#include "../Server.hpp"

#define CR 		"\r"
#define LF 		"\n"
#define CRLF 	CR + LF
#define SP 		" "
#define HT 		"\t"

class Request
{
public:
	enum state_e {
		START = 0,
		REQUEST_LINE,
		HEADER,
		BODY,
		FINISHED,
		WAIT_CLOSE,
		ERROR,
	} ;
	state_e state ;
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

	Request( const int& socketfd, const Server& owner, const struct sockaddr& in_addr ) ;
	Request( const Request& rhs ) ;
	~Request( void ) ;

	void parse( std::string buf, ssize_t bytesReceived ) ;
	void sendError( void ) ;

	// Getters
	const int&          			getSocketFD( void ) const ;
	const Server& 					getServer( void ) const ;

} ;

std::ostream& operator<<( std::ostream& os, const Request& server ) ;

#endif
