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
            return;
        }

        /* File Descriptor/Socket Descriptor created based on the serverInfo - socket() */
        if ((this->socketFileDescriptor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0)
        {
            // maybe try to implement exception with strerror(errno), error string may be useful
            throw bahiart::NetworkManager::SocketException("Couldn't create file descriptor - socket()");
            return;
        }

        return;
    }

    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during socket creation or address setup ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at setupAddress()" << std::endl;
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
            // maybe try to implement exception with strerror(errno), error string may be useful
            throw bahiart::NetworkManager::SocketException("Couldn't connect to remote host - connect()");
            return;
        }
        return;
    }

    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during socket connection ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at openConnection()" << std::endl;
        return;
    }

    return;
}

void bahiart::NetworkManager::TcpSocket::sendMessage(std::string message)
{
    try
    {
        if (send(this->socketFileDescriptor, message.c_str(), strlen(message.c_str()), 0) < 0)
        {
            // maybe try to implement exception with strerror(errno), error string may be useful
            throw SocketException("Couldn't send the message to the server - send()");
            return;
        }
    }

    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during socket messaging ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at sendMessage()" << std::endl;
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
        hints.ai_socktype = SOCK_DGRAM; // TCP

        /* Getting server info from the initial addrinfo (hints), HOST/IP and port - getaddrinfo() */
        if ((getAddrStatus = getaddrinfo(HOST_NAME.c_str(), PORT.c_str(), &hints, &(this->serverInfo))) != 0)
        {
            // tests may be needed for port length ---> gai_strerror() exceptions not clear
            throw bahiart::NetworkManager::SocketException(gai_strerror(getAddrStatus));
            return;
        }

        /* File Descriptor/Socket Descriptor created based on the serverInfo - socket() */
        if ((this->socketFileDescriptor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0)
        {
            // maybe try to implement exception with strerror(errno), error string may be useful
            throw bahiart::NetworkManager::SocketException("Couldn't create file descriptor - socket()");
            return;
        }

        return;
    }

    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during socket creation or address setup ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at setupAddress()" << std::endl;
        return;
    }
}

void bahiart::NetworkManager::UdpSocket::sendMessage(std::string message){

    try
    {
        if (sendto(socketFileDescriptor, message.c_str(), strlen(message.c_str()), 0, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
        {
            // maybe try to implement exception with strerror(errno), error string may be useful
            throw SocketException("Couldn't send the message to the server - sendto()");
            return;
        }
    }

    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during socket messaging ---> " << exception.what() << std::endl;
        return;
    }

    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at sendMessage()" << std::endl;
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