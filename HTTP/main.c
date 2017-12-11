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


int
main (int argc, char *argv[])
{
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  char c;
  int i;

  //Default Values PATH=pwd and PORT=10000
  char PORT[6];
  // The root is the current working directory
  ROOT = getenv ("PWD");
  strcpy (PORT, "10000");

  int slot = 0;

  //Parsing the command line arguments
  while ((c = getopt (argc, argv, "p:d:")) != -1)
    switch (c)
      {
      case 'd':
	ROOT = malloc (strlen (optarg));
	strcpy (ROOT, optarg);
	break;
      case 'p':
	strcpy (PORT, optarg);
	break;
      case '?':
	fprintf (stderr, "Usage ./main [-p port] [-d rootDirectory]\n");
	exit (1);
      default:
	exit (1);
      }

  printf ("Server started at port: %s with root directory: %s\n", PORT, ROOT);
  // Setting all elements to -1: signifies there is no client connected
  for (i = 0; i < CONNMAX; i++)
    clients[i] = -1;

  startServer (PORT);

  // main loop (Accept Connections)
  while (1)
    {
      addrlen = sizeof (clientaddr);
      // Add the new fd to the array
      clients[slot] =
	accept (sockfd, (struct sockaddr *) &clientaddr, &addrlen);
      if (clients[slot] < 0)
	error ("accept() error");
      else
	{
	  printf ("Got connection from %s\n",
		  inet_ntoa (clientaddr.sin_addr));
	  if (fork () == 0)
	    {
	      respond (slot);
	      exit (0);
	    }
	}

      while (clients[slot] != -1)
	slot = (slot + 1) % CONNMAX;
    }

  return 0;
}
