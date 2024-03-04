#include "../../includes/Multiplex.hpp"

SOCKET                      Multiplex::epollFD ;
Multiplex::listeners_t      Multiplex::listeners ;
Multiplex::requests_t       Multiplex::requests ;
Multiplex::epoll_event_t    Multiplex::events[SOMAXCONN] = {} ;

void Multiplex::setup( const servers_t& servers )
{
    if (servers.empty())
    {
        throw std::runtime_error("Servers are not set") ;
        return ;
    }

    servers_t::const_iterator servIt = servers.begin() ;
    // each server should have a socket
    epollFD = SocketManager::createEpoll() ;
    while (servIt != servers.end())
    {
        std::cout << "Listening on: " << servIt->getHost() << ":" << servIt->getPort() << "..." << std::endl ;
        SOCKET sfd = SocketManager::createSocket(servIt->getHost().c_str(), servIt->getPort().c_str(), AF_INET, SOCK_STREAM, AI_PASSIVE) ;
        SocketManager::makeSocketNonBlocking (sfd);
        SocketManager::startListening(sfd) ;
        SocketManager::epollCtlSocket(sfd, EPOLL_CTL_ADD) ;
        listeners[sfd] = *servIt ;
        servIt++ ;
    }
}

void Multiplex::newConnection( SOCKET fd )
{
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd, s;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    in_len = sizeof in_addr;
    infd = accept (fd, &in_addr, &in_len); // Accept connection
    if (!ISVALIDSOCKET(infd))
    {
        if ((errno == EAGAIN) ||
            (errno == EWOULDBLOCK))
        {
            /* We have processed all incoming
                connections. */
            return ;
        }
        else
        {
            perror ("accept");
            return ;
        }
    }
    s = getnameinfo (&in_addr, in_len,
                    hbuf, sizeof hbuf,
                    sbuf, sizeof sbuf,
                    NI_NUMERICHOST | NI_NUMERICSERV);
    if (s == 0)
    {
        printf("Accepted connection on descriptor %d "
                "(host=%s, port=%s)\n", infd, hbuf, sbuf);
    }
    /**
     * Make the incoming socket non-blocking and add it to the list of fds to monitor. 
    */
    SocketManager::makeSocketNonBlocking(infd);
    SocketManager::epollCtlSocket(infd, EPOLL_CTL_ADD) ;
    requests.insert(std::make_pair(infd, Request(infd))) ;
}

void Multiplex::handleRequest( Request& request )
{
    /**
     * We have a notification on the connection socket meaning there is more data to be read
    */
    ssize_t bytesReceived; // number of bytes read returned
    char buf[R_SIZE] = {0}; // read buffer
    SOCKET fd = request.getSocketFD() ;
    /**
     * NOTE: make sure the buf is null-terminated
    */
    bytesReceived = read (fd, buf, sizeof(char) * R_SIZE - 1);
    if (bytesReceived == -1)
    {
        perror ("read");
        /* Closing the descriptor will make epoll remove it
            from the set of descriptors which are monitored. */
        request.state = Request::WAIT_CLOSE ;
    }
    else if (bytesReceived == 0)
    {
        /* End of file. The remote has closed the
            connection. */
        printf ("Closed connection on descriptor %d by client\n", fd);

        /* Closing the descriptor will make epoll remove it
            from the set of descriptors which are monitored. */
        request.state = Request::WAIT_CLOSE ;
    }
    // requests.find(fd)->second.body += buf ;
    /* Write the buffer to standard output */
    std::cout << FOREGRN ;
    std::cout << "============== Request ==============" << std::endl ;
    std::cout << "==============+++++++++==============" << std::endl ;
    write (1, buf, bytesReceived);
    std::cout << "==============+++++++++==============" << std::endl ;
    std::cout << "==============+++++++++==============" << std::endl ;
    std::cout << RESETTEXT ;
    // if (s == -1) // this is only for debug purpose
    // {
    //     perror ("write");
    //     throw std::runtime_error("Could not write in ") ;
    // }
    
    /**
     * Set connection socket to EPOLLOUT to write reponse in the next iteration
     * don't forget that if you didnt set the connection to EPOLLOUT the program
     * wont send your response and keep waiting for EPOLLIN
    */
    if (request.state < Request::FINISHED)
        request.parse(buf) ;
}

