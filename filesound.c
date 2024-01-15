/* filesound: convert arbitrary files to wave audio files. (DISCLAIMER: it's a little bit hacky, but that's good enough for now!) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define SAMPLE_RATE 44000
#define NUM_CHANNELS 2
#define BIT_DEPTH 16
/* two bytes per sample; four bytes per stereo sample
 * (NUM_CHANNELS * BIT_DEPTH / 8 == 4) */

/* parameters for sine tone output */
#define SINE_TIME_SCALE 0.001
#define BYTES_PER_SECOND 128
#define SINE_AMP_SCALE 32767

#define OFF 0
#define ON 1

/* NOTE: Subchunk2Size (the size of the actual data in bytes) is 4 bytes (stored as an unsigned int)
 * so WAVE files are limited to a maximum file size of (2^32 - 1) bytes: approx 4.3 gigabytes. */

void parse_args(char*[], char**, char**, int*, char*);
unsigned long copy_bytes(FILE*, FILE*, int);
unsigned long write_bytes_sine(FILE*, FILE*, int);
unsigned long write_sine_stereo(FILE*, FILE*, int);
void write_riff_header(FILE*, int);
void write_fmt_subchunk(FILE*);
void write_data_header(FILE*, unsigned long);

int main(int argc, char *argv[]) {

	// Parse arguments
	char sine_flag = OFF;
	int stretch_factor = 1;
	char *srcfn, *tgtfn;
	parse_args(argv, &srcfn, &tgtfn, &stretch_factor, &sine_flag);

	// Parse arguments (OLD) --------------------------------------------------
	/* int stretch_factor = 1;
	char *srcfn, *tgtfn;
	srcfn = tgtfn = NULL;
	if (argc == 4) {
		srcfn = argv[1];
		tgtfn = argv[2];
		stretch_factor = atoi(argv[3]);

	} else if (argc == 3) {
		srcfn = argv[1];
		if (isdigit((int) *argv[2]))
			stretch_factor = atoi(argv[2]);
		else
			tgtfn = argv[2];

	} else if (argc == 2) {
		if (isdigit((int) *argv[1]))
			stretch_factor = atoi(argv[1]);
		else
			srcfn = argv[1];

		tgtfn = srcfn;

	} else if (argc == 1) {
		// We'll go stdin --> stdout, with stretch factor 1.
		
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments.\n");

	} else {
		// Should never happen!
		fprintf(stderr, "Something really weird happened.\n");
	} */

	// THE ABOVE IS OLD ------------------------------------------------------
	
	// Open source file
	FILE *src;
	if (srcfn != NULL)
		src = fopen(srcfn, "rb");
	else
		// We'll read from stdin
		src = stdin;

	if (src == NULL) {
		fprintf(stderr, "I can't find the file '%s'. :(\n", argv[1]);
		return 1;
	}

	// Open target file
	FILE *tgt;
	if (tgtfn != NULL) {
		strcat(tgtfn, ".wav");
		tgt = fopen(tgtfn, "wb");
	} else
		// We'll write to stdout
		tgt = stdout;

	if (tgt == NULL) {
		fprintf(stderr, "Failed to open output file.\n");
		return 1;
	}

	// Seek to data chunk location (44 bytes in)
	fseek(tgt, 44, SEEK_SET);

	// Either write raw data or sine tones, depending on presence of -s flag
	unsigned long byte_count;
	if (sine_flag == ON) {
		byte_count = write_bytes_sine(src, tgt, SAMPLE_RATE / BYTES_PER_SECOND);
	} else {
		byte_count = copy_bytes(src, tgt, stretch_factor);
	}
	
	// Seek back to the beginning again
	fseek(tgt, 0, SEEK_SET);

	// Write RIFF subchunk header
	write_riff_header(tgt, byte_count);

	// Write fmt subchunk header
	write_fmt_subchunk(tgt);

	// Write data subchunk header
	write_data_header(tgt, byte_count);

	// Close files
	fclose(src);
	fclose(tgt);

	int secs = byte_count / (SAMPLE_RATE * NUM_CHANNELS * BIT_DEPTH / 8);
	printf("Wrote %lu bytes (~%d seconds of audio).\n", byte_count, secs);

	return 0;
}

