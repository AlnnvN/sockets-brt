#include "tcpsocket.hpp"

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
        if (connect(this->socketFileDescriptor, this->serverInfo->ai_addr, this->serverInfo->ai_addrlen) < 0)
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
        plus 4 bytes for the initial unsigned int representing message length. */
        const unsigned long bufferLength = message.length() + 4;
        
        /* Encodes the length of the message, from a host unsigned int, to a network one. */
        const unsigned long encodedMsgLength = htonl(message.length());

        /* Initializes vector for the message buffer as char type (1 byte per element), and
        resizes it to it's expected length. */
        this->buffer.clear();
        this->buffer.resize(bufferLength + 1, 0);
        /* +1 to preserve null terminator. */

        /* Sets first 4 bytes of the buffer to store the encoded message length unsigned int. */
        std::memcpy(this->buffer.data(), &encodedMsgLength, 4);

        /* Sets the rest of the buffer to store the message received as a parameter, fifth index onwards. */
        std::memcpy(this->buffer.data() + 4, message.c_str(), strlen(message.c_str()) + 1);

        /* Tries to send the message buffer from the just established connection, otherwise, throws an error. */
        if (send(this->socketFileDescriptor, this->buffer.data(), bufferLength, 0) < 0){
            throw SocketException("Couldn't send the message to the server - send()", errno);
        }
        
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
    ufds.fd = this->socketFileDescriptor;
    ufds.events = POLLIN;
    /* Sets the type of event that poll() will be waiting to happen -> POLLIN stands for normal data */

    /*
    The rv variable will receive the output of poll() function, it could be:
        -1 for error
        0 for no event occurred in socketfiledescriptor during the limit of waiting time
        >0 for the number of events occurred in the socketfiledescriptor
    */ 
    int rv {};

    try {
        /*
        poll() receives 3 parameters: 
            the address of the object that keep the struct of pollfd
            the number of objects that poll() will be following
            limit of waiting time that the function will wait for events (-1 makes it wait forever)
        */
        rv = poll(&ufds, 1, 20); 

        if(rv < 0){
            throw bahiart::NetworkManager::SocketException("Error during checking messages from server --> poll()", errno);
        }
    
        /* Retuns true if poll detects incoming data */
        return (rv > 0 && (ufds.revents && POLLIN));
        //else throw bahiart::NetworkManager::SocketException("No message from server --> poll()"); DEBUG
    }
    catch (bahiart::NetworkManager::SocketException exception)
    {
        std::cout << "SocketException - Error during checking messages ---> " << exception.what() << std::endl;
        return false;
    }

    catch (std::exception& exception)
    {
        std::cerr << "std::exception - Default exception at checkMessage()" << exception.what() << std::endl;
        return false;
    }

}

bool bahiart::NetworkManager::TcpSocket::receiveMessage()
{
    if (!checkMessages())
    {
        return false;
    }

    /*Number of bytes read from the received data.*/
    int bytesRead{};

    /* Total length of the received data. */
    unsigned long bufferLength{};

    try {
        
        /* Cleaning buffer before using */
        this->buffer.clear();
        
        /* Resizing buffer to fit the first four bytes */

        //this->buffer.resize(4);
        //this->buffer.shrink_to_fit();
        /* 
        -> 
        the shkrink to fit is an alternative for reducing buffer capacity to 4, as the
        vector allocated size in memory remains bigger if resized to a greater number
        before, but it has operational costs, since the items are copied and moved to another
        block, and when resized, the same happens. A suggestion woul be start the vector with 4 bytes capacity
        and only increase it when necessary, since our priority would be perfomance over free memory.
        */
    
        /* Checking if the data size of received message is equal/greater than 4 bytes */
        if (recv(this->socketFileDescriptor, this->buffer.data(), 4, 0) < 4){
            throw bahiart::NetworkManager::SocketException("Length of message is less than 4 bytes.");
        }
            
        /* Convert received string length from network to host */
        bufferLength = ntohl(*((unsigned long*) this->buffer.data())); 

        /* Resizing buffer to fit the entire data expected to be received */
        this->buffer.resize(bufferLength + 1, 0);
        /* +1 to preserve null terminator. */

        /*
        This while function (faithfully) will do the following steps:
        1. Checks if the number of read bytes is minor than the total number of bytes and if there is more data to be received, if positive, the loop continues
        
        2. Sum the number of read bytes up to the number it is expected to be received:
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
    catch (bahiart::NetworkManager::SocketException& exception)
    {
        std::cout << "SocketException - Error during receiving messages ---> " << exception.what() << std::endl;
        return false;
    }

    catch (std::exception& exception)
    {
        std::cout << "std::exception - Default exception at receiveMessage()" << std::endl;
        return false;
    }
    
}

inline std::string bahiart::NetworkManager::TcpSocket::getMessage(){
    return (std::string)this->buffer.data();
}

bahiart::NetworkManager::TcpSocket::~TcpSocket()
{
    close(this->socketFileDescriptor); // closes connection to the server
    freeaddrinfo(this->serverInfo);    // getaddrinfo()'s linked tree, used in struct addrinfo *serverInfo, is freed;
}
