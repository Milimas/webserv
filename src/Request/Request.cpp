#include "../../includes/Request/Request.hpp"

const int&          			Request::getSocketFD( void ) const
{
    return (socketfd) ;
}

const Server& 					Request::getServer( void ) const
{
    return (owner) ;
}

Request::Request( const int& socketfd, const Server& owner, const struct sockaddr& in_addr ) : owner(owner), socketfd(socketfd), in_addr(in_addr), method(Location::NONE), host(), isConnectionClose(Location::NONE), requestBody(), isRequestFinished(false), response(), isReponseFinished(false)
{
}

Request::Request( const Request& rhs ) : owner(rhs.owner), socketfd(rhs.socketfd), in_addr(rhs.in_addr), method(rhs.method), host(rhs.host), isConnectionClose(rhs.isConnectionClose), requestBody(rhs.requestBody), isRequestFinished(rhs.isRequestFinished), response(rhs.response), isReponseFinished(rhs.isReponseFinished)
{
}

Request::~Request( void )
{   
}

Request& Request::operator=( const Request& rhs )
{
    (void) rhs ;
    memcpy(&this->in_addr, &rhs.in_addr, sizeof(struct sockaddr)) ;
    this->method = rhs.method ;
    this->isConnectionClose = rhs.isConnectionClose ;
    this->requestBody = rhs.requestBody ;
    this->isRequestFinished = rhs.isRequestFinished ;
    this->response = rhs.response ;
    this->isReponseFinished = rhs.isReponseFinished ;
    return (*this) ;
}