#include "dhcp.h"

int
DHCP_offer (unsigned char *array)
{
  /*Opcode (Boot Reply) */
  array[0] = 2;

  /*Number fo seconds */
  array[8] = 0;
  array[9] = 0;

  /*Client IP address */
  array[16] = 192;
  array[17] = 168;
  array[18] = 1;
  array[19] = 110;

  /*Your IP address */
  array[24] = 192;
  array[25] = 168;
  array[26] = 1;
  array[27] = 110;

  /*Magic Cookie */
  array[236] = 0x63;		// D
  array[237] = 0x82;		// H
  array[238] = 0x53;		// C
  array[239] = 0x63;		// P

  /*DHCP Message Type */
  array[240] = 0x35;		// Type
  array[241] = 0x01;		// Length
  array[242] = 0x02;		// Is an Offer

  /*DHCP Server Identifier */
  array[243] = 0x36;		// Type
  array[244] = 0x04;		// Length
  // Identifier
  array[245] = 192;
  array[246] = 168;
  array[247] = 1;
  array[248] = 100;

  /* IP Address Lease Time */
  array[249] = 0x33;		// Type
  array[250] = 0x04;		// Length
  // Time
  array[251] = 0x00;
  array[252] = 0x01;
  array[253] = 0x51;
  array[254] = 0x80;

  /*Renewal Time Value */
  array[255] = 0x3a;		// Type
  array[256] = 0x04;		// Length
  // Time
  array[257] = 0x00;
  array[258] = 0x00;
  array[259] = 0x00;
  array[260] = 0x3c;

  /*Subnet Mask */
  array[261] = 0x01;		// Type
  array[262] = 0x04;		// Length
  // Mask
  array[263] = 255;
  array[264] = 255;
  array[265] = 255;
  array[266] = 0;

  /*Domain Name Server */
  array[267] = 0x06;		// Type
  array[268] = 0x04;		// Length
  array[269] = 8;
  array[270] = 8;
  array[271] = 8;
  array[272] = 8;

  /*Domain Name */
  array[273] = 0x0f;		// Type
  array[274] = 0x03;		// Length
  array[275] = 'l';
  array[276] = 'a';
  array[277] = 'n';

  /*Router */
  array[278] = 0x03;		// Type
  array[279] = 0x04;		// Length
  array[280] = 192;
  array[281] = 168;
  array[282] = 1;
  array[283] = 100;

  /*End */
  array[284] = 0xff;		// Type

  return 285;
}


int
DHCP_ACK (unsigned char *array)
{
  /*Opcode (Boot Reply) */
  array[0] = 2;

  /*Number fo seconds */
  array[8] = 0;
  array[9] = 0;

  /*Client IP address */
  array[16] = 192;
  array[17] = 168;
  array[18] = 1;
  array[19] = 110;

  /*Your IP address */
  array[24] = 192;
  array[25] = 168;
  array[26] = 1;
  array[27] = 100;

  /*Magic Cookie */
  array[236] = 0x63;
  array[237] = 0x82;
  array[238] = 0x53;
  array[239] = 0x63;

  /*DHCP Message Type */
  array[240] = 0x35;		// Type
  array[241] = 0x01;		// Length
  array[242] = 0x05;

  /*DHCP Server Identifier */
  array[243] = 0x36;		// Type
  array[244] = 0x04;		// Length
  // Identifier
  array[245] = 192;
  array[246] = 168;
  array[247] = 1;
  array[248] = 100;

  /* IP Address Lease Time */
  array[249] = 0x33;		// Type
  array[250] = 0x04;		// Length
  // Time
  array[251] = 0x00;
  array[252] = 0x01;
  array[253] = 0x51;
  array[254] = 0x80;

  /*Renewal Time Value */
  array[255] = 0x3a;		// Type
  array[256] = 0x04;		// Length
  // Time
  array[257] = 0x00;
  array[258] = 0x00;
  array[259] = 0x00;
  array[260] = 0x3c;

  /*Subnet Mask */
  array[261] = 0x01;		// Type
  array[262] = 0x04;		// Length
  // Mask
  array[263] = 255;
  array[264] = 255;
  array[265] = 255;
  array[266] = 0;

  /*Domain Name Server */
  array[267] = 0x06;		// Type
  array[268] = 0x04;		// Length
  array[269] = 8;
  array[270] = 8;
  array[271] = 8;
  array[272] = 8;

  /*Domain Name */
  array[273] = 0x0f;		// Type
  array[274] = 0x03;		// Length
  array[275] = 'l';
  array[276] = 'a';
  array[277] = 'n';

  /*Router */
  array[278] = 0x03;		// Type
  array[279] = 0x04;		// Length
  array[280] = 192;
  array[281] = 168;
  array[282] = 1;
  array[283] = 254;
  //array[283] = 100; 

  array[284] = 0xff;		// Type

  return 285;
}
