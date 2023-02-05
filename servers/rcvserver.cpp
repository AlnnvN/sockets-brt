#include <iostream>
#include <string.h>
#include <memory>
#include <vector>

/*UNIX dependencies*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/poll.h>

int main(int argc, char * argv[]){
    std::vector<char> buffer;
    int bytesRead {}; 
    int bufferLength {};
    int theirFileDescriptor{};

    int fileDescriptor{};
    struct addrinfo hints{}, *serverInfo{};
    struct sockaddr_storage theirAddress{};
    socklen_t sin_size;


    memset(&hints, 0 , sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, argv[1], &hints, &serverInfo);

    fileDescriptor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

    bind(fileDescriptor, serverInfo->ai_addr, serverInfo->ai_addrlen);

    listen(fileDescriptor,10);

    sin_size = sizeof(theirAddress);
    theirFileDescriptor = accept(fileDescriptor, (struct sockaddr*)&theirAddress, &sin_size);

    if(theirFileDescriptor == -1){
        std::cout << "accept error";
    }

    buffer.resize(65536);

    recv(theirFileDescriptor, buffer.data(), 4, 0);

    bufferLength = ntohl(*((unsigned int*)buffer.data())); 
        
    while (bytesRead < bufferLength){
        bytesRead += recv(theirFileDescriptor, buffer.data() + bytesRead, bufferLength - bytesRead, 0);
    }

    send(theirFileDescriptor, "FUNCIONA PORRA", bufferLength, 0);   
    std::cout << "Received message: " << buffer.data();

    return 0;
}