void Multiplex::start( void )
{
    int s;
    std::map<int, std::string> eventName ;

    eventName[EPOLLIN] = "EPOLLIN" ;
    eventName[EPOLLET] = "EPOLLET" ;
    eventName[EPOLLOUT] = "EPOLLOUT" ;
    eventName[EPOLLERR] = "EPOLLERR" ;
    eventName[EPOLLHUP] = "EPOLLHUP" ;
    /* The event loop */
    while (1)
    {
        int eventCount ; // Number of events epoll_wait returned

        eventCount = epoll_wait (epollFD, events, SOMAXCONN, -1); // Waiting for new event to occur
        // std::cout << eventCount << " events ready" << std::endl ;
        for (int i = 0; i < eventCount; i++)
        {
            // std::cout << "descriptor " << events[i].data.fd << " " ;
            // if (events[i].events & EPOLLOUT)
            //     std::cout << eventName[EPOLLOUT] ;
            // if (events[i].events & EPOLLIN)
            //     std::cout << eventName[EPOLLIN] ;
            // if (events[i].events & EPOLLET)
            //     std::cout << eventName[EPOLLET] ;
            // if (events[i].events & EPOLLERR)
            //     std::cout << eventName[EPOLLERR] ;
            // if (events[i].events & EPOLLHUP)
            //     std::cout << eventName[EPOLLHUP] ;
            // std::cout << std::endl ;
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP))
            {
                /* An error has occured on this fd, or the socket is not
                    ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                perror("EPOLLERR | EPOLLHUP") ;
                continue;
            }
            else if (listeners.find(events[i].data.fd) != listeners.end()) // Check if socket belong to a server
            {
                /* We have a notification on the listening socket, which
                    means one or more incoming connections. */
                newConnection(events[i].data.fd) ;
                continue;
            }
            else 
            {
                if (events[i].events & EPOLLIN) // check if we have EPOLLIN (connection socket ready to read)
                {
                    requests_t::iterator currRequest = requests.find(events[i].data.fd) ;
                    if (currRequest == requests.end())
                        throw std::runtime_error("request not found in requests map") ;

                    handleRequest(currRequest->second) ;
                }
                if (events[i].events & EPOLLOUT) // check if we have EPOLLOUT (connection socket ready to write)
                {
                    requests_t::iterator currRequest = requests.find(events[i].data.fd) ;
                    if (currRequest->second.state == Request::ERROR)
                    {
                        std::string response("HTTP/1.1 ") ;
                        response += currRequest->second.statusCode ;
                        response += " " ;
                        response += "Error Message Here" ;
                        response += "\r\nConnection: close\r\n\r\n" ;
                        currRequest->second.state = Request::WAIT_CLOSE ;
                    }
                    if (currRequest->second.state == Request::FINISHED)
                    {
                        std::string response("HTTP/1.1 200 OK\r\nContent-Length: 1042\r\nContent-Type: text/xml; charset=UTF-8\r\n\r\n<?xml version=\"1.0\" encoding=\"UTF-8\"?><animalKingdom><kingdom name=\"Animalia\"><phylum name=\"Chordata\"><class name=\"Mammalia\"><animal name=\"Lion\" habitat=\"Savanna\" lifespan=\"10-15 years\"><diet type=\"carnivore\"><prey>Gazelle, Zebra, Antelope</prey></diet><lifeCycle stages=\"Cub, Adolescent, Adult\" gestation=\"4 months\"/></animal><animal name=\"Elephant\" habitat=\"Savanna, Rainforest\" lifespan=\"60-70 years\"><diet type=\"herbivore\"><food>Leaves, Fruits, Bark</food></diet><lifeCycle stages=\"Calf, Adolescent, Adult\" gestation=\"22 months\"/></animal></class><class name=\"Aves\"><animal name=\"Eagle\" habitat=\"Mountains, Forests\" lifespan=\"20-30 years\"><diet type=\"carnivore\"><prey>Rodents, Fish, Rabbits</prey></diet><lifeCycle stages=\"Egg, Chick, Fledgling, Adult\" incubation=\"35 days\"/></animal><animal name=\"Penguin\" habitat=\"Antarctica\" lifespan=\"15-20 years\"><diet type=\"piscivore\"><food>Fish, Krill, Squid</food></diet><lifeCycle stages=\"Egg, Chick, Juvenile, Adult\" incubation=\"53-59 days\"/></animal></class></phylum></kingdom></animalKingdom>\n") ;
                        s = write (events[i].data.fd, response.c_str(), response.size());
                        if (s == -1)
                            throw std::runtime_error("Cant write response") ;
                        // std::cout << FOREBLU ;
                        // std::cout << "============== Response ==============" << std::endl ;
                        // std::cout << "==============++++++++++==============" << std::endl ;
                        // write (1, response.c_str(), response.size());
                        // std::cout << "==============+++++++++==============" << std::endl ;
                        // std::cout << "==============+++++++++==============" << std::endl ;
                        // std::cout << RESETTEXT ;
                        currRequest->second.state = Request::WAIT_CLOSE ;
                    }
                    if (currRequest->second.state == Request::WAIT_CLOSE)
                    {
                        printf ("Closed connection on descriptor %d\n",
                                events[i].data.fd);
                        /* Closing the descriptor will make epoll remove it
                            from the set of descriptors which are monitored. */
                        close (events[i].data.fd);
                        requests.erase(events[i].data.fd) ;
                        continue ; // avoid using request fd after close
                    }
                }
            }
        }
    }
    // close (sfd);
}


