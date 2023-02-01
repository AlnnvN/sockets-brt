#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <string.h>
#include <iostream>
#include <memory>

/*UNIX dependencies*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


namespace bahiart
{
    namespace NetworkManager
    {
        class SocketException : std::exception
        {
        private:
            const char *description{};
            int errNum{};

        public:
            SocketException(const char *description) : description(description) {}

            /* Receives error description and the just set global variable 'errno' by the failed function */
            SocketException(const char *description, int errNum) : description(description), errNum(errNum) {}

            const char *what() const throw()
            {
                if(errNum != (int){} && errNum != 22){
                    auto result = std::make_shared<char *>(new char[strlen(description) + strlen(strerror(this->errNum))+1]);
                    strcpy(*result, description);
                    strcat(*result, " -> ");
                    strcat(*result,strerror(this->errNum));
                    return *result;
                }
                else{
                    return description;
                }
            }
        };

        /* Socket base abstract class */
        class Socket
        {
        protected:
            int socketFileDescriptor{};
            struct addrinfo *serverInfo{};

        public:
            virtual void setupAddress(const std::string HOST_NAME, const std::string PORT) = 0;

            virtual void openConnection() = 0;

            virtual void sendMessage(std::string message) = 0;

            /* Empty destructor function body required */
            virtual ~Socket() = 0;
        };

        /* TCP Socket implementation class */
        class TcpSocket : public bahiart::NetworkManager::Socket
        {
        public:
            /* Sets socket object's addrinfo -> getaddrinfo() and the file descriptor -> socket() */
            void setupAddress(const std::string HOST_NAME, const std::string PORT) override;

            /* Establishes TCP connection to the server -> connect() */
            void openConnection() override;

            /* Sends message to the stored socket descriptor -> send() */
            void sendMessage(std::string message) override;

            /* Closes connection to remote host (socket descriptor) -> close(), and
            frees stored addrinfo's linked tree -> freeaddrinfo() */
            ~TcpSocket() override;
        };

        /* UDP Socket implementation class */
        class UdpSocket : public bahiart::NetworkManager::Socket
        {
        public:
            /* Sets socket object's addrinfo -> getaddrinfo() and file descriptor -> socket()*/
            void setupAddress(const std::string HOST_NAME, const std::string PORT) override;

            /* Sends message directly using UDP protocol to the stored socket descriptor -> sendto() */
            void sendMessage(std::string message) override;

            /* Frees stored addrinfo's linked tree -> freeaddrinfo */
            ~UdpSocket() override;
            
        private:
            void openConnection() override {}
        };
    }
}

#endif