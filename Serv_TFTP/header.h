#include <sys/types.h>
#define ACK 1
#define FIN 2
#define OPCODE_SIZE 2
#define NUMBLOCK_SIZE 2
#define PACKET_SIZE 516
#define ACK_PACKET_SIZE (OPCODE_SIZE + NUMBLOCK_SIZE)
#define DATA_SIZE (PACKET_SIZE-OPCODE_SIZE-NUMBLOCK_SIZE)

typedef struct _packet_ {
        u_short opcode;
        u_char data[DATA_SIZE + 2];
} packet_t;

typedef struct _data_packet_ {
        u_short opcode;
        u_short num_block;
        u_char data[DATA_SIZE];
} data_packet_t;

typedef struct _ack_packet_ {
        u_short opcode;
        u_short num_block;
} ack_packet_t;

int array_to_packet(u_char* src, packet_t* packet);

int packet_to_array(u_char* dest, packet_t* packet);

int data_packet_to_array(u_char* dest, data_packet_t* data_packet);

int array_to_data_packet(u_char* src, data_packet_t* data_packet);

int ack_packet_to_array(u_char* dest, ack_packet_t* ack_packet);

int array_to_ack_packet(u_char* src, ack_packet_t* data_packet);

int recvfromTimeOut(int socket, long sec, long usec);
