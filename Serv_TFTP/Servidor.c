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

#define PORT "69"		// the port users will be connecting to

// get sockaddr, IPv4 or IPv6:
void *
get_in_addr (struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
    {
      return &(((struct sockaddr_in *) sa)->sin_addr);
    }
  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int
main (int argc, char *argv[])
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int opcode;
  int yes = 1;			//for reuse addrs
  int wr_bytes;
  u_short num_block = 0;
  FILE *snd_file, *rcv_file;
  int rcv_bytes;
  int tries = 15;
  int snd_bytes;
  int read_bytes = DATA_SIZE;
  int difference;
  int snd_filename_len;
  packet_t packet;
  data_packet_t data_packet;
  ack_packet_t ack_packet;
  struct sockaddr_storage their_addr;
  u_char rcv_buffer[PACKET_SIZE];
  u_char snd_buffer[PACKET_SIZE];
  socklen_t addr_len;
  char s[INET6_ADDRSTRLEN];
  memset (&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;	// set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;	// use my IP
  if ((rv = getaddrinfo (NULL, PORT, &hints, &servinfo)) != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
      return 1;
    }
  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next)
    {
      if ((sockfd =
	   socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
	{
	  perror ("Server: socket");
	  continue;
	}
      //for the address in use error message
      setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int));
      if (bind (sockfd, p->ai_addr, p->ai_addrlen) == -1)
	{
	  close (sockfd);
	  perror ("Server: bind");
	  continue;
	}
      break;
    }
  if (p == NULL)
    {
      fprintf (stderr, "Server: failed to bind socket\n");
      return 2;
    }
  freeaddrinfo (servinfo);
  while (1)
    {
      printf ("Server: waiting for connections\n");
      addr_len = sizeof their_addr;
      if ((rcv_bytes =
	   recvfrom (sockfd, rcv_buffer, PACKET_SIZE, 0,
		     (struct sockaddr *) &their_addr, &addr_len)) == -1)
	{
	  perror ("recvfrom");
	  exit (1);
	}
      inet_ntop (their_addr.ss_family,
		 get_in_addr ((struct sockaddr *) &their_addr), s, sizeof s);
      printf ("Server: got packet from %s\n", s);
      printf ("Server: packet is %d bytes long\n", rcv_bytes);
      opcode = array_to_packet (rcv_buffer, &packet);
      printf ("opcode%u\n", opcode >> 8);

      // Ignore
      if (argc == 2)
	if (*argv[1] != 't')
	  continue;
      rcv_bytes = ACK_PACKET_SIZE;
      // Switch the opcode of the packet
      switch (opcode >> 8)
	{
	case 1:		//RRQ packet
	  num_block = 1;
	  printf ("Server: RRQ Recieved\n");
	  snd_file = fopen (packet.data, "rb");
	  printf ("    File:%s\n", packet.data);
	  do
	    {
	      //fill the data packet
	      data_packet.opcode = 768;
	      data_packet.num_block = htons (num_block);
	      //Fill the data of the packet
	      read_bytes =
		fread (data_packet.data, sizeof (u_char), DATA_SIZE,
		       snd_file);
	      data_packet_to_array (snd_buffer, &data_packet);
	      difference = DATA_SIZE - read_bytes;
	      //the try to send and recieve-loop begins
	      while (tries > 0)
		{
		  //send data block
		  if ((snd_bytes =
		       sendto (sockfd, snd_buffer, PACKET_SIZE - difference,
			       0, (struct sockaddr *) &their_addr,
			       addr_len)) == -1)
		    {
		      perror ("Server: sendto");
		      exit (1);
		    }
		  printf ("Data Packet sended to client\n");
		  printf ("    Opcode: %u  ", data_packet.opcode >> 8);
		  printf ("    SND Bytes: %d  ", snd_bytes);
		  printf ("    Numblock: %u\n",
			  ntohs (data_packet.num_block));
		  //wait 1 second for the client 15 times
		  if (recvfromTimeOut (sockfd, 0, 100000))
		    {
		      if ((rcv_bytes =
			   recvfrom (sockfd, rcv_buffer, ACK_PACKET_SIZE, 0,
				     (struct sockaddr *) &their_addr,
				     &addr_len)) == -1)
			{
			  perror ("Server: recvfrom");
			  exit (1);
			}
		      printf ("ACK packet arrived from client\n");
		      opcode = array_to_ack_packet (rcv_buffer, &ack_packet);
		      printf ("    OPCODE: %u  ", opcode >> 8);
		      printf ("    RECV Bytes: %d  ", rcv_bytes);
		      printf ("    Numblock: %u\n",
			      ntohs (ack_packet.num_block));
		      //Check if the num_blocks are equal
		      if ((htons (num_block)) == (ack_packet.num_block))
			{
			  num_block++;
			  tries = 15;
			  break;
			}
		      else
			{
			  printf ("    Error, num_blocks arent equal\n");
			  continue;
			}
		    }
		  else
		    {
		      printf ("Timed out. Resending the packet\n");
		      printf ("Tries left %d\n", tries);
		      tries--;
		    }
		}

	    }
	  while ((rcv_bytes == ACK_PACKET_SIZE) && (read_bytes == DATA_SIZE));
	  printf ("Transfer Ended\n");
	  fclose (snd_file);
	  break;





	case 2:		//WRQ packet
	  printf ("Server: WRQ Recieved\n");
	  // In the WRQ packet the servers respons with an ACK with num_block = 0
	  // num_block = 0;
	  rcv_file = fopen (packet.data, "wb+");
	  printf ("    File to be writed:%s\n", packet.data);
	  num_block = 0;
	  // printf("start\n");
	  do
	    {
	      while (tries > 0)
		{

		  //Fill the ack packet
		  ack_packet.opcode = 1024;

		  ack_packet.num_block = htons (num_block);
		  ack_packet_to_array (snd_buffer, &ack_packet);
		  //send ack packet
		  printf ("sockfd\n");
		  if ((snd_bytes =
		       sendto (sockfd, snd_buffer, ACK_PACKET_SIZE, 0,
			       (struct sockaddr *) &their_addr,
			       addr_len)) == -1)
		    {
		      perror ("Server: sendto");
		      exit (1);
		    }
		  printf ("ACK Packet sended to client\n");
		  printf ("    Opcode: %u  ", ack_packet.opcode >> 8);
		  printf ("    SND Bytes: %d  ", snd_bytes);
		  printf ("    Numblock: %u\n", ntohs (ack_packet.num_block));

		  //wait 1 second for the client 15 times 
		  if (recvfromTimeOut (sockfd, 0, 100000))
		    {
		      if ((rcv_bytes =
			   recvfrom (sockfd, rcv_buffer, PACKET_SIZE, 0,
				     (struct sockaddr *) &their_addr,
				     &addr_len)) == -1)
			{
			  perror ("Server: recvfrom");
			  exit (1);
			}
		      printf ("DATA packet arrived from client\n");
		      opcode =
			array_to_data_packet (rcv_buffer, &data_packet);
		      printf ("    OPCODE: %u  ", opcode >> 8);
		      printf ("    RECV Bytes: %d  ", rcv_bytes);
		      printf ("    Numblock: %u\n",
			      ntohs (data_packet.num_block));
		      if (htons (num_block + 1) == data_packet.num_block)
			{
			  difference = rcv_bytes - 4;
			  wr_bytes =
			    fwrite (data_packet.data, sizeof (u_char),
				    difference, rcv_file);
			  num_block++;
			  tries = 15;
			  printf ("wr_bytes: %d\n", wr_bytes);
			  break;
			}
		      printf ("    Error, num_blocks arent equal\n");
		      continue;
		    }

		  printf ("Timed out. Resending the packet\n");
		  printf ("Tries left %d\n", tries);
		  tries--;
		}
	    }
	  while (rcv_bytes == PACKET_SIZE);
	  printf ("Transfer Ended\n");
	  fclose (rcv_file);
	  break;
	default:
	  printf ("Error in recieved packet RRQ-WRQ\n");
	}
    }
  close (sockfd);
  return 0;
}
