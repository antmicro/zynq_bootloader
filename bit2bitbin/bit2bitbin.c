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

char magic_hdr_cmp[] = { 0x00, 0x09, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x00, 0x00, 0x01 };

#define fail(...) do { fprintf(stderr, "Error: " __VA_ARGS__); exit(-1); } while (0)

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Zynq .bit to .bit.bin file converted\n");
		printf("(c) 2013 Antmicro Ltd.\n");
		printf("Usage: bit2bitbin <bitstream> <output_raw_bitstream>\n");
		exit(-1);
	}
	FILE *f_bit = fopen(argv[1], "r");
        if (!f_bit) {
                fail("file %s does not exist\n", argv[1]);
        }
        char magic_hdr[13];
        fread(&magic_hdr, 1, sizeof(magic_hdr), f_bit);
        if (memcmp(&magic_hdr, &magic_hdr_cmp, 13) != 0) {
                fclose(f_bit);
                fail("bit file seems to be incorrect.\n");
        }
        while (1) {
                char section_hdr[2];
                fread(&section_hdr, 1, sizeof(section_hdr), f_bit);
                if (section_hdr[1] != 0x0) {
                        fclose(f_bit);
                        fail("bit file seems to have mismatched sections.\n");
                }
                if (section_hdr[0] == 'e') break;
                uint8_t section_size;
                fread(&section_size, 1, sizeof(uint8_t), f_bit);
                char section_data[255];
                fread(&section_data, 1, section_size, f_bit);
                printf("Section '%c' size=%d : data = \"%s\"\n", section_hdr[0], section_size, section_data);
        }
        uint32_t bit_size;
        fread(&bit_size, 1, 3, f_bit);
        bit_size = ((bit_size >> 16) & 0xFF) | (bit_size & 0xFF00) | ((bit_size << 16) & 0xFF0000);
        printf("bitstream size is %u\n",bit_size);


        FILE *f = fopen(argv[2], "w");
        if (!f) {
                fclose(f_bit);
                fail("Error creating file '%s'.\n", argv[2]);
        }

        int i;
	char old_val[4];
        char new_val[4];
	for (i = 0; i < bit_size; i+=sizeof(old_val)) {
		int read = fread(&old_val, 1, sizeof(old_val), f_bit);
                new_val[0] = old_val[3];
                new_val[1] = old_val[2];
                new_val[2] = old_val[1];
                new_val[3] = old_val[0];
		fwrite(&new_val, 1, read, f);
	}
	fclose(f_bit);
        fclose(f);
}
