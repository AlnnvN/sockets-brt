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
        const std::size_t bufferLength = message.length() + 4;
        
        /* Encodes the length of the message, from a host unsigned int, to a network one */
        const std::size_t encodedMsgLength = htonl(message.length());

        /* Ensures that all of message buffer's memory is set to 0 */
        this->buffer.clear();

        /* Resizes buffer to it's expected length */
        this->buffer.resize(bufferLength);

        /* Sets first 4 bytes of the buffer to store the encoded message length unsigned int */
        memcpy(this->buffer.data(), &encodedMsgLength, 4);

        /* Sets the rest of the buffer to store the message received as a parameter, fifth index onwards */
        strcpy(this->buffer.data() + 4, message.c_str());

        /* Tries to send the message buffer from the just established connection, otherwise, throws an error */
        if (send(this->socketFileDescriptor, this->buffer.data(), bufferLength, 0) < 0)
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

bool bahiart::NetworkManager::TcpSocket::checkMessages()

{
    /*
    Object that holds data structure describing a polling request.
    */
    struct pollfd ufds;
    /*
    The rv variable will receive the output of poll() function, it could be:
        -1 for error
        0 for no event occurred in socketfiledescriptor during the limit of waiting time
        >0 for the number of events occurred in socketfiledescriptor
    */ 
    int rv {};
        try {
        ufds.fd = this->socketFileDescriptor;
        ufds.events = POLLIN; //Set the type of event that poll() will be waiting to happen -> POLLIN stands for normal data

        /*
        poll() receives 3 parameters: 
            the address of the object that keep the struct of pollfd
            the number of objects that poll() will be following
            limit of waiting time that the function will wait for events (-1 makes it wait forever)
        */

        rv = poll(&ufds, 1, 2000); // -> setted the timeout to 2000 only for working with the debug server

        if (rv > 0 && (ufds.revents && POLLIN))
            {   
                return true;
            }
        else 
            {
                //throw bahiart::NetworkManager::SocketException("No message from server --> poll()");
                return false;
            }
    }
    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during checking messages ---> poll()" << exception.what() << std::endl;
        return false;
    }

}

