#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dhcp.h"

int
main (int argc, char *argv[])
{
  int sockfd;
  int size;
  unsigned char array[3000];
  struct sockaddr_in sendaddr;
  struct sockaddr_in recvaddr;
  int numbytes;
  socklen_t addr_len;
  int broadcast = 1;

  if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
      perror ("socket");
      exit (1);
    }
  else
    {
      printf ("Socket Created\n");
    }
  // Bind the socket to a specific interface 
  if (setsockopt (sockfd, SOL_SOCKET, SO_BINDTODEVICE, "eno1", 4) < 0)
    {
      perror ("SetSockop-bind-eno1:");
      exit (1);
    }
  else
    printf ("Bind socket to the interface eno1\n");
  // Set Broadcast Option to the socket
  if (setsockopt
      (sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) < 0)
    {
      perror ("SetSockop-broadcast:");
      exit (1);
    }
  else
    printf ("Set broadcast Option to the socket\n");

  memset (&sendaddr, 0, sizeof sendaddr);
  sendaddr.sin_family = AF_INET;
  sendaddr.sin_port = htons (68);
  sendaddr.sin_addr.s_addr = INADDR_BROADCAST;

  memset (&recvaddr, 0, sizeof recvaddr);
  recvaddr.sin_family = AF_INET;
  recvaddr.sin_port = htons (67);
  recvaddr.sin_addr.s_addr = INADDR_ANY;
  if (bind (sockfd, (struct sockaddr *) &recvaddr, sizeof recvaddr) < 0)
    {
      perror ("bind:");
      exit (1);
    }
  else
    printf ("Socket Binded\n");
  // Main Loop
  while (1)
    {
      if (recvfrom
	  (sockfd, array, sizeof array, 0, (struct sockaddr *) &recvaddr,
	   &addr_len) < 0)
	{
	  perror ("recvfrom Discover:");
	  exit (1);
	}
      else
	printf ("Discover Recieved\n");

      int size = DHCP_offer (array);
      if (sendto
	  (sockfd, array, size, 0, (struct sockaddr *) &sendaddr,
	   sizeof sendaddr) < 0)
	{
	  perror ("Sendto-Offer:");
	  exit (1);
	}
      else
	printf ("DHCP offer sent\n");

      if (recvfrom
	  (sockfd, array, sizeof array, 0, (struct sockaddr *) &recvaddr,
	   &addr_len) < 0)
	{
	  perror ("rcvfrom-DCHPRequest:");
	  exit (1);
	}
      else
	printf ("Request Recieved\n");

      // Fill the DHCPACK
      size = DHCP_ACK (array);
      if (sendto
	  (sockfd, array, size, 0, (struct sockaddr *) &sendaddr,
	   sizeof sendaddr) < 0)
	{
	  perror ("Sendto-DHCPACK:");
	  exit (1);
	}
      else
	printf ("DHCPACK sent\n");
    }
  close (sockfd);
  return 0;
}
