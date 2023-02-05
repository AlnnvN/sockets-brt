#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <string.h>
#include <iostream>
#include <vector>
#include <memory>

/*UNIX dependencies*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/poll.h>



namespace bahiart::NetworkManager
{   
    class SocketException : std::exception
    {
    private:
        const char *description{};
        int errNum{};

    public:
        SocketException(const char *description) : description(description) {}

        /* Overload that receives error description and the just set global variable 'errno' by the failed function */
        SocketException(const char *description, int errNum) : description(description), errNum(errNum) {}

        const char *what() const throw()
        {
            /* Ensures errno was set by a failed function and gets it's error string */
            if (errNum != (int){} && errNum != 22)
            {
                /* Initializes a empty array, big enough to hold message, respective error string, and formatting spaces */
                auto resultStr = std::make_shared<char *>(new char[strlen(description) + strlen(strerror(this->errNum)) + 5]);
                memset(*resultStr, 0, strlen(*resultStr));

                /* Concatenates message, formatting, and error string from errno */
                strcpy(*resultStr, description);
                strcat(*resultStr, " -> ");
                strcat(*resultStr, strerror(this->errNum));
                return *resultStr;
            }
            else /* If errno was not defined, returns the description alone */
            {
                return description;
            }
        }
    };

    /* Socket base abstract class */
    class Socket
    {
    protected:
        std::vector<char> buffer{};
        int socketFileDescriptor{};
        struct addrinfo *serverInfo{};

    public:
        virtual void setupAddress(const std::string HOST_NAME, const std::string PORT) = 0;

        virtual void openConnection() = 0;

        virtual void sendMessage(std::string message) = 0;

        virtual bool checkMessages() = 0;

        virtual bool receiveMessage() = 0;

        virtual std::string getBuffer() = 0;
        
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

        /* Check if there is data ready to be received in socket descriptor -> poll() */
        bool checkMessages() override;

        /* Reads messages from the server -> read() */
        bool receiveMessage() override;
        
        /* Returns vector holding the message received from server */
        std::string getBuffer() override;

        /* Closes connection to remote host (socket descriptor) -> close(), and
        frees stored addrinfo's linked tree -> freeaddrinfo() */
        ~TcpSocket() override;
    };

    /* UDP Socket implementation class */
    class UdpSocket : public bahiart::NetworkManager::Socket
    {
    public:
        /* Sets socket object's addrinfo -> getaddrinfo(), file descriptor -> socket(),
        and connects to the server -> connect() */
        void setupAddress(const std::string HOST_NAME, const std::string PORT) override;

        /* Sends message directly using UDP protocol to the stored socket descriptor -> sendto() */
        void sendMessage(std::string message) override;

        /* Check if there is data ready to be received in socket descriptor -> poll() */
        bool checkMessages() override;

        /* Reads messages from the server -> read() */
        bool receiveMessage() override;
        
        /* Returns vector holding the message received from server */
        std::string getBuffer() override;

        /* Frees stored addrinfo's linked tree -> freeaddrinfo */
        ~UdpSocket() override;
        
    private:
        void openConnection() override {};
    };
}

#endif
