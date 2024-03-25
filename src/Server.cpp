#include "../includes/Server.hpp"

Server::Server( void ) : _serverNames(), _port(), _root("/var/www/"), _host(""), _clientMaxBodySize(), _errorPages()
{
}

Server::Server( const Server& rhs )
{
    serverNames_t::const_iterator itS = rhs._serverNames.begin() ;
    while (itS != rhs._serverNames.end())
        this->_serverNames.push_back(*itS++) ;
    
    this->_port = rhs._port ;
    this->_host = rhs._host ;
    this->_root = rhs._root ;
    this->_index = rhs._index ;
    this->_clientMaxBodySize = rhs._clientMaxBodySize ;

    errorPages_t::const_iterator itE = rhs._errorPages.begin() ;
    while (rhs._errorPages.size() &&  itE != rhs._errorPages.end())
        this->_errorPages.insert(*itE++) ;

    std::map<std::string, Location>::const_iterator itD = rhs._locations.begin() ;
    while (rhs._locations.size() &&  itD != rhs._locations.end())
        this->_locations.insert(*itD++) ;
}

Server& Server::operator=( const Server& rhs )
{
    
    serverNames_t::const_iterator itS = rhs._serverNames.begin() ;
    this->_serverNames.clear() ;
    while (itS != rhs._serverNames.end())
        this->_serverNames.push_back(*itS++) ;

    this->_port = rhs._port ;
    this->_host = rhs._host ;
    this->_root = rhs._root ;
    this->_index = rhs._index ;
    this->_clientMaxBodySize = rhs._clientMaxBodySize ;

    errorPages_t::const_iterator itE = rhs._errorPages.begin() ;
    this->_errorPages.clear() ;
    while (rhs._errorPages.size() &&  itE != rhs._errorPages.end())
        this->_errorPages.insert(*itE++) ;

    Locations_t::const_iterator itD = rhs._locations.begin() ;
    this->_locations.clear() ;
    while (itD != rhs._locations.end())
        this->_locations.insert(*itD++) ;
    return (*this) ;
}

Server::~Server( void )
{
}

const Server::serverNames_t&						Server::getServerNames ( void ) const
{
    return (_serverNames) ;
}

const std::vector<std::string>&						Server::getIndex ( void ) const
{
    return (_index) ;
}

const std::string&						Server::getPort ( void ) const
{
    return (_port) ;
}

const std::string&						Server::getRoot ( void ) const
{
    return (_root) ;
}

const std::string&						Server::getHost ( void ) const
{
    return (_host) ;
}

const size_t&							Server::getClientMaxBodySize ( void ) const
{
    return (_clientMaxBodySize) ;
}

const Server::errorPages_t&				Server::getErrorPages ( void ) const
{
    return (_errorPages) ;
}

const std::map<std::string, Location>&	Server::getLocations ( void ) const
{
    return (_locations) ;
}

void 							Server::setServerNames( const Server::serverNames_t& _serverNames )
{
    serverNames_t::const_iterator itS = _serverNames.begin() ;
    this->_serverNames.clear() ;
    while (itS !=_serverNames.end())
        this->_serverNames.push_back(*itS++) ;
}

void 							Server::setIndex( const std::string& _index )
{
    this->_index.push_back(_index) ;
}

void 							Server::setPort( const std::string& _port )
{
    this->_port = _port ;
}

void 							Server::setRoot( const std::string& _root )
{
    this->_root = _root ;
}

void 							Server::setHost( const std::string& _host )
{
    this->_host = _host ;
}

void 							Server::setClientMaxBodySize( const size_t& _clientMaxBodySize )
{
    this->_clientMaxBodySize = _clientMaxBodySize ;
}

void 							Server::setErrorPages( const Server::errorPages_t& _errorPages )
{
    errorPages_t::const_iterator itE = _errorPages.begin() ;
    this->_errorPages.clear() ;
    while (_errorPages.size() &&  itE != _errorPages.end())
        this->_errorPages.insert(*itE++) ;
}

void 							Server::setLocations( const Locations_t& _locations )
{
    this->_locations = _locations ;
}

void 							Server::appendLocation( const std::string& path, const Location& _location )
{
    if (path.empty())
        throw std::runtime_error("empty path in location") ;
    this->_locations[path] = _location ;
}

void 							Server::appendErrorPage( const int& statusCode, const std::string& errorPagePath )
{
    if (errorPagePath.empty())
        throw std::runtime_error("expected error page path") ;
    this->_errorPages[statusCode] = errorPagePath ;
}

void 							Server::appendServerName( const std::string& serverName )
{
    if (serverName.empty())
        throw std::runtime_error("expected server name") ;
    this->_serverNames.push_back(serverName) ;
}

void 							Server::setSocketFD( const int& fd )
{
    this->socketfd = fd ;
}

std::ostream& operator<<( std::ostream& os, const Server& server )
{
    int borderWitdh = 75 ;
    os << std::setw(borderWitdh) << std::setfill('-') << "" << std::endl ;

    os << "Server Names: " ;
    Server::serverNames_t serverNames = server.getServerNames() ;
    Server::serverNames_t::const_iterator itS = serverNames.begin() ;
    while (itS != serverNames.end())
    {
        os << *itS << " " ;
        itS++ ;
    }
    os << std::endl ;

    std::vector<std::string>::const_iterator inx = server.getIndex().begin() ;
    os << "\t\tIndex: " ;
    while (inx != server.getIndex().end())
    {
        os << *inx++ ;
        if (inx != server.getIndex().end())
            os << ", " ;
        
    }
    os << "Port: " << server.getPort() << std::endl ;
    os << "Root: " << server.getRoot() << std::endl ;
    os << "Host: " << server.getHost() << std::endl ;
    os << "Client Max Body Size: " << server.getClientMaxBodySize() << std::endl ;

    os << "Error Pages: " << std::endl ;
    Server::errorPages_t errorPages = server.getErrorPages() ;
    Server::errorPages_t::const_iterator itE = errorPages.begin() ;
    while (itE != errorPages.end())
    {
        os << "\t" << itE->first << ": " << itE->second << std::endl ;
        itE++ ;
    }

    os << "Locations: " << std::endl ;
    Server::Locations_t locations = server.getLocations() ;
    Server::Locations_t::const_iterator it = locations.begin() ;
    while (it != locations.end())
    {
        os << "\t" << it->first << ": " << std::endl << it->second ;
        it++ ;
    }
    os << std::setw(borderWitdh) << std::setfill('-') << "" << std::endl ;
    return (os) ;
}