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
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char *argv[]){
    std::vector<char> msgBuffer;

    int socketDescriptor, theirFileDescriptor;
    struct addrinfo hints, *serverInfo , *p;
    struct sockaddr_storage theirAddress;
    socklen_t sin_size;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int errnum;
    //GETADDRINFO()
    if((errnum = getaddrinfo(NULL, argv[1], &hints, &serverInfo) != 0)){
        std::cout << "Server -> error getaddrinfo() -> " << gai_strerror(errnum) << std::endl;
    }

    for(p = serverInfo; p != NULL; p = p->ai_next){
        //SOCKET()
        socketDescriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if((socketDescriptor < 0)){
            std::cout << "Server -> error socket() " << std::endl;
            continue;
        }

        //BIND()
        if((bind(socketDescriptor, p->ai_addr, p->ai_addrlen)) == -1){
            std::cout << "Server -> error bind()" << std::endl;
            continue;
        }

        break;
    }
    
    freeaddrinfo(serverInfo);

    if(p == NULL){
        std::cout << "Server -> failed completely to bind" << std::endl;
        exit(1);
    }
    else{
        std::cout << "Server -> binded successfully!" << std::endl;
    }

    //LISTEN()
    if((listen(socketDescriptor, 10)) == -1){
        std::cout << "Server -> error listen()" << std::endl;
    }

    std::cout << "Server -> waiting connections!" << std::endl; 

    //MAIN CONNECTION LOOP
    while(true){
        unsigned int recvLength{};
           
        sin_size = sizeof(theirAddress);

        if((theirFileDescriptor = accept(socketDescriptor, (struct sockaddr *)&theirAddress, &sin_size)) == -1){
            std::cout << "Server -> error accept()" << std::endl;
            continue;
        }
        
        std::cout << "Server -> got connection!" << std::endl;

        //RECV()
        /* receives first four bytes */
        int bytesRead = 0;
        msgBuffer.resize(4);
        if(recv(theirFileDescriptor, msgBuffer.data(), 4, 0) < 4){
            std::cout << "Server -> failed to read the first four bytes (length) of the message - recv()" << std::endl;
        }
        
        recvLength = ntohl(*((unsigned int *)msgBuffer.data()));
        std::cout << "Receive length: " << recvLength << std::endl;
    
        /* clears msgBuffer to receive the message */
        msgBuffer.resize(recvLength, 0);
        memset(msgBuffer.data(), 0, msgBuffer.capacity());

        while(bytesRead < recvLength){
            bytesRead += recv(theirFileDescriptor, msgBuffer.data() + bytesRead, recvLength - bytesRead, 0);
        }
        
        /* copies the message from the msgBuffer to the recv string */
        std::string recvMsg(msgBuffer.begin(), msgBuffer.end());
        
        std::cout << "Server -> received message '" << recvMsg << "' - length: " << recvLength << std::endl;

        /* response (sends back the received message)*/
        std::string sendMsg = "Message received! -> ";
        unsigned int sendLength = sendMsg.length() + msgBuffer.size();
        unsigned long encondedSendLength = htonl(sendLength);

        /* clears msgBuffer to store response */
        msgBuffer.resize(sendLength + 4, 0);
        std::cout << "Buffer capacity: " << msgBuffer.capacity() << std::endl;
        
        memset(msgBuffer.data(), 0, msgBuffer.capacity());

        /* structures response inside msgBuffer */
        memcpy(msgBuffer.data(), &encondedSendLength, 4); //first four bytes (length)
        strcpy(msgBuffer.data() + 4, sendMsg.c_str());
        strcpy(msgBuffer.data() + 4 + sendMsg.length(), recvMsg.c_str());

        for(int i = 0; i < 5; i++){ //send the message i times
            std::cout << "Server -> sending response '" << msgBuffer.data() + 4 << "' - length: " << msgBuffer.size() - 4 << std::endl;

            //SEND()
            if(send(theirFileDescriptor, msgBuffer.data(), sendLength + 4, 0) < sendLength){
                std::cout << "Server -> failed to send response" << std::endl;
                exit(0);
            }

            sleep(1); //delay between messages 
        }
    }
    return 0;
}
