#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44000
#define NUM_CHANNELS 2
#define BIT_DEPTH 16 // two bytes per sample; four bytes per stereo sample (the 'byterate')

/* NOTE: Subchunk2Size (the size of the actual data in bytes) is 4 bytes (stored as an unsigned int)
 * so WAVE files are limited to a maximum file size of (2^32 - 1) bytes: approx 4.3 gigabytes. */

unsigned long copy_bytes(FILE*, FILE*, int);
void write_riff_header(FILE*, int);
void write_fmt_subchunk(FILE*);
void write_data_header(FILE*, unsigned long);

int main(int argc, char *argv[]) {
	// Get stretch factor as (optional) argument
	int stretch_factor;
	char *s;
	if ((s = argv[3]) != NULL)
		stretch_factor = atoi(s);
	else
		stretch_factor = 1;
	
	// Open source file
	FILE *src = fopen(argv[1], "rb");

	// Open target file
	strcat(argv[2], ".wav");
	FILE *tgt = fopen(argv[2], "wb");

	// Seek to data chunk location (44 bytes in)
	fseek(tgt, 44, SEEK_SET);

	// Write raw data
	unsigned long n = copy_bytes(src, tgt, stretch_factor);

	// Seek back to the beginning again
	fseek(tgt, 0, SEEK_SET);

	// Write RIFF subchunk header
	write_riff_header(tgt, n);

	// Write fmt subchunk header
	write_fmt_subchunk(tgt);

	// Write data subchunk header
	write_data_header(tgt, n * stretch_factor);

	int secs = (n * stretch_factor) / (SAMPLE_RATE * NUM_CHANNELS * BIT_DEPTH / 8);
	printf("Wrote %lu bytes (about %d seconds of audio).\n", n * stretch_factor, secs);

	return 0;
}

/* write bytes from stream pointed to by fi to that pointed to by fo, stretching by factor sf. */
unsigned long copy_bytes(FILE *fi, FILE *fo, int sf) {
	char c;
	unsigned long i;
	int j;
	// Write bytes from source to target
	for (i = 0; !feof(fi); i++) {
		c = fgetc(fi);
		for (j = 0; j < sf; j++)
			fputc(c, fo);
	}

	//printf("Last byte was %02x.\n", c);
	// return number of bytes copied across (ignoring sf)
	return i;
}

void write_riff_header(FILE *f, int n) {
	/* (the below data (excluding ASCII strings) should be in little-endian) */
	// ChunkID - 4 bytes
	fputs("RIFF", f);
	
	// ChunkSize - 4 bytes
	unsigned int a[1];
	a[0] = (unsigned int) n + 36; // number of bytes in the data plus 36
	fwrite(a, 4, 1, f);
	
	// Format - 4 bytes
	fputs("WAVE", f); // (0x57415645 big-endian)	
}

void write_fmt_subchunk(FILE *f) {
	unsigned int a[1];
	unsigned short b[1];

	// Subchunk1ID - 4 bytes
	fputs("fmt ", f);

	// Subchunk1Size - 4 bytes (16 for PCM)
	a[0] = 16;
	fwrite(a, 4, 1, f);

	// AudioFormat - 2 bytes (1 for PCM) (i.e. linear quantization)
	b[0] = 1;
	fwrite(b, 2, 1, f);

	// NumChannels - 2 bytes (in this case, stereo. so 2)
	b[0] = NUM_CHANNELS;
	fwrite(b, 2, 1, f);

	// SampleRate - 4 bytes (in this case, 44000Hz)
	a[0] = SAMPLE_RATE;
	fwrite(a, 4, 1, f);

	// ByteRate - 4 bytes (== SampleRate*NumChannels*BitsPerSample/8 == 176000)
	a[0] = SAMPLE_RATE * NUM_CHANNELS * BIT_DEPTH / 8;
	fwrite(a, 4, 1, f);

	// BlockAlign - 2 bytes (== NumChannels*BitsPerSample/8 == 4)
	// (number of bytes per sample, including both (for stereo) channels)
	b[0] = NUM_CHANNELS * BIT_DEPTH / 8;
	fwrite(b, 2, 1, f);

	// BitsPerSample - 2 bytes (in this case, 16)
	b[0] = BIT_DEPTH;
	fwrite(b, 2, 1, f);
}

void write_data_header(FILE *f, unsigned long n) {
	// Subchunk2ID - 4 bytes
	fputs("data", f); // (0x666d7420 big-endian form)

	// Subchunk2Size - 4 bytes (== NumSamples*NumChannels*BitsPerSample/8)
	// (number of bytes in the data)
	unsigned int a[1]; a[0] = (unsigned int) n;
	fwrite(a, 4, 1, f);
}
