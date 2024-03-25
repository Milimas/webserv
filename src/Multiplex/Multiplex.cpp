#include "../../includes/Multiplex.hpp"

#include "../../includes/Request/Http_req.hpp"
SOCKET                      Multiplex::epollFD ;
Multiplex::listeners_t      Multiplex::listeners ;
Multiplex::requests_t       Multiplex::requests ;
Multiplex::epoll_event_t    Multiplex::events[SOMAXCONN] = {} ;
Multiplex::host_port_map_t  Multiplex::hostPortMap ;

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
        SOCKET sfd ;
        std::string id = servIt->getHost() + ":" + servIt->getPort() ;
        std::cerr << "Listening on: " << id << "..." << std::endl ;
        if (hostPortMap.find(id) != hostPortMap.end()) // host:port already binded
            sfd = hostPortMap[id] ;
        else // create new socket, bind, listen and add to epoll finally add it to hostPortMap
        {
            sfd = SocketManager::createSocket(servIt->getHost().c_str(), servIt->getPort().c_str(), AF_INET, SOCK_STREAM, AI_PASSIVE) ;
            SocketManager::makeSocketNonBlocking (sfd);
            SocketManager::startListening(sfd) ;
            SocketManager::epollCtlSocket(sfd, EPOLL_CTL_ADD) ;
            hostPortMap.insert(std::make_pair(id, sfd)) ;
        }
        listeners[sfd] = *servIt ;
        servIt++ ;
    }
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
  
    bool isHeaderSkipped = false ;

    /* The event loop */
    while (1)
    {
        int eventCount ; // Number of events epoll_wait returned
        /// i add this to get some data from config file like clientMaxSize
      
        eventCount = epoll_wait (epollFD, events, SOMAXCONN, -1); // Waiting for new event to occur
        std::cerr << eventCount << " events ready" << std::endl ;
        for (int i = 0; i < eventCount; i++)
        {
            std::cerr << "descriptor " << events[i].data.fd << " " ;
            if (events[i].events & EPOLLOUT)
                std::cerr << eventName[EPOLLOUT] ;
            if (events[i].events & EPOLLIN)
                std::cerr << eventName[EPOLLIN] ;
            if (events[i].events & EPOLLET)
                std::cerr << eventName[EPOLLET] ;
            if (events[i].events & EPOLLERR)
                std::cerr << eventName[EPOLLERR] ;
            if (events[i].events & EPOLLHUP)
                std::cerr << eventName[EPOLLHUP] ;
            std::cerr << std::endl ;

            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP))
            {
                /* An error has occured on this fd, or the socket is not
                    ready for reading (why were we notified then?) */
                // fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                perror("EPOLLERR | EPOLLHUP") ;
                continue;
            }
            else if (listeners.find(events[i].data.fd) != listeners.end()) // Check if socket belong to a server
            {
                /* We have a notification on the listening socket, which
                    means one or more incoming connections. */
                struct sockaddr in_addr;
                socklen_t in_len;
                int infd;
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                in_len = sizeof in_addr;
                infd = accept (events[i].data.fd, &in_addr, &in_len); // Accept connection
                if (!ISVALIDSOCKET(infd))
                {
                    if ((errno == EAGAIN) ||
                        (errno == EWOULDBLOCK))
                    {
                        /* We have processed all incoming
                            connections. */
                        continue ;
                    }
                    else
                    {
                        perror ("accept");
                        continue ;
                    }
                }
                
                s = getnameinfo (&in_addr, in_len,
                                hbuf, sizeof hbuf,
                                sbuf, sizeof sbuf,
                                NI_NUMERICHOST | NI_NUMERICSERV);
                if (s == 0)
                {
                    // printf("Accepted connection on descriptor %d "
                    //         "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                }

                /**
                 * Make the incoming socket non-blocking and add it to the list of fds to monitor. 
                */
                SocketManager::makeSocketNonBlocking(infd);
                SocketManager::epollCtlSocket(infd, EPOLL_CTL_ADD) ;
                requests.insert(std::make_pair(infd, Request(infd, listeners[events[i].data.fd], in_addr))) ;
                continue;
            }
            else if (events[i].events & EPOLLIN) // check if we have EPOLLIN (connection socket ready to read)
            {
                /**
                 * We have a notification on the connection socket meaning there is more data to be read
                */
                ssize_t bytesReceived; // number of bytes read returned
                char buf[R_SIZE] = {0}; // read buffer
                bzero(buf, R_SIZE) ;
                bytesReceived = read (events[i].data.fd, buf, sizeof(char) * R_SIZE - 1);
                if (bytesReceived == -1)
                {
                    perror ("read");
                    // printf ("Closed connection on descriptor %d\n",
                    // events[i].data.fd);

                    /* Closing the descriptor will make epoll remove it
                        from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                    requests.erase(events[i].data.fd) ;
                    continue ;
                }
                else if (bytesReceived == 0)
                {
                    /* End of file. The remote has closed the
                        connection. */
                    // printf ("Closed connection on descriptor %d by client\n",
                    // events[i].data.fd);

                    /* Closing the descriptor will make epoll remove it
                        from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                    requests.erase(events[i].data.fd) ;
                    continue ;
                }
               
                /* Write the buffer to standard output */
                // skip the headers in the buffer
                ssize_t pos = 0 ;
                while (isHeaderSkipped == false && pos < bytesReceived)
                {
                    if (buf[pos] == '\r' && buf[pos + 1] == '\n' && buf[pos + 2] == '\r' && buf[pos + 3] == '\n')
                    {
                        isHeaderSkipped = true ;
                        pos += 4 ;
                        break ;
                    }
                    pos++ ;
                }
                if (isHeaderSkipped)
                {
                    s = write (1, buf + pos, bytesReceived - pos);
                }
                // get 
                //  start parse
                Http_req htt(buf,bytesReceived,listeners);
                // if (s == -1)
                // {
                //     perror ("write");
                //     throw std::runtime_error("Could not write in ") ;
                // }
                
                /**
                 * Set connection socket to EPOLLOUT to write reponse in the next iteration
                 * don't forget that if you didnt set the connection to EPOLLOUT the program
                 * wont send your response and keep waiting for EPOLLIN
                */
                // if (requests.find(events[i].data.fd)->second.request.find("\r\n\r\n") != std::string::npos)
                    // SocketManager::epollCtlSocket(events[i].data.fd, EPOLL_CTL_MOD, EPOLLOUT) ;
            }
            else if (events[i].events & EPOLLOUT) // check if we have EPOLLOUT (connection socket ready to write)
            {
                /**
                 * Set connection socket to EPOLLIN to read another request in the next iteration
                */
                SocketManager::epollCtlSocket(events[i].data.fd, EPOLL_CTL_MOD, EPOLLIN) ;
                std::string response("HTTP/1.1 200 OK\r\nContent-Length: 13\r\nContent-Type: text/html\r\n\r\nHello World!\n") ;
                // std::string response("HTTP/1.1 302 Found\r\nLocation: http://example.com/new-page\r\n\r\n") ; // redirection response
                s = write (events[i].data.fd, response.c_str(), response.size());
                if (s == -1)
                    throw std::runtime_error("Cant write response") ;
                // std::cerr << FOREBLU ;
                // std::cerr << "============== Response ==============" << std::endl ;
             //    std::cerr << "==============++++++++++==============" << std::endl ;
              //   write (1, response.c_str(), response.size());
                // std::cerr << "==============+++++++++==============" << std::endl ;
                // std::cerr << "==============+++++++++==============" << std::endl ;
                // std::cerr << RESETTEXT ;
                /**
                 * Incas the client request Connection: close we close the connection
                 * else the connection remains open and waiting for another rquest from the client
                */
                if (requests.find(events[i].data.fd)->second.request.find("Connection: close") != std::string::npos)
                {
                    // printf ("Closed connection on descriptor %d\n",
                            // events[i].data.fd);
                    /* Closing the descriptor will make epoll remove it
                        from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                    // requests.erase(events[i].data.fd) ;
                }
                /**
                 * clear request buffer after processing it and sending request
                */
                requests.find(events[i].data.fd)->second.request.clear() ;
                std::cerr << "Response Sent" << std::endl ;
            }
        }
    }
    // close (sfd);
}
