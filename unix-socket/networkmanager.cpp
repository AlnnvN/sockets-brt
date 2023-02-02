#include "networkmanager.hpp"

void bahiart::NetworkManager::TcpSocket::setupAddress(const std::string HOST_NAME, const std::string PORT){
    int getAddrStatus{};
    struct addrinfo hints{};

    try
    {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM; // TCP

        /* Getting server info from the initial addrinfo (hints), HOST/IP and port - getaddrinfo() */
        if ((getAddrStatus = getaddrinfo(HOST_NAME.c_str(), PORT.c_str(), &hints, &(this->serverInfo))) != 0)
        {
            // tests may be needed for port length ---> gai_strerror() exceptions not clear
            throw bahiart::NetworkManager::SocketException(gai_strerror(getAddrStatus));
        }

        /* File Descriptor/Socket Descriptor created based on the serverInfo - socket() */
        if ((this->socketFileDescriptor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0)
        {
            throw bahiart::NetworkManager::SocketException("Couldn't create file descriptor - socket()", errno);
        }

        return;
    }

    catch (bahiart::NetworkManager::SocketException& exception)
    {
        std::cerr << "SocketException - Error during socket creation or address setup ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception& exception)
    {
        std::cerr << "std::exception - Default exception at setupAddress()" << exception.what() << std::endl;
        return;
    }
}

void bahiart::NetworkManager::TcpSocket::openConnection()
{
    try
    {
        /* Connects to remote host - connect() */
        if (connect(socketFileDescriptor, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
        {
            throw bahiart::NetworkManager::SocketException("Couldn't connect to remote host - connect()", errno);
        }
        return;
    }

    catch (bahiart::NetworkManager::SocketException& exception)
    {
        std::cerr << "SocketException - Error during socket connection ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception& exception)
    {
        std::cerr << "std::exception - Default exception at openConnection() --->" << exception.what() << std::endl;
        return;
    }

    return;
}

void bahiart::NetworkManager::TcpSocket::sendMessage(std::string message)
{
    try
    {
        /* Defines the size of the message buffer as the length of the message per se, 
        plus 4 bytes for the initial unsigned int representing message length */
        const unsigned int bufferLength = message.length() + 4;
        
        /* Encodes the length of the message, from a host unsigned int, to a network one */
        const unsigned int encodedMsgLength = htonl(message.length());

        /* Initializes vector for the message buffer as char type (1 byte per element), and
        resizes it to it's expected length */
        std::vector<char> buffer{};
        buffer.resize(bufferLength);

        /* Ensures that all of message buffer's memory is set to 0 */
        memset(buffer.data(), 0, bufferLength);

        /* Sets first 4 bytes of the buffer to store the encoded message length unsigned int */
        memcpy(buffer.data(), &encodedMsgLength, 4);

        /* Sets the rest of the buffer to store the message received as a parameter, fifth index onwards */
        strcpy(buffer.data() + 4, message.c_str());

        /* Tries to send the message buffer from the just established connection, otherwise, throws an error */
        if (send(this->socketFileDescriptor, buffer.data(), bufferLength, 0) < 0)
            throw SocketException("Couldn't send the message to the server - send()", errno);
        
    }

    catch (bahiart::NetworkManager::SocketException& exception)
    {
        std::cerr << "SocketException - Error during socket messaging ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception& exception)
    {
        std::cerr << "std::exception - Default exception at sendMessage()" << exception.what() << std::endl;
        return;
    }

}

bahiart::NetworkManager::TcpSocket::~TcpSocket()
{
    close(this->socketFileDescriptor); // closes connection to the server
    freeaddrinfo(this->serverInfo);    // getaddrinfo()'s linked tree, used in struct addrinfo *serverInfo, is freed;
}

void bahiart::NetworkManager::UdpSocket::setupAddress(const std::string HOST_NAME, const std::string PORT){
    
    int getAddrStatus{};
    struct addrinfo hints{};

    try
    {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM; // UDP

        /* Getting server info from the initial addrinfo (hints), HOST/IP and port - getaddrinfo() */
        if ((getAddrStatus = getaddrinfo(HOST_NAME.c_str(), PORT.c_str(), &hints, &(this->serverInfo))) != 0)
        {
            // tests may be needed for port length ---> gai_strerror() exceptions not clear
            throw bahiart::NetworkManager::SocketException(gai_strerror(getAddrStatus));
        }

        /* File Descriptor/Socket Descriptor created based on the serverInfo - socket() */
        if ((this->socketFileDescriptor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0)
        {
            throw bahiart::NetworkManager::SocketException("Couldn't create file descriptor - socket()", errno);
        }

        return;
    }

    catch (bahiart::NetworkManager::SocketException& exception)
    {
        std::cerr << "SocketException - Error during socket creation or address setup ---> " << exception.what() << std::endl;
        return;
    }

    catch (const std::exception& exception)
    {
        std::cerr << "std::exception - Default exception at setupAddress() ---> " << exception.what() << std::endl;
        return;
    }
}

void bahiart::NetworkManager::UdpSocket::sendMessage(std::string message){

    try
    {
        /* Defines the size of the message buffer as the length of the message per se, 
        plus 4 bytes for the initial unsigned int representing message length */
        const unsigned int bufferLength = message.length() + 4;
        
        /* Encodes the length of the message, from a host unsigned int, to a network one */
        const unsigned int encodedMsgLength = htonl(message.length());

        /* Initializes vector for the message buffer as char type (1 byte per element), and
        resizes it to it's expected length */
        std::vector<char> buffer{};
        buffer.resize(bufferLength);

        /* Ensures that all of message buffer's memory is set to 0 */
        memset(buffer.data(), 0, bufferLength);

        /* Sets first 4 bytes of the buffer to store the encoded message length unsigned int */
        memcpy(buffer.data(), &encodedMsgLength, 4);

        /* Sets the rest of the buffer to store the message received as a parameter, fifth index onwards */
        strcpy(buffer.data() + 4, message.c_str());

        /* Tries to send the message buffer from the just established connection, otherwise, throws an error */
        if (sendto(socketFileDescriptor, buffer.data(), bufferLength, 0, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
            throw SocketException("Couldn't send the message to the server - sendto()", errno);
    }

    catch (const bahiart::NetworkManager::SocketException& exception)
    {
        std::cerr << "SocketException - Error during socket messaging ---> " << exception.what() << std::endl;
        return;
    }

    catch (const std::exception& e)
    {
        std::cerr << "std::exception - Default exception at sendMessage() ---> " << e.what() << std::endl;
        return;
    }

}

bahiart::NetworkManager::UdpSocket::~UdpSocket(){
    freeaddrinfo(this->serverInfo);    // getaddrinfo()'s linked tree, used in struct addrinfo *serverInfo, is freed;
}

bahiart::NetworkManager::Socket::~Socket()
{
    /* Empty destructor function body required for pure virtual*/
}