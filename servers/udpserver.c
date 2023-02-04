
/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[255];
    const char *hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(argv[1]));

    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
    }

    int n;
    socklen_t len;
    len = sizeof(cliaddr);  //len is value/result

    memset(buffer, 0, 255);
    n = recvfrom(sockfd, buffer, 4, 
                MSG_PEEK, ( struct sockaddr *) &cliaddr,
                &len);
    
    ssize_t bufferlgnt = htonl(*((unsigned int*) buffer)); 
    printf("Here is the length: %ld\n", bufferlgnt);

    n = recvfrom(sockfd, buffer, 255, 
                0, ( struct sockaddr *) &cliaddr,
                &len);


    printf("Here is the message: %s\n",buffer+4);


    memset(buffer, 0, 255);
    unsigned long size = strlen(hello) + 4;
    unsigned long net_size = htonl(size);
    memcpy(buffer, &net_size, 4);
    strcpy(buffer+4, hello);


    n = sendto(sockfd, buffer, strlen(hello) + 4, 
        0, (const struct sockaddr *) &cliaddr,
            len);
    if (n < 0) error("Error on sending");


    close(sockfd);
    return 0; 
}