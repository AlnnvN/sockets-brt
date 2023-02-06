#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#include "socket.hpp"

namespace bahiart::NetworkManager
{   
    /* TCP Socket implementation class */
    class TcpSocket : public bahiart::NetworkManager::Socket
    {
    protected:
        /* 
        Check if there is data ready to be received in socket descriptor -> poll().
        Is only called inside the object scope by the receiveMessage() function.
        */
        bool checkMessages() override;

    public:
        /* Sets socket object's addrinfo -> getaddrinfo() and the file descriptor -> socket() */
        void setupAddress(const std::string HOST_NAME, const std::string PORT) override;

        /* Establishes TCP connection to the server -> connect() */
        void openConnection() override;

        /* Sends message to the stored socket descriptor -> send() */
        void sendMessage(std::string message) override;

        /* Reads messages from the server -> read() */
        bool receiveMessage() override;
        
        /* Returns string holding the message received from server */
        std::string getMessage() override;

        /* Closes connection to remote host (socket descriptor) -> close(), and
        frees stored addrinfo's linked tree -> freeaddrinfo() */
        ~TcpSocket() override;
    };
}
#endif