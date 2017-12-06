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

#define SERVERPORT "69"		// the port users will be connecting to

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
  int i, tries = 5;
  u_short opcode;
  int filename_len;
  int mode_len;
  int rq_len;
  int difference = 0;
  int snd_bytes;
  int read_bytes = 512;
  int ack_len;
  int rcv_bytes = 516;
  u_short num_block = 0;
  FILE *rcv_file, *snd_file;

  packet_t packet;
  data_packet_t data_packet;
  ack_packet_t ack_packet;
  u_char rcv_buffer[PACKET_SIZE];
  u_char snd_buffer[PACKET_SIZE];
  // char s[INET6_ADDRSTRLEN];
  if (argc != 5)
    {
      fprintf (stderr, "usage: ./client hostname OPCODE filename mode\n");
      exit (1);
    }
  memset (&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  if ((rv = getaddrinfo (argv[1], SERVERPORT, &hints, &servinfo)) != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
      return 1;
    }
  // loop through all the results and make a socket
  for (p = servinfo; p != NULL; p = p->ai_next)
    {
      if ((sockfd =
	   socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
	{
	  perror ("talker: socket");
	  continue;
	}
      break;
    }
  if (p == NULL)
    {
      fprintf (stderr, "talker: failed to create socket\n");
      return 2;
    }
  //Don't need this anymore
  freeaddrinfo (servinfo);
  filename_len = strlen (argv[3]);
  mode_len = strlen (argv[4]);
  rq_len = filename_len + mode_len + 4;	//rq header len
  //Sender options
  switch (atoi (argv[2]))
    {
    case 1:			//RRQ packet
      rcv_file = fopen (argv[3], "ab+");
      packet.opcode = 256;	// Opcode in Big-Endian (Network byte order).This is the same as htons(1)
      //Put the filename on the array
      memcpy (packet.data, argv[3], filename_len);
      //put the 0 pad
      packet.data[filename_len] = 0;
      //put the mode
      memcpy (packet.data + filename_len + 1, argv[4], filename_len);
      //put the 0 pad
      packet.data[filename_len + mode_len + 1] = 0;
      //Convert the packet to a sendable array
      packet_to_array (snd_buffer, &packet);
      if ((snd_bytes =
	   sendto (sockfd, snd_buffer, rq_len, 0, p->ai_addr,
		   p->ai_addrlen)) == -1)
	{
	  perror ("talker: sendto");
	  exit (1);
	}
      //Try 15 times
      //main loop
      while (rcv_bytes == 516)
	{
	  num_block++;
	  while (tries > 0)
	    {
	      //Wait for the server response 0.5 s
	      if (recvfromTimeOut (sockfd, 1, 0))
		{
		  //Recieve the data
		  if ((rcv_bytes =
		       recvfrom (sockfd, rcv_buffer, PACKET_SIZE, 0,
				 p->ai_addr, &p->ai_addrlen)) == -1)
		    {
		      perror ("recvfrom");
		      exit (1);
		    }
		  printf ("Data packet arrived from server\n");
		  opcode = array_to_data_packet (rcv_buffer, &data_packet);
		  printf ("    OPCODE: %u  ", opcode >> 8);
		  printf ("RECV bytes: %d  ", rcv_bytes);
		  printf ("NUMBLOCK: %u\n", data_packet.num_block >> 8);
		  printf ("    Num_block of ack: %u\n", num_block);
		  //write the data
		  fwrite (data_packet.data, sizeof (u_char),
			  rcv_bytes - NUMBLOCK_SIZE - OPCODE_SIZE, rcv_file);

		  //chack if the num_blocks are equal
		  //if ((num_block) != (data_packet.num_block >> 8))
		  if ((htons (num_block)) != (data_packet.num_block))
		    {
		      printf ("    Error, num_blocks arent equal\n");
		      // Reduce the number of the block for handling
		      num_block--;
		      //Do not send the ACK
		      continue;
		    }

		  // Fill the ack 
		  ack_packet.opcode = 1024;	// htons(4) = 1024
		  ack_packet.num_block = htons (num_block);	// Convert the endian
		  // Convert the packet to the array format
		  ack_packet_to_array (snd_buffer, &ack_packet);
		  // Send the ACK
		  if ((snd_bytes =
		       sendto (sockfd, snd_buffer, ACK_PACKET_SIZE, 0,
			       p->ai_addr, p->ai_addrlen)) == -1)
		    {
		      perror ("talker: sendto");
		      exit (1);
		    }
		  printf
		    ("    Client: ack packet send, waiting for answer\n");
		  tries = 15;
		  //if there are no problems, break the try cycle
		  break;
		}
	      //if we are here there are problems(timeout or different num_block)
	      printf ("Timed out. Resending the packet\n");
	      printf ("Tries left %d\n", tries);
	      tries--;
	    }
	  if (tries == 0)
	    break;

	}
      //The transfer has finaliz
      printf ("Final packet Recieved, num_block: %u\n", num_block);
      fclose (rcv_file);
      break;
    case 2:			//WRQ packet
      rcv_bytes = 4;
      snd_file = fopen (argv[3], "rb");	//Opening the file to be transfered
      packet.opcode = 512;	// Opcode in Big-Endian (Network byte order).This is the same as htons(2)
      //Put the filename on the array
      memcpy (packet.data, argv[3], filename_len);
      //put the 0 pad
      packet.data[filename_len] = 0;
      //put the mode
      memcpy (packet.data + filename_len + 1, argv[4], filename_len);
      //put the 0 pad
      packet.data[filename_len + mode_len + 1] = 0;
      //Convert the packet to a sendable array
      packet_to_array (snd_buffer, &packet);
      if ((snd_bytes =
	   sendto (sockfd, snd_buffer, rq_len, 0, p->ai_addr,
		   p->ai_addrlen)) == -1)
	{
	  perror ("talker: sendto");
	  exit (1);
	}
      printf ("WRQ packet sended to server\n");
      //Try 15 times
      //main loop
      while (rcv_bytes == 4 && read_bytes == 512)
	{
	  // if (read_bytes < 512)
	  //     St_flag;
	  while (tries > 0)
	    {
	      //Wait for the server response 0.5 s
	      if (recvfromTimeOut (sockfd, 1, 500000))
		{
		  //Recieve the ack
		  if ((rcv_bytes =
		       recvfrom (sockfd, rcv_buffer, ACK_PACKET_SIZE, 0,
				 p->ai_addr, &p->ai_addrlen)) == -1)
		    {
		      perror ("recvfrom");
		      exit (1);
		    }
		  printf ("ACK packet arrived from server\n");
		  opcode = array_to_ack_packet (rcv_buffer, &ack_packet);
		  printf ("    OPCODE: %u  ", opcode >> 8);
		  printf ("RECV bytes: %d  ", rcv_bytes);
		  printf ("NUMBLOCK of the recieved ACK packet: %u\n",
			  ntohs (ack_packet.num_block));

		  //chack if the num_blocks are equal
		  if ((htons (num_block)) != (ack_packet.num_block))
		    {
		      printf ("    Error, num_blocks arent equal\n");
		      // Reduce the number of the block for handling
		      num_block--;
		      //Do not send the ACK
		      continue;
		    }
		  //Read the data
		  read_bytes =
		    fread (data_packet.data, sizeof (u_char), DATA_SIZE,
			   snd_file);
		  num_block++;
		  data_packet.opcode = 768;	// htons(3) = 1024s
		  data_packet.num_block = htons (num_block);	// Convert the endian to nework byte order(Big Endian)
		  // Convert the packet to the array format
		  data_packet_to_array (snd_buffer, &data_packet);
		  // Send the DATA
		  difference = DATA_SIZE - read_bytes;
		  if ((snd_bytes =
		       sendto (sockfd, snd_buffer, PACKET_SIZE - difference,
			       0, p->ai_addr, p->ai_addrlen)) == -1)
		    {
		      perror ("talker: sendto");
		      exit (1);
		    }
		  printf
		    ("    Client: data packet sended with numblock:%u , waiting for server's ACK\n",
		     ntohs (data_packet.num_block));
		  tries = 5;
		  //if there are no problems, break the try cycle
		  break;
		}
	      //if we are here there are problems(timeout or different num_block)
	      printf ("Timed out. Resending the packet\n");
	      printf ("Tries left %d\n", tries);
	      tries--;
	    }
	  if (tries == 0)
	    break;


	}
      //The transfer has finalized
      printf ("Final packet sended, num_block: %u\n", num_block);
      fclose (snd_file);
      break;


    default:
      printf ("Error in WRQ-RRQ\n");
    }
  close (sockfd);
  return 0;
}
