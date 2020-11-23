// BitMap to Y scanlines for 8086 scanline effect

#include <string.h>
#include <stdio.h>
#include <math.h>
float M_PI = 3.1415926;
//COLORS FF CB 95 00
int main() {
	unsigned long i;
	int wide = 0;
	FILE *fout = fopen("sinetables.c", "w");
	
	fprintf(fout,"SINEX\n\n");
	for( i = 0; i < 256; ++i ) {
		int val = 320 * ( (sin( 2.0 * M_PI * i / 256.0 ) + 1.0 ) / 2.0 );
		fprintf(fout,"%i, ",val);
		wide++;
		if (wide == 16) {wide = 0;fprintf(fout,"\n");}
	}
	fprintf(fout,"\n\nSINEY\n\n");
	for( i = 0; i < 256; ++i ) {
		int val = 200 * ( (sin( 2.0 * M_PI * i / 128.0 ) + 1.0 ) / 2.0 );
		fprintf(fout,"%i, ",val);
		wide++;
		if (wide == 16) {wide = 0;fprintf(fout,"\n");}
	}

	fclose(fout);
	
	return 0;
}

