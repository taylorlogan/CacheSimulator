#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"
// No starter code here
// At this point in ECE/CS 250, you should be able to write a basic C I/O program from scratch.

// TODO: Everything

// gets the log of a given number
int log2int(int n) {
    int r=0;
    while (n>>=1) r++;
	return r; 
}

// starting actual program
int main (int argc, char* argv[]) {
	int addressBits, pageSize, numPages;

	// open the file & set the address bits, page size, # of pages
	FILE* pageTable = fopen(argv[1], "r");
	fscanf(pageTable, "%d", &addressBits);
	fscanf(pageTable, "%d", &pageSize);
	fscanf(pageTable, "%*[^\n]\n"); // bypass newline char after 1st line
	
	// get the virtual address (int form)
	char* virtAddrStr;
	sscanf(argv[2], "%s", virtAddrStr);
	int virtAddress;
	virtAddress = (int)strtol(virtAddrStr, NULL, 16);

	// get # of offset bits
	int offsetbits;
	offsetbits = log2int(pageSize);
	// idea to get actual offset each time -> create mask of
	// offsetbits # of 1's to retain offset bits from virtual address
	// ex: 4 offset bits -> create mask of 00001111 and AND it with 
	// virtual address to only get the offset you want to add to PPN
	int offsetmask = 1;
	offsetmask = offsetmask<<offsetbits;
	offsetmask -= 1;
	// get actual page offset using mask
	int pageOffset = virtAddress & offsetmask;

	// get line number of page table
	int lnNum = (virtAddress - pageOffset) >> offsetbits;

	// read the rest of the file and do what you gotta do
	int currentPPN;
	for (int i = 0; i < lnNum; i++) {
		// get the current PPN from the page table
		fscanf(pageTable, "%d", &currentPPN);
	}
	
	fscanf(pageTable, "%d", &currentPPN);
	// if PPN is -1 generate PAGEFAULT, else generate physical address
	if (currentPPN == -1) {
		printf("%s\n", "PAGEFAULT");
	} else {
		// output the PPN's corresponding physical address
		int val = (currentPPN << offsetbits) + pageOffset;
		printf("%x\n", val);
	}

	return 0;
}