/* parse the command line arguments given to filesound. */
void parse_args(char *argv[], char **srcp, char **tgtp, int *sf, char *sinep) {
	int i = 1;
	char c;
	// deal with flags
	while (argv[i][0] == '-') {
		switch (c = argv[i++][1]) {
			case 's':
				*sinep = ON;
				break;
			default:
				// some other flag
				fprintf(stderr, "ERROR: Unknown flag %c.\n", c);
				break;
		}
	}

	// deal with arguments (filenames and stretch factor)
	if (argv[i++] == NULL) {
		// no more arguments; default to stdin and stdout (just return a null pointer from here)
		*srcp = NULL;
		*tgtp = NULL;
	} else {
		// at least one more argument...
		if (argv[i++] == NULL) {
			// exactly one argument; interpret as stretch_factor if (first char) a digit, else source_file.
			// NOTE: this means we should probably avoid trying to have filenames that start with a digit.
			if (isdigit((int) argv[i - 2][0]))
				*sf = atoi(argv[i - 2]);
			else
				*srcp = argv[i - 2];
				*tgtp = argv[i - 2];
		} else {
			// at least two arguments...
			if (argv[i++] == NULL) {
				// exactly two arguments; interpret as source_file target_file
				*srcp = argv[i - 3];
				*tgtp = argv[i - 2];
			} else {
				// three arguments; interpret as source_file target_file stretch_factor
				*srcp = argv[i - 3];
				*tgtp = argv[i - 2];
				*sf = atoi(argv[i - 1]);
			}
		}
	}
}

// I completely forget that we have argc... so we don't need to do the above to work out how many argument's we've got. :(

/* write bytes from stream pointed to by fi to that pointed to by fo, stretching by factor sf. */
unsigned long copy_bytes(FILE *fi, FILE *fo, int sf) {
	short stereo_sample_bytes = BIT_DEPTH * NUM_CHANNELS / 8;
	char c[stereo_sample_bytes];
	unsigned long i;
	int j, k;
	// -- Write bytes from source to target --
	for (i = 0; !feof(fi); i++) {
		// Collect single stereo sample's worth of bytes
		for (j = 0; j < stereo_sample_bytes; j++) {
			if (feof(fi))
				break;
			c[j] = fgetc(fi);
		}
		// Write this sequence of bytes into file sf many times
		for (j = 0; j < sf; j++)
			for (k = 0; k < stereo_sample_bytes; k++)
				fputc(c[k], fo);
	}

	// return (approx.) number of bytes copied across
	return i * stereo_sample_bytes * sf;
}

/* convert each byte to a sine tone of duration t samples and pitch depending on the byte value. */
unsigned long write_bytes_sine(FILE *fi, FILE *fo, int t) {
	int i, j, k;
	char c;
	short s[1];
	for (i = 0; !feof(fi); i++) {
		// Get a byte
		c = fgetc(fi);
		// Write out sine tone (that lasts for t samples)
		for (j = 0; j < t; j++) {
			for (k = 0; k < NUM_CHANNELS; k++)
				s[0] = (short) (sin(SINE_TIME_SCALE * j * (float) c) * SINE_AMP_SCALE);
				//if (j == 0)
				//	printf("byte: 0x%x\n", c);
				//	printf("amplitude: %hi\n", s[0]);
				//printf("input to sine: %f\n", SINE_SCALE * j * (float) c);
				//printf("j: %d\n\n", j);
				fwrite(s, sizeof(short), 1, fo);
		}
	}
	return i * t *  NUM_CHANNELS * BIT_DEPTH / 8;
}

/* convert each sample-length byte sequence to a sine tone in each channel of duration t */
unsigned long write_sine_stereo(FILE *fi, FILE *fo, int t) {
	return 0.0;
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
