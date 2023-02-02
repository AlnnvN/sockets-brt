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


bool bahiart::NetworkManager::TcpSocket::checkMessages()

{
    /*
    Object that receives data structure describing a polling request.
    */
    struct pollfd ufds;
    /*
    The rv variable will receive the output of poll() function, it could be:
        -1 for error
        0 for no event occurred in socketfiledescriptor during the limit of waiting time
        >0 for the number of events occurred in the socketfiledescriptor

    */ 
    int rv {};

    ufds.fd = this->socketFileDescriptor;
    ufds.events = POLLIN; //Set the type of event that poll() will be waiting to happen

    /*poll() receives 3 parameters: 
        the address of the object that keep the struct of pollfd
        the number of objects that poll() will be following
        limit of waiting time that the function will wait for events (-1 makes it wait forever)
    */
    rv = poll(&ufds, 1, 0); 
    if (rv > 0 & ufds.revents & POLLIN)
    {
        return true;
    }
    else 
    {
        throw bahiart::NetworkManager::SocketException("No message from server.");
        return false;
    }
    
}

bool bahiart::NetworkManager::TcpSocket::receiveMessage()
{
    if (! checkMessages()) {
        return false;
    }
    
    char buffer[65536];

    /*Number of bytes read from the received data.*/
    int bytesRead {};
    /*Total length of the data received.*/
    int bufferLength {};
    
    try {
        if (read(this->socketFileDescriptor, buffer, 4) < 4) {
            throw bahiart::NetworkManager::SocketException("Length of message is less than 4 bytes.");
            return false;
        }

        bufferLength = ntohl(*((unsigned int*) buffer)); //convert data from network to host
        
        /*
        This while function (faithfully) will do the following steps:

        1. Checks if the number of read bytes is minor than the total number of bytes, if it's not, continues the loop

        2. will sum to the number of bytes read the number of bytes received:
                In this step, the read function parameters will be - besides the socketfiledescriptor
                parameter - buffer + the number of read bytes AND the total size of the message minus
                the total of read bytes, i.e: for every loop, the buffer offset will be sumn with
                the read bytes.
                ----> 
                example: if 8 bytes of the message are already read, in the next loop the buffer
                will receive buffer+8 and the message will start to be written in buffer[8], as so
                the size of message that the function will read will be updated every loop, until
                the number of read bytes be equal to the total number of bytes.
                ---->

        3. finally, will check if there is more data do be received, if positive, the loop continues
        */

        while (bytesRead < bufferLength) {
            bytesRead += read(this->socketFileDescriptor, buffer+bytesRead, bufferLength - bytesRead);
            if (!checkMessages())
                return false;
        }

        std::cout << "Message: " << buffer << std::endl; //Debug purposes only
        return true;

    }
    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during receiving messages ---> " << exception.what() << std::endl;
        return false;
    }

    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at receiveMessage()" << std::endl;
        return false;
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
