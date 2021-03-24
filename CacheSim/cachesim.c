#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"

// Note: You won't find <YOUR CODE HERE> comments in this file.
// This is to get you to read and understand everything here.
// Good luck!

// - Anshu

// gets the log of a given number
int log2int(int n) {
    int r=0;
    while (n>>=1) r++;
	return r; 
}

// create struct for cache
struct cacheEntry {
	int offset;
	int index;
	int tag;
	int dirtyBit;
	int validBit;
	int refBit;
	int lru;
	unsigned char* data[64];
};
struct cacheEntry myCache;



// start program
int main (int argc, char* argv[]) {
	init_memory();
	int cacheSize, associativity, blockSize;

    // Buffer to store instruction (i.e. "load" or "store")
	char instruction_buffer[6];

    // Open the trace file in read mode
	FILE* myFile = fopen(argv[1], "r");

    // Read in the command line arguments
	sscanf(argv[2], "%d", &cacheSize);
	sscanf(argv[3], "%d", &associativity);
	sscanf(argv[4], "%d", &blockSize);

	// create cache with appropriate size
	int numBlocks = (cacheSize * 1024) / blockSize;
	int numSets = numBlocks / associativity;
	//unsigned char cache[numSets][associativity];

	// make struct for set
	struct setEntry {
		// one-liner for blocks per set
		struct cacheEntry blocks[numBlocks / numSets];
	};

	// initialize the cache
	//struct setEntry* cache[numSets][associativity];
	struct setEntry* cache = (struct setEntry*) malloc(sizeof(struct cacheEntry*) * numBlocks);
	for (int r=0; r < numSets; r++) {
		for (int c=0; c < associativity; c++) {
			struct cacheEntry init;
			cache[r].blocks[c] = init;
			// cache[r].blocks[c].validBit = 0;
			//cache[r].blocks[c].data = (unsigned char*) malloc(blockSize);
		}
	}
	// for (int r=0; r < numSets; r++) {
	// 	struct setEntry init;
	// 	cache[r] = init;
	// 	//cache[r].validBit = 0;
	// }

	// number of offset, index, and tag bits
	int offsetBits = log2int(blockSize);
	int indexBits = log2int(numSets);
	int tagBits = 24 - offsetBits - indexBits;	// addresses are 24 bits

    // Keep reading the instruction until end of file
	while(fscanf(myFile,"%s", instruction_buffer)!=EOF) {
		int currAddress, accessSize;

        // Read the address and access size info
		fscanf(myFile, "%x", &currAddress);
		fscanf(myFile, "%d", &accessSize);
		
		// break the address into its parts for the cache
		int tag = currAddress >> indexBits >> offsetBits;
		int index = (currAddress >> offsetBits) & ((1 >> indexBits) - 1);	// latter half is the mask
		int offset = currAddress & ((1 >> offsetBits) - 1);		// latter half still is the mask

		bool hit = false;
		if (instruction_buffer[0]=='l'){    // If load
			for (int i=0; i < associativity; i++) {
				if (cache[index].blocks[i].validBit == 1) {
					if (cache[index].blocks[i].tag == tag) {
						hit = true;
						cache[index].blocks[i].refBit = 1;
						cache[index].blocks[i].lru = 0;
						//cache[index].blocks[i].dirtyBit = 1;
						//int addy = strtol(cache[index].blocks[i].data, NULL, 2);
						// Print the load line in the same format as trace file
						// printf("load 0x%x hit %x\n", currAddress, (int)strtol(*cache[index].blocks[i].data, NULL, 2));
						printf("load 0x%x hit ", currAddress);
						for (int j=0; j < accessSize; j++) {
							printf("%02x", (unsigned int)strtol(cache[index].blocks[i].data[j], NULL, 16));
							//printf("%02x", cache[index].blocks[i].data[j]);
						}
						printf("\n");
						break;
					} else {
						cache[index].blocks[i].refBit = 0;
					}
				} else {
					cache[index].blocks[i].refBit = 0;
				}
			}
			if (hit == false) {
				char data_buffer[accessSize*2 + 1];
				read_from_memory(data_buffer, currAddress, 2*accessSize);
				for (int i=0; i < associativity; i++) {
					if (cache[index].blocks[i].refBit == 0) {
						// find lru
						int lruIndex = 0;
						for (int k=0; k < associativity; k++) {
							if (cache[index].blocks[k].lru > lruIndex) {
								lruIndex = k;
							}
						}

						if (cache[index].blocks[i].dirtyBit == 1) {
							// write previous data back to memory
							write_to_memory(data_buffer, currAddress, 2*accessSize);
						}
						// set its attributes
						*cache[index].blocks[i].data = data_buffer;
						cache[index].blocks[i].tag = atoi(data_buffer) >> (indexBits + offsetBits);
						cache[index].blocks[i].validBit = 1;
						cache[index].blocks[i].refBit = 1;
						cache[index].blocks[i].dirtyBit = 0; 
						//int addy = strtol(cache[index].blocks[i].data, NULL, 2);
						//printf("load 0x%x miss %x\n", currAddress, (int)strtol(*cache[index].blocks[i].data, NULL, 2));
						printf("load 0x%x miss ", currAddress);
						for (int j=0; j < accessSize; j++) {
							printf("%02x", (unsigned int)strtol(cache[index].blocks[i].data[j], NULL, 16));
							//printf("%02x", cache[index].blocks[i].data[j]);
						}
						printf("\n");

						//increase each lru
						for (int h=0; h < associativity; h++) {
							cache[index].blocks[h].lru += 1;
						}
						break;
					}
				}
			}
            
			
		}
        else {                              // Else store
            // Buffer to store data to be stored
            char data_buffer[17];

            // Read the data (value to be written to memory)
            fscanf(myFile, "%s", data_buffer);

			// write to memory, need to see if it's a hit or miss
			for (int i=0; i < associativity; i++) {
				if (cache[index].blocks[i].validBit == 1) {
					if (cache[index].blocks[i].tag == tag) {
						hit = true;
						cache[index].blocks[i].refBit = 1;
						*cache[index].blocks[i].data = data_buffer;
						cache[index].blocks[i].dirtyBit = 1;
						// Print the store line in the same format as trace file
						printf("store 0x%x hit\n", currAddress);
						break;
					} else {
						cache[index].blocks[i].refBit = 0;
					}
				} else {
					cache[index].blocks[i].refBit = 0;
				}
			}
			if (hit == false) {
				// char data_buffer[accessSize];
				// read_from_memory(data_buffer, currAddress, blockSize);
				for (int i=0; i < associativity; i++) {
					if (cache[index].blocks[i].refBit == 0) {
						if (cache[index].blocks[i].dirtyBit == 1) {
							// write previous data back to memory
							write_to_memory(data_buffer, currAddress, 2*accessSize);
						}
						char data_buffer[accessSize];
						read_from_memory(data_buffer, currAddress, 2*accessSize);
						// set its attributes
						*cache[index].blocks[i].data = data_buffer;
						cache[index].blocks[i].tag = atoi(data_buffer) >> (indexBits + offsetBits);
						cache[index].blocks[i].validBit = 1;
						cache[index].blocks[i].refBit = 1;
						cache[index].blocks[i].dirtyBit = 1;
						// Print the store line in the same format as trace file
						printf("store 0x%x miss\n", currAddress);
					}
				}
			}

            
		}
	}
	// for (int r=0; r < numSets; r++) {
	// 	for (int c=0; c < associativity; c++) {
	// 		free(cache[r].blocks[c].data);
	// 	}
	// }

	destroy_memory();
	return EXIT_SUCCESS;
}
