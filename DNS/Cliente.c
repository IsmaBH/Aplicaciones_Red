#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "header.h"
#include "handler.h"

#define SERVERPORT "2712" // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int i, tries = 5;
    int rcv_bytes, snd_bytes;
    u_char rcv_buffer[1024];
    u_char snd_buffer[1024];
    unsigned int num_bytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == NULL) 
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return 1;
    }
    num_bytes = argvToArray(argc, argv, snd_buffer);
    //printf("Tam:%u\n", num_bytes);
    //Don't need this anymore
    freeaddrinfo(servinfo);
    //Sender options
    if ((snd_bytes = sendto(sockfd, snd_buffer, num_bytes, 0, p->ai_addr, p->ai_addrlen)) == -1) 
    {
        perror("talker: sendto");
        exit(1);
    }
    // while(tries > 0)
    // {
    //     //Wait for the server response 0.5 s
    //     if (recvfromTimeOut(sockfd, 0, 500000))
    //     {
    //         // Recieve the response
    //         if ((rcv_bytes = recvfrom(sockfd, rcv_buffer, 1024, 0, p->ai_addr, &p->ai_addrlen)) == -1) 
    //         {
    //             perror("recvfrom");
    //             exit(1);
    //         }
    //         // if there are no problems, break the cycle
    //         break;
    //     }
    //     tries--;
    // }
    // close(sockfd);
    return 0;
}
