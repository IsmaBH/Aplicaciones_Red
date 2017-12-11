#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include "http.h"
//start server
void
startServer (char *port)
{
  struct addrinfo hints, *res, *p;

  // getaddrinfo for host
  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_INET;	// IPv4
  hints.ai_socktype = SOCK_STREAM;	// TCP
  hints.ai_flags = AI_PASSIVE;	// Use my ip
  if (getaddrinfo (NULL, port, &hints, &res) != 0)
    {
      perror ("getaddrinfo");
      exit (1);
    }
  // Create socket and bind
  for (p = res; p != NULL; p = p->ai_next)
    {
      sockfd = socket (p->ai_family, p->ai_socktype, 0);
      if (sockfd == -1)
	continue;
      if (bind (sockfd, p->ai_addr, p->ai_addrlen) == 0)
	break;
    }
  if (p == NULL)
    {
      perror ("socket or bind");
      exit (1);
    }
  freeaddrinfo (res);
  // Don't need this anymore


  // listen for incoming connections
  if (listen (sockfd, 100) != 0)
    {
      perror ("listen() error");
      exit (1);
    }
}

//client connection
void
respond (int n)
{
  char mesg[99999], *reqline[3], data_to_send[MAXBYTES], path[99999];
  int rcvd, fd, bytes_read;
  // Initialize the array with 0
  memset ((void *) mesg, (int) '\0', 99999);
  // Recieve data from the client and put int the mesg
  rcvd = recv (clients[n], mesg, 99999, 0);
  if (rcvd < 0)
    fprintf (stderr, ("recv\n"));
  else if (rcvd == 0)		// receive socket closed
    fprintf (stderr, "Client disconnected\n");
  else				// message received
    {
      printf ("%s", mesg);
      reqline[0] = strtok (mesg, " \t\n");
      if (strncmp (reqline[0], "GET\0", 4) == 0)
	{
	  reqline[1] = strtok (NULL, " \t");
	  reqline[2] = strtok (NULL, " \t\n");
	  if (strncmp (reqline[2], "HTTP/1.0", 8) != 0
	      && strncmp (reqline[2], "HTTP/1.1", 8) != 0)
	    {
	      write (clients[n], "HTTP/1.0 400 Bad Request\n", 25);
	    }
	  else
	    {
	      if (strncmp (reqline[1], "/\0", 2) == 0)
		reqline[1] = "/index.html";	// Default file

	      strcpy (path, ROOT);
	      strcpy (&path[strlen (ROOT)], reqline[1]);
	      printf ("Requested file: %s\n", path);

	      // If the file was found
	      if ((fd = open (path, O_RDONLY)) != -1)
		{
		  // Send the ok message to the client
		  send (clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
		  // Read the data on 1024-byte chunks and write it into the sockfd
		  while ((bytes_read = read (fd, data_to_send, MAXBYTES)) > 0)
		    write (clients[n], data_to_send, bytes_read);
		}
	      else
		write (clients[n], "HTTP/1.0 404 Not Found\n", 23);
	    }
	}
    }

  //Closing SOCKET
  shutdown (clients[n], SHUT_RDWR);
  //All further send and recieve operations are DISABLED...
  close (clients[n]);
  // The slot for a client is available
  clients[n] = -1;
}
