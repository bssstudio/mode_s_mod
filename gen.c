
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

//crc stuff
unsigned int crc_table[256];
const unsigned int POLY=0xFFF409;

void generate_crc_table(void)
{
    unsigned int crc = 0;
    for(int n=0; n<256; n++) {
        crc = n<<16;
        for(int k=0; k<8; k++) {
            if(crc & 0x800000) {
                crc = ((crc<<1) ^ POLY) & 0xFFFFFF;
            } else {
                crc = (crc<<1) & 0xFFFFFF;
            }
        }
        crc_table[n] = crc & 0xFFFFFF;
    }
}

//Perform a bytewise CRC check
unsigned int modes_check_crc(unsigned char data[], int length)
{
    unsigned int crc=0;
    for(int i=0; i<length; i++) {
        crc = crc_table[((crc>>16) ^ data[i]) & 0xff] ^ (crc << 8);
    }
    return crc & 0xFFFFFF;
}


void main() {

  generate_crc_table();

  char buf[] = {0x00, 0x00, 0x00, 0x00};
  char buf2[] = {0x01, 0x00, 0x00, 0x00};

  //int  rawpkt[] = {1,0,1,0, 0,0,0,0, 1,0,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,0,0,1,1,1,0,1,1,0,1,1,0,1};
  int  preamble[] = {1,0,1,0, 0,0,0, 1,0,1,0, 0,0,0,0, 0};
  int rawpkt[4096];

  //generate raw bits -- TODO: do this better
  srand(time(NULL));
  int tx_counter = 0;

  while (1) {

  unsigned char packet[14];
  int packet_bits[112];
  // random data
  for (int k=0; k<14-3; k++) {
      packet[k] = (unsigned char) (rand() % 256);
  }
  packet[0] = 0x8a;
  //crc
  int crc = modes_check_crc(packet, 11);
  fprintf(stderr, "CRC: %d\n", crc);
  packet[11] = (unsigned char) ((crc >> 16) & 0xff);
  packet[12] = (unsigned char) ((crc >> 8) & 0xff);
  packet[13] = (unsigned char) ((crc >> 0) & 0xff);

  //packet[0]  = 0x8d;
  //packet[1]  = 0x44;
  //packet[2]  = 0x07;
  //packet[3]  = 0x53;
  //packet[4]  = 0xea;
  //packet[5]  = 0x46;
  //packet[6]  = 0x68;
  //packet[7]  = 0x60;
  //packet[8]  = 0x01;
  //packet[9]  = 0x5f;
  //packet[10] = 0x08;
  //packet[11] = 0xcb;
  //packet[12] = 0xfd;
  //packet[13] = 0x28;

  for (int j=0; j<14;j++) {
    for (int b=0; b<8; b++) {
      unsigned char byt = packet[j];
      int bit = (byt >> (7-b)) & 1;
      packet_bits[j*8 + b] = bit;
      fprintf(stderr, "%d", bit);
    }
  }
  fprintf(stderr, "\n");

  //construct the packet
  //preamble
  int pktpos = 0;
  int preamble_len = sizeof(preamble)/4;
  for (int k=0; k<preamble_len; k++) {
      rawpkt[pktpos] = preamble[k];
      pktpos++;
  }

  //data
  int data_len = sizeof(packet_bits)/4;
  for (int k=0; k<data_len; k++) {
      int bit = packet_bits[k];
      int left, right = 0;
      if (bit == 0) {
        left = 0; 
        right = 1;  //??
      } else {
        left = 1; 
        right = 0;  //??
      }
      rawpkt[pktpos] = left;
      pktpos++;
      rawpkt[pktpos] = right;
      pktpos++;
  }


      
      for (int k=0; k<pktpos; k++) {
        int bit = rawpkt[k];
        
        if (bit == 0) {
            for (int i=0; i<10; i++) {
              fwrite(buf, 1, 4, stdout);
            }
        } else {
            for (int i=0; i<4; i++) {
              fwrite(buf, 1, 4, stdout);
            }
            fwrite(buf2, 1, 4, stdout);
            //fwrite(buf2, 1, 4, stdout);
            for (int i=0; i<5; i++) {
              fwrite(buf, 1, 4, stdout);
            }
        }

        //usleep(100);
      }

      for (int k=0; k<5000; k++) {
            fwrite(buf, 1, 4, stdout);
            //usleep(100);
      }
      
      tx_counter++;
      fprintf(stderr, "Tx_cnt: %d\n",tx_counter);
      usleep(1000 * 250);
      //exit(0);
  }
}
