#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string.h>
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>

/*UNIX dependencies*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/poll.h>



namespace bahiart::NetworkManager
{   
    /* General socket exception class -> is able to concatenate errors from UNIX socket's functions (errno) */
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
                std::memset(*resultStr, 0, strlen(*resultStr));

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
        std::vector<char> buffer;
        int socketFileDescriptor{};
        struct addrinfo *serverInfo{};

        virtual bool checkMessages() = 0;

    public:
        virtual void setupAddress(const std::string HOST_NAME, const std::string PORT) = 0;

        virtual void openConnection() = 0;

        virtual void sendMessage(std::string message) = 0;

        virtual bool receiveMessage() = 0;

        inline virtual std::string getMessage() = 0;
        
        /* '= 0' along with an empty destructor function body would be required for pure virtual*/
        virtual ~Socket() {};
    };
}

#endif