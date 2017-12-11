//Header Files
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
 
int main( int argc , char *argv[])
{
    unsigned char hostname[100];
    unsigned int hostnames_len = 0;
    unsigned short number_of_names;
    // The first argument of the command line
    // strcpy(hostname, argv[1]);
    //Now get the ip of this hostname , A record
    int sockfd;
    char s[INET6_ADDRSTRLEN];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int i, tries = 5;
    int yes =1;
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    int rcv_bytes, snd_bytes;
    u_char rcv_buffer[1024];
    u_char snd_buffer[1024];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, "2712", &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("Server: socket");
            continue;
        }
        //for the address in use error message
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("Server: bind");
            continue;
        }
        break;
    }
    if (p == NULL) 
    {
        fprintf(stderr, "Server: failed to bind socket\n");
        return 1;
    }
    freeaddrinfo(servinfo);
    addr_len = sizeof their_addr;
    while(1)
    {
        printf("Server: Waiting for connections...\n");
        if ((rcv_bytes = recvfrom(sockfd, rcv_buffer, 1024, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
        {
            perror("Server: recvfrom");
            exit(1);
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("Server: got packet from %s\n",s);

        /* Transform the data from the array into variables */
        // Number of names
        memcpy(&number_of_names, rcv_buffer, NUM_NAMES_LEN);
        number_of_names = ntohs(number_of_names);
        // Names
        hostnames_len = NUM_NAMES_LEN + 1;
        for (i = 0; i < number_of_names-2; i++)
        {
            strcpy(hostname, rcv_buffer + hostnames_len);
            printf("%s\n", hostname);
            hostnames_len += strlen(hostname) + 1;
            ngethostbyname(hostname , T_A, "148.204.198.2");
        }
    }
    close(sockfd);
    return 0;
}
