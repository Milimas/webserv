#include "../../includes/Request/Request.hpp"

const int&          			Request::getSocketFD( void ) const
{
    return (socketfd) ;
}

const Server& 					Request::getServer( void ) const
{
    return (owner) ;
}

Request::Request( const int& socketfd, const Server& owner ) : state(START), owner(owner), socketfd(socketfd), method(), body()
{
    this->statusCode = 0 ;
}

Request::Request( const Request& rhs ) : owner(rhs.owner), socketfd(rhs.socketfd), method(rhs.method)
{
    this->state = START ;
    this->body = rhs.body ;
    this->statusCode = rhs.statusCode ;
}

Request::~Request( void )
{   
}

Request& Request::operator=( const Request& rhs )
{
    (void) rhs ;
    this->state = rhs.state ;
    this->method = rhs.method ;
    this->body = rhs.body ;
    this->statusCode = rhs.statusCode ;
    return (*this) ;
}

/**
 * NOTE: It is expected that the folding LWS will be 
 *       replaced with a single SP before interpretation of the TEXT value.
 *        LWS            = [CRLF] 1*( SP | HT )
*/
void LWS2SP( std::string& buf )
{
    // replace HT with SP
    size_t pos = 0;
    while((pos = buf.find(HT, pos)) != std::string::npos)
    {
        buf.replace(pos, 1, SP) ;
        pos += 1 ;
    }
    // find LWS and replace with SP
    pos = 0 ;
    while((pos = buf.find("\r\n ", pos)) != std::string::npos)
    {
        buf.replace(pos, 3, SP) ;
        pos += 1 ;
    }
    // replace 1*SP with SP
    pos = 0 ;
    while((pos = buf.find("  ", pos)) != std::string::npos)
    {
        buf.replace(pos, 2, SP) ;
        pos += 1 ;
    }
}

void Request::setStatusCode( const int errorNumber )
{
    this->statusCode = errorNumber ;    
}

Request::state_e Request::parseRequestLine( std::stringstream& ss )
{
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
    /**
     * check if the httpVersion is supported (the only supported version is HTTP/1.1)
     * if the version given by the request follow this sementics and its not HTTP/1.1
     * HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
     * then send "HTTP/1.1 505 HTTP Version Not Supported"
     * 
    */
    std::string line ;
    /**
     * In the interest of robustness, servers SHOULD ignore any empty
     * line(s) received where a Request-Line is expected.
    */
    while (line.empty() && ss.good() && !ss.eof()) 
    {
        getline(ss, line) ;
    }
    std::stringstream requestLine(line) ;
    requestLine >> headers["Method"] >> headers["Target"] >> std::ws ;
    getline(requestLine, headers["Version"], '\r') ;
    std::cout << "request line: " << line << std::endl ;
    if (headers["Version"].empty())
    {
        setStatusCode(400) ;
        return (ERROR);
    }
    return (REQUEST_LINE) ;
}

Request::state_e Request::parseHeaders( std::stringstream& ss )
{
    std::string line ;
    while (getline(ss, line))
    {
        std::cout << "line: " << line << std::endl ;
        if (line.empty()) // header not finished yet
        {
            std::cout << "NOT FINISHED" << std::endl ;
            return (state);
        }
        if (line == "\r") // header is finished CRLF
        {
            std::cout << "FINISHED" << std::endl ;
            headers_t::iterator it = headers.begin() ;
            while (it != headers.end())
            {
                std::cout << it->first << ": " << it->second << std::endl ;
                it++ ;
            }
            return (HEADER) ;
        }
        size_t delimiterPos = line.find(": ") ;
        if (delimiterPos == std::string::npos)
        {
            // bad header
            std::cout << "bad requst line \"" << line << "\"" << std::endl ;
            setStatusCode(400) ;
            return (ERROR) ;
        }
        std::string key = line.substr(0, delimiterPos) ;
        std::string value = line.substr(delimiterPos + 2, line.size() - 2) ;
        headers.insert(std::make_pair(key, value)) ;
    }
    return (state) ;
}

void Request::parse( std::string buf, ssize_t bytesReceived )
{
    (void) bytesReceived ;
    LWS2SP(buf) ;
    std::cout << "Normalized buf: '" << buf << "'" << std::endl ;
    std::stringstream ss(buf) ;
    /**
     * 
     * HTTP-message   = Request | Response     ; HTTP/1.1 messages
     * 
     * generic-message = start-line
     *                   *(message-header CRLF)
     *                   CRLF
     *                   [ message-body ]
     * 
     * start-line      = Request-Line | Status-Line
     * 
    */
    /**
     * Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    */
    if (state == START)
        state = parseRequestLine(ss) ;
    if (state == REQUEST_LINE)
    {
        
        state = parseHeaders(ss) ;
        /**
         * request-header = Accept                   ; Section 14.1
         *                | Accept-Charset           ; Section 14.2
         *                | Accept-Encoding          ; Section 14.3
         *                | Accept-Language          ; Section 14.4
         *                | Authorization            ; Section 14.8
         *                | Expect                   ; Section 14.20
         *                | From                     ; Section 14.22
         *                | Host                     ; Section 14.23
         *                | If-Match                 ; Section 14.24
         *                | If-Modified-Since        ; Section 14.25
         *                | If-None-Match            ; Section 14.26
         *                | If-Range                 ; Section 14.27
         *                | If-Unmodified-Since      ; Section 14.28
         *                | Max-Forwards             ; Section 14.31
         *                | Proxy-Authorization      ; Section 14.34
         *                | Range                    ; Section 14.35
         *                | Referer                  ; Section 14.36
         *                | TE                       ; Section 14.39
         *                | User-Agent               ; Section 14.43
        */
        /**
         * message-header = field-name ":" [ field-value ]
         * field-name     = token
         * field-value    = *( field-content | LWS )
         * field-content  = <the OCTETs making up the field-value
         *                     and consisting of either *TEXT or combinations
         *                     of token, separators, and quoted-string>
        */
        /**
         * In the interest of robustness, servers SHOULD ignore any empty
         * line(s) received where a Request-Line is expected. In other words, if
         * the server is reading the protocol stream at the beginning of a
         * message and receives a CRLF first, it should ignore the CRLF.
        */
        /**
         * A server SHOULD
         * read and forward a message-body on any request; if the request method
         * does not include defined semantics for an entity-body, then the
         * message-body SHOULD be ignored when handling the request.
        */
    }
    if (state == HEADER)
    {
        if ( headers["Method"] == "GET" )
        {
            state = FINISHED ;
            return ;
        }
        // proccess body for post
        state = BODY ;
    }
    if (state == BODY)
    {
        state = FINISHED ;
    }
    

    
    // you need to match location before checking if the method is allowed


}