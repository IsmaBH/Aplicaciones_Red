
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define SERVER_IP   "192.168.1.71"
#define SERVER_PORT  21
#define MAXBUFSIZE 1024
#define RECVPORT 60535

void ipdots_to_ipcoms (char *ip);

int
main (int argc, char **argv)
{
  if (argc != 5)
    {
      printf ("Usage: ./main user password file mode\n");
      exit (1);
    }
  int socket_ctrl, socket_data, rcv_file, socket_snd;
  int reply_len, req_len;
  int file_desc, read_bytes, snd_bytes;
  int count = 0;
  socklen_t addrlen;
  char req_msg[MAXBUFSIZE], reply_msg[MAXBUFSIZE];
  char *user = argv[1];
  char *pass = argv[2];
  char *file_name = argv[3];
  char *my_ip;
  unsigned int snd_port;
  char data[MAXBUFSIZE];
  char *reply_code;
  struct sockaddr_in server, client, sender;
  struct ifreq ifr;		// For my ip
  char *p1, *p2;

  // get a fd for the control socket
  if ((socket_ctrl = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("socket desc");
      exit (1);
    }
  // get a fd for the data socket
  if ((socket_data = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("socket recv");
      exit (1);
    }
  // get a fd for the sender socket
  if ((socket_snd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("socket desc");
      exit (1);
    }

  // Create client(data) sockaddr
  memset (&client, 0, sizeof client);
  client.sin_family = AF_INET;
  client.sin_port = htons (RECVPORT);
  client.sin_addr.s_addr = INADDR_ANY;
  if (setsockopt (socket_data, SOL_SOCKET, SO_REUSEADDR, &(int)
		  {
		  1}, sizeof (int)) < 0)
    {
      perror ("setsockopt(SO_REUSEADDR) failed");
      exit (1);
    }
  if (bind (socket_data, (struct sockaddr *) &client, sizeof client) < 0)
    {
      perror ("bind:");
      close (socket_data);
      exit (1);
    }





  // Get my ip
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy (ifr.ifr_name, "wlo1", IFNAMSIZ - 1);
  ioctl (socket_data, SIOCGIFADDR, &ifr);
  my_ip = inet_ntoa (((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
  // Convert ip x.x.x.x to x,x,x,x (for PORT command)
  ipdots_to_ipcoms (my_ip);

  // Create server(control) sockaddr
  memset (&server, 0, sizeof server);
  server.sin_addr.s_addr = inet_addr (SERVER_IP);
  server.sin_family = AF_INET;
  server.sin_port = htons (SERVER_PORT);

  // Connect to server
  if (connect (socket_ctrl, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
      perror ("Connection failed");
      close (socket_ctrl);
      close (socket_data);
      exit (1);
    }
  else
    printf ("Connected to FTP server with IP:%s\n", SERVER_IP);

  // Get the greetings message
  recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
  // Get the Reply code
  reply_code = strtok (reply_msg, " ");
  printf ("Received Message from server: %s\n", reply_code);

  if (strcmp (reply_code, "220") == 0)
    {
      printf ("Server accepted the connection\n");
      // USER command
      sprintf (req_msg, "user %s", user);
      req_len = strlen (req_msg);
      send (socket_ctrl, req_msg, req_len + 1, 0);
      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
      reply_code = strtok (reply_msg, " ");
      printf ("Received Message from server: %s, sending pass\n", reply_code);

      // PASS command
      sprintf (req_msg, "pass %s", pass);
      req_len = strlen (req_msg);
      send (socket_ctrl, req_msg, req_len + 1, 0);
      printf ("Login sent\n");

      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
      reply_code = strtok (reply_msg, " ");
      printf ("Received Message from server %s\n", reply_code);

      // Succesful login
      if (strcmp (reply_code, "230") == 0)
	{
	  printf ("Login accepted\n");
	  // PORT command

	  if (strcmp (argv[4], "retr") == 0)
	    {
	      // Send port command
	      sprintf (req_msg, "port %s,236,119", my_ip);
	      req_len = strlen (req_msg);
	      send (socket_ctrl, req_msg, req_len + 1, 0);
	      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
	      reply_code = strtok (reply_msg, " ");
	      printf ("Received Message from server %s\n", reply_code);
	      // send retr commands
	      sprintf (req_msg, "retr %s", file_name);
	      req_len = strlen (req_msg);
	      send (socket_ctrl, req_msg, req_len + 1, 0);
	      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
	      reply_code = strtok (reply_msg, " ");
	      printf ("Received Message from server %s\n", reply_code);

	      // Opening the data channel
	      if (strcmp (reply_code, "150") == 0)
		{
		  if (listen (socket_data, 1) != 0)
		    {
		      perror ("listen error");
		      close (socket_ctrl);
		      close (socket_data);
		      exit (1);
		    }
		  addrlen = sizeof client;
		  if ((rcv_file =
		       accept (socket_data, (struct sockaddr *) &client,
			       &addrlen)) < 0)
		    {
		      perror ("accept error");
		      close (socket_data);
		      close (socket_ctrl);
		      exit (1);
		    }
		  // Start recieving the file
		  file_desc =
		    open (file_name, O_CREAT | O_EXCL | O_WRONLY, 0666);
		  while ((read_bytes =
			  recv (rcv_file, data, MAXBUFSIZE, 0)) > 0)
		    write (file_desc, data, read_bytes);
		  close (file_desc);
		  recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
		  reply_code = strtok (reply_msg, " ");
		  printf ("Received Message from server %s\n", reply_code);
		  if (strcmp (reply_code, "226") == 0)
		    {
		      strcpy (req_msg, "quit");
		      send (socket_ctrl, req_msg, req_len + 1, 0);
		      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
		      reply_code = strtok (reply_msg, " ");
		      printf ("Received Message from server %s\n",
			      reply_code);
		    }
		}
	    }
	  else if (strcmp (argv[4], "send") == 0)
	    {
	      // Send pasv command
	      sprintf (req_msg, "pasv");
	      req_len = strlen (req_msg);
	      send (socket_ctrl, req_msg, req_len + 1, 0);
	      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
	      reply_code = strtok (reply_msg, " ");
	      printf ("Received Message from server %s\n", reply_code);
	      // Passive mode accepted
	      if (strcmp (reply_code, "227") == 0)
		{
		  // Get the first token
		  strtok (reply_msg + 26, ",");
		  strtok (NULL, ",");
		  strtok (NULL, ",");
		  strtok (NULL, ",");
		  p1 = strtok (NULL, ",");
		  p2 = strtok (NULL, ",");
		  p2[strlen (p2) - 3] = '\0';
		  snd_port = atoi (p1) * 256 + atoi (p2);
		  printf ("Port %u recieved from server\n", snd_port);
		  // Create the send addr
		  memset (&sender, 0, sizeof sender);
		  sender.sin_family = AF_INET;
		  sender.sin_port = htons (snd_port);
		  sender.sin_addr.s_addr = inet_addr (SERVER_IP);
		  if (connect
		      (socket_snd, (struct sockaddr *) &sender,
		       sizeof (sender)) < 0)
		    {
		      perror ("Connection failed");
		      close (socket_snd);
		      close (socket_ctrl);
		      exit (1);
		    }
		  sprintf (req_msg, "stor %s", file_name);
		  req_len = strlen (req_msg);
		  send (socket_ctrl, req_msg, req_len + 1, 0);
		  recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
		  reply_code = strtok (reply_msg, " ");
		  printf ("Received Message from server %s\n", reply_code);
		  // Server opening connection
		  if (strcmp (reply_code, "150") == 0)
		    {
		      // Start sending the file
		      file_desc = open (file_name, O_RDONLY);
		      while ((read_bytes =
			      read (file_desc, data, MAXBUFSIZE)) > 0)
			send (socket_snd, data, read_bytes, 0);
		      close (file_desc);
		      // Put an end to all further connections
		      shutdown (socket_snd, SHUT_RDWR);
		      recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
		      reply_code = strtok (reply_msg, " ");
		      printf ("Received Message from server %s\n",
			      reply_code);
		      // Succesful transfer
		      if (strcmp (reply_code, "226") == 0)
			{
			  strcpy (req_msg, "quit");
			  send (socket_ctrl, req_msg, req_len + 1, 0);
			  recv (socket_ctrl, reply_msg, MAXBUFSIZE, 0);
			  reply_code = strtok (reply_msg, " ");
			  printf
			    ("Received Message from server %s(goodbye)\n",
			     reply_code);
			}
		    }
		}
	    }
	}
    }
  close (socket_data);
  close (socket_ctrl);
  close (socket_snd);

  return 0;
}


void
ipdots_to_ipcoms (char *ip)
{
  int i, len;
  len = strlen (ip);
  for (i = 0; i < len; i++)
    {
      if (ip[i] == '.')
	ip[i] = ',';
    }
  return;
}
