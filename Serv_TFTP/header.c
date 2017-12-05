#include "header.h"
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

int
recvfromTimeOut (int socket, long sec, long usec)
{
  // Setup timeval variable
  struct timeval timeout;
  timeout.tv_sec = sec;
  timeout.tv_usec = usec;
  // Setup fd_set structure
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (socket, &fds);
  // >0: data ready to be read
  select (FD_SETSIZE, &fds, NULL, NULL, &timeout);
  return FD_ISSET (socket, &fds);
}

int
array_to_packet (u_char * src, packet_t * packet)
{
  memcpy (&(*packet).opcode, src, OPCODE_SIZE);
  memcpy (packet->data, src + OPCODE_SIZE, DATA_SIZE + 2);
  return (*packet).opcode;
}

int
packet_to_array (u_char * dest, packet_t * packet)
{
  memcpy (dest, &(*packet).opcode, OPCODE_SIZE);
  memcpy (dest + sizeof (*packet).opcode, packet->data, DATA_SIZE + 2);
  return 0;
}

// DATA
int
data_packet_to_array (u_char * dest, data_packet_t * data_packet)
{
  memcpy (dest, &(*data_packet).opcode, OPCODE_SIZE);
  memcpy (dest + sizeof (*data_packet).opcode, &(*data_packet).num_block,
	  NUMBLOCK_SIZE);
  memcpy (dest + sizeof (*data_packet).opcode +
	  sizeof (*data_packet).num_block, data_packet->data, DATA_SIZE);
  return 0;
}

int
array_to_data_packet (u_char * src, data_packet_t * data_packet)
{
  memcpy (&(*data_packet).opcode, src, OPCODE_SIZE);
  memcpy (&(*data_packet).num_block, src + OPCODE_SIZE, NUMBLOCK_SIZE);
  memcpy (data_packet->data, src + OPCODE_SIZE + NUMBLOCK_SIZE, DATA_SIZE);
  return (*data_packet).opcode;
}

// ACK
int
ack_packet_to_array (u_char * dest, ack_packet_t * ack_packet)
{
  memcpy (dest, &(*ack_packet).opcode, OPCODE_SIZE);
  memcpy (dest + sizeof (*ack_packet).opcode, &(*ack_packet).num_block,
	  NUMBLOCK_SIZE);
}

int
array_to_ack_packet (u_char * src, ack_packet_t * ack_packet)
{
  memcpy (&(*ack_packet).opcode, src, OPCODE_SIZE);
  memcpy (&(*ack_packet).num_block, src + OPCODE_SIZE, NUMBLOCK_SIZE);
  return (*ack_packet).opcode;
}
