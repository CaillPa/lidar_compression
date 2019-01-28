#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/poll.h>

#define SIZE_GLOBAL_HEADER 24
#define SIZE_PACKET_HEADER 16
#define SIZE_PACKET 1248
#define TIMEOUT 5

// builds a bitmask the same size as a Velodyne data packet
// with nbits zeroed bits on each range measurement
char *make_mask(int nbits) {
    char symbols[] = {255, 254, 252, 248, 240, 224, 192, 128, 0};
    char symbol = symbols[nbits];
    char *mask = (char*)malloc(SIZE_PACKET*sizeof(char));
    memset(mask, 255, SIZE_PACKET);
    int i, j;
    for(i = 0; i < 12; i++) {
        for(j = 0; j < 32; j++) {
            mask[42+i*100+4+j*3] = symbol;
        }
    }
    return mask;
}

// the program reads data packets from its stdin and writes the
// bitmasked data packets to its stdout
int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("usage : ./mask nbits");
        return EXIT_SUCCESS;
    }
    // create the bitmask
    char *mask = make_mask(atoi(argv[1]));
    // number of iterations per packet
    size_t iters = SIZE_PACKET / sizeof(long long);
    // allocate a buffer to store one packet
    char *buff = (char*)malloc(SIZE_PACKET*sizeof(char));
    // so we can manipulate buff and mask 16 bytes at a time
    long long *ptr = (long long*) buff;
    long long *mask_ptr = (long long*) mask;

    // copy the global PCAP header
    fread(buff, sizeof(char), SIZE_GLOBAL_HEADER, stdin);
    fwrite(buff, sizeof(char), SIZE_GLOBAL_HEADER, stdout);

    while(fread(buff, sizeof(char), SIZE_PACKET_HEADER, stdin) == SIZE_PACKET_HEADER) {
        // copy the pcap packet header
        fwrite(buff, sizeof(char), SIZE_PACKET_HEADER, stdout);

        // read a packet and perform the bitwise AND
        if(fread(buff, sizeof(char), SIZE_PACKET, stdin) == SIZE_PACKET) {
            for(size_t i = 0; i < iters; ++i) {
                ptr[i] &= mask_ptr[i];
            }
            fwrite(buff, sizeof(char), SIZE_PACKET, stdout);
        }
    }
    return EXIT_SUCCESS;
}
