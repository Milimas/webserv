#include "../../includes/Request/Request.hpp"

const int&          			Request::getSocketFD( void ) const
{
    return (socketfd) ;
}

const Server& 					Request::getServer( void ) const
{
    return (owner) ;
}

Request::Request( const int& socketfd, const Server& owner, const struct sockaddr& in_addr ) : owner(owner), socketfd(socketfd), in_addr(in_addr), method(), body(), isParsed(false), response(), isFinished(false)
{
}

Request::Request( const Request& rhs ) : owner(rhs.owner), socketfd(rhs.socketfd), in_addr(rhs.in_addr), method(rhs.method), isParsed(rhs.isParsed), response(rhs.response), isFinished(rhs.isFinished)
{
    this->body = rhs.body ;
}

Request::~Request( void )
{   
}

Request& Request::operator=( const Request& rhs )
{
    (void) rhs ;
    memcpy(&this->in_addr, &rhs.in_addr, sizeof(struct sockaddr)) ;
    this->method = rhs.method ;
    this->body = rhs.body ;
    this->isParsed = rhs.isParsed ;
    this->response = rhs.response ;
    this->isFinished = rhs.isFinished ;
    return (*this) ;
}

void Request::parse( void )
{
    if (!isParsed)
    {
        if (body.find("\r\n\r\n") == std::string::npos )
        {
            return ;
        }
        std::stringstream ss(body) ;
        std::string line ;
        std::string requestLine ;
        getline(ss, requestLine, '\r') ;
        getline(ss, line) ; // skip \n after \r
        /**
         * split the requestLine to 3 parts
         * 1. Method (method)
         * 2. path (uri)
         * 3. version (HTTP/1.1)
         * 
         * you need to verify if the method is supported (we only support GET, POST and DELETE)
         * verify the version we only support HTTP/1.1
         * 
         * Errors:
         *  - Method not supported
         *  - Unsupported version (for http version)
         *  - bad request with status code 400
        */
        std::stringstream reqss(requestLine) ;
        reqss >> method >> uri >> httpVersion ;
        std::cout << "method: " << method << std::endl ;
        std::cout << "request uri: " << uri << std::endl ;
        std::cout << "http version: " << httpVersion << std::endl ;
        while (getline(ss, line))
        {
            size_t delimiterPos = line.find(": ") ;
            if (delimiterPos == std::string::npos)
            {
                if (line.empty() || line == "\r")
                    break;
                // bad header
                std::cout << "bad requst line \"" << line << "\"" << std::endl ;
                std::string response("HTTP/1.1 400 Bad Request\r\n\r\n") ;
                write(1, response.c_str(), response.size()) ;
                write(socketfd, response.c_str(), response.size()) ;
                break ;
            }
            std::string key = line.substr(0, delimiterPos) ;
            std::string value = line.substr(delimiterPos + 2, line.size() - 1) ;
            headers.insert(std::make_pair(key, value)) ;
        }
        isParsed = true ;
        // delete headers from body

        std::cout << "Host: " << headers["Host"] << std::endl ;
        std::cout << "Connection: " << headers["Connection"] << std::endl ;
        std::cout << "Accept: " << headers["Accept"] << std::endl ;
    }
}