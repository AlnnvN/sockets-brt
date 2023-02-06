#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include "socket.hpp"

namespace bahiart::NetworkManager
{   
/* UDP Socket implementation class */
    class UdpSocket : public bahiart::NetworkManager::Socket
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

        /* Sends message directly using UDP protocol to the stored socket descriptor -> sendto() */
        void sendMessage(std::string message) override;

        /* Reads messages from the server -> read() */
        bool receiveMessage() override;
        
        /* Returns string holding the message received from server */
        std::string getMessage() override;

        /* Frees stored addrinfo's linked tree -> freeaddrinfo */
        ~UdpSocket() override;
        
    private:
        /* Forced implementation due to pure virtual base class -> connection is not needed in UDP */
        void openConnection() override {};
    };
}
#endif