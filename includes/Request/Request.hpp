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
					START = 0,		// Request ready to be parsed
					REQUEST_LINE,	// Request-Line parsed
					HEADER,			// Headers parsed
					BODY,			// body parsed
					FINISHED,		// Finished parsing the request
					ERROR,			// Error occured and statusCode is set to error number
					WAIT_CLOSE,		// Request socket waiting to be closed
	} ;
	state_e 		state ;
	int 			statusCode ;
private:
    Server*   		owner ;
	const int       socketfd ;

	Request& operator=( const Request& rhs ) ;
	Request( void ) ;

	typedef std::map<std::string, std::string> headers_t ;

public:
	std::string 		method ;
	std::string			uri ;
	std::string			httpVersion ;

	headers_t			headers ;
    std::string 		body ;

	Request( const int& socketfd ) ;
	Request( const Request& rhs ) ;
	~Request( void ) ;

	void parse( std::string buf ) ;
	state_e parseRequestLine( std::stringstream& ss ) ;
	state_e parseHeaders( std::stringstream& ss) ;

	/**
	 * when error set the state to ERROR and set the errorNumber
	*/
	void setStatusCode( const int errorNumber ) ; // this should not be here

	// Getters
	const int&          			getSocketFD( void ) const ;
	const Server* 					getServer( void ) const ;

} ;

std::ostream& operator<<( std::ostream& os, const Request& server ) ;

#endif
