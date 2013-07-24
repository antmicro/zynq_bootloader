/* 
 * (c) 2013 Antmicro Ltd <www.antmicro.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// bin file structure
typedef struct {
        uint32_t magic1;
        uint32_t magic2; 
        uint32_t zero1;
        uint32_t magic3;
        uint32_t bootloader_addr;
        uint32_t bootloader_len;
        uint32_t zero2;
        uint32_t zero3;
        uint32_t bootloader_len2;
        uint32_t magic4;
        uint32_t crcsum;
} binhdr;

long fsize(char *fname) {
	long size = 0;
	FILE *f = fopen(fname, "rb");
	if (f) {
	   fseek(f, 0, SEEK_END);
	   size = ftell(f);
	   fclose(f);
	}
	return size;
}

#define BIN_START 0xac0

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Zynq .bin file generator\n");
		printf("(c) 2013 Antmicro Ltd.\n");
		printf("Usage: zbin <bootloader_binary> <output_bin_file>\n");
		exit(-1);
	}
	uint32_t fs = fsize(argv[1]);
	if (fs == 0) {
		printf("Error: file %s does not exists or its size equals to 0\n", argv[1]);
		exit(-1);
	}
        if (fs > 0x30000) {
                printf("Error: binary cannot be greater than 0x30000.\n");
                exit(-1);
        }
        binhdr hdr;
        memset(&hdr, 0x0, sizeof(binhdr));
        hdr.magic1 = 0xaa995566;
        hdr.magic2 = 0x584c4e58;
        hdr.magic3 = 0x01010000;
        hdr.magic4 = 0x00000001;
        hdr.bootloader_addr = BIN_START;
        hdr.bootloader_len = fs;
        hdr.bootloader_len2 = hdr.bootloader_len;
        uint32_t *data = (uint32_t*)&hdr;
        int i;
        for (i = 0; i < (sizeof(binhdr)/4)-1; i++) hdr.crcsum = hdr.crcsum + data[i];
        hdr.crcsum = ~hdr.crcsum;

        FILE *f = fopen(argv[2], "w");
        if (f == NULL) {
                printf("Error creating file '%s'.\n", argv[2]);
                exit(-1);
        }
        // insert start sequence
        uint32_t magic = 0xeafffffe;
        for (i = 0; i < 8; i++) fwrite((char*)&magic,1,4,f);
        // insert the header
        fwrite((char*)&hdr,1,sizeof(hdr),f);
        uint32_t zero = 0x00000000;
	uint32_t ff = ~0x00000000;
        for (i = 0; i < 21; i++) fwrite((char*)&zero,1,4,f);
        for (i = 0; i < 256; i++) {
                fwrite((char*)&ff,1,4,f);
                fwrite((char*)&zero,1,4,f);
        }
        // pad the header
        int left_to_pad = (hdr.bootloader_addr-ftell(f))/4;
        for (i = 0; i < left_to_pad; i++) fwrite((char*)&zero,1,4,f);
	FILE *f_bin = fopen(argv[1], "r");
	char buf[1024];
        // copy the binary
	for (i = 0; i < fs; i+=sizeof(buf)) {
		int read = fread(buf, 1, sizeof(buf), f_bin);
		fwrite(buf, 1, read, f);
	}
	fclose(f_bin);
        fclose(f);
}