bool bahiart::NetworkManager::TcpSocket::receiveMessage()
{
    if (!checkMessages()) {
        return false;
    }

    /*Number of bytes read from the received data.*/
    int bytesRead {};

    /* Total length of the received data. */
    std::size_t bufferLength {};
    
    try {

        /* Cleaning buffer before using */
        this->buffer.clear();
        
        /* Resizing buffer to fit the first four bytes */
        this->buffer.resize(4);

        /* Checking if the data size of received message is equal/greater than 4 bytes */
        if (recv(this->socketFileDescriptor, this->buffer.data(), 4, 0) < 4)
            throw bahiart::NetworkManager::SocketException("Length of message is less than 4 bytes.");
        
        
        /* Convert received string length from network to host */
        bufferLength = ntohl(*((unsigned int*) this->buffer.data())); 
        
        /* Resizing buffer to fit the entire data expected to be received + null terminator */
        this->buffer.resize(bufferLength + 1);


        /*
        This while function (faithfully) will do the following steps:
        1. Checks if the number of read bytes is minor than the total number of bytes and if there is more data to be received, if positive, the loop continues
        
        2. Sumn to the number of bytes read the number of bytes received:
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
        */
        
        while (bytesRead < bufferLength && checkMessages()) {
            bytesRead += recv(this->socketFileDescriptor, this->buffer.data() + bytesRead, bufferLength - bytesRead, 0);
        }

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

std::string bahiart::NetworkManager::TcpSocket::getBuffer(){
    return (std::string)this->buffer.data();
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
        const std::size_t bufferLength = message.length() + 4;
        
        /* Encodes the length of the message, from a host unsigned int, to a network one */
        const std::size_t encodedMsgLength = htonl(message.length());

        /* Ensures that all of message buffer's memory is set to 0 */
        this->buffer.clear();

        /* Resizes buffer to it's expected length */
        this->buffer.resize(bufferLength);

        /* Sets first 4 bytes of the buffer to store the encoded message length unsigned int */
        memcpy(this->buffer.data(), &encodedMsgLength, 4);

        /* Sets the rest of the buffer to store the message received as a parameter, fifth index onwards */
        strcpy(this->buffer.data() + 4, message.c_str());

        /* Tries to send the message buffer from the just established connection, otherwise, throws an error */
        if (sendto(socketFileDescriptor, this->buffer.data(), bufferLength, 0, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
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

bool bahiart::NetworkManager::UdpSocket::checkMessages()
{
    /*
    Object that holds data structure describing a polling request.
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
    ufds.events = POLLIN; //Set the type of event that poll() will be waiting to happen -> POLLIN stands for normal data

    /*
    poll() receives 3 parameters: 
        the address of the object that keep the struct of pollfd
        the number of objects that poll() will be following
        limit of waiting time that the function will wait for events (-1 makes it wait forever)
    */
    try {
        rv = poll(&ufds, 1, 20); 
        if (rv > 0 && (ufds.revents && POLLIN))
            {   
                return true;
            }
        else 
            {
                throw bahiart::NetworkManager::SocketException("No message from server --> poll()");
            }
    }
    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during checking messages ---> " << exception.what() << std::endl;
        return false;
    }
}

bool bahiart::NetworkManager::UdpSocket::receiveMessage()
{
    if (!checkMessages()) {
        return false;
    } 
    
    /* Struct designed to be large enough to fit both ipv4 and ipv6 structures */
    struct sockaddr_storage addr{};

    /* Type designed to hold length of socket structures. */
    socklen_t fromlen = sizeof addr;

    /* Total length of the received data. */
    std::size_t bufferLength {};
    

    try {  
        
        /* Cleaning buffer before using */
        this->buffer.clear();

        /* Resizing buffer to fit the first 4 bytes */
        this->buffer.resize(4);

        /* Checking if the data size of received message is equal/greater than 4 bytes */
        if (recvfrom(this->socketFileDescriptor, this->buffer.data(), this->buffer.capacity(), MSG_PEEK, (struct sockaddr *)&addr, &fromlen) < 4)
            throw bahiart::NetworkManager::SocketException("Length of message is less than 4 bytes.");   

        /* 
        Clarifying, in the condition above the parameter MSG_PEEK was used
        because UDP doesn't have support to lost packets, so it only sends
        the data one time. MSG_PEEK condition tells the server we are only
        taking a peek inside the data, and we will actually read it later.
        */

        /* Convert received string length from network to host */
        bufferLength = ntohl(*((unsigned int*) this->buffer.data())); 
        std::cout << "\nMessage length: " << bufferLength << std::endl; // only for debug purposes

        /* Resizing buffer to fit the entire message + null terminator */
        this->buffer.resize(bufferLength + 5);
        
        /* Writing the message in buffer vector */
        if (recvfrom(this->socketFileDescriptor, this->buffer.data(), bufferLength, 0, (struct sockaddr *)&addr, &fromlen) < 0)
            throw bahiart::NetworkManager::SocketException("recvfrom()");

        /* Erasing first 4 elements from buffer where length of message is */
        this->buffer.erase(this->buffer.begin(), this->buffer.begin()+4);

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

std::string bahiart::NetworkManager::UdpSocket::getBuffer()
{   
    return (std::string)this->buffer.data();
}

bahiart::NetworkManager::UdpSocket::~UdpSocket(){
    freeaddrinfo(this->serverInfo);    // getaddrinfo()'s linked tree, used in struct addrinfo *serverInfo, is freed;
}

bahiart::NetworkManager::Socket::~Socket()
{
    /* Empty destructor function body required for pure virtual*/
}
