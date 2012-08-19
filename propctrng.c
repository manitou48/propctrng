/**
 * @file propctrng.c
 * This program demonstrates starting a PASM COG to generate TRNGs
 *  uses Chip Gracey's Real Randmo v1.2
 *  uses CNTA
 *
 * to use:
 * from directory containing source
 * make clean
 * make
 * propeller-load -b quickstart  -t -r propctrng.elf 
 * MIT Licensed - terms of use below.
 */

#include <stdio.h>
#include <propeller.h>                    // propeller specific definitions

// http://gurmeetsingh.wordpress.com/2008/08/05/fast-bit-counting-routines/
#define MASK_01010101 (((unsigned int)(-1))/3)
#define MASK_00110011 (((unsigned int)(-1))/5)
#define MASK_00001111 (((unsigned int)(-1))/17)
int bitcount (unsigned int n) {
	n = (n & MASK_01010101) + ((n >> 1) & MASK_01010101) ;
	n = (n & MASK_00110011) + ((n >> 2) & MASK_00110011) ;
	n = (n & MASK_00001111) + ((n >> 4) & MASK_00001111) ;
	return n % 255 ;
}

// the STATIC HUB mailbox for communication to PASM routine 
//
static unsigned int rng; // a pointer to this gets passed to the PASM code as the PAR register

// C stub function to start the PASM routine
// need to be able to provide the entry point to the PASM
// and a pointer to the STATIC HUB mailbox
// the cognew function in the propeller.c library returns the COG #
//
int start(unsigned int *pinptr)
{
    // The label binary_propctrng_dat_start is automatically placed
    // on the cog code from porpctrng.dat by objcopy (see the Makefile).
    extern unsigned int binary_propctrng_dat_start[];
    return cognew(&binary_propctrng_dat_start, pinptr);
}

// C main function
// LMM model
void main (int argc,  char* argv[])
{
	unsigned int cnt=0, prev, current, ones, onescnt=0;
	static unsigned int bytes[256];

    printf("hello, world!\n");            // let the lead LMM COG say hello
    printf ("New COG# %d started.\n",start(&rng)); // start a new COG passing a pointer to the STATIC mailbox structure
    waitcnt(CLKFREQ/200 + CNT);  // give TRNG 5ms to collect entropy
	prev = rng;
    while (1)
    {
      waitcnt(16000 + CNT);  // TRNG needs 200us for 32 bits, 16000 ticks at 80MHz
	  current = rng;
	  prev = prev ^ current;
	  ones = bitcount(prev);
	  onescnt += ones;
	  bytes[current &0xff]++;
	  bytes[(current>>8) &0xff]++;
	  bytes[(current>>16) &0xff]++;
	  bytes[(current>>24) &0xff]++;
	  cnt++;
      if(cnt % 1000 == 0) {
		unsigned int i, min = 0xffffffff,mini,max=0,maxi;
		for (i=0; i< 256;i++) {
			if (bytes[i] < min) { mini = i; min = bytes[i];}
			if (bytes[i] > max) { maxi = i; max = bytes[i];}
		}
	  	printf("%08x ones %d %d cnt %d  %d min %02x %d max %02x %d \n",
		 current,ones,onescnt/cnt,cnt, cnt/64,mini,min,maxi,max);
	  }
	  prev = current;
    }    
}

/*
    +--------------------------------------------------------------------
      TERMS OF USE: MIT License
    +--------------------------------------------------------------------
    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    +--------------------------------------------------------------------
*/
