// BitMap to Y scanlines for 8086 scanline effect

#include <string.h>
#include <stdio.h>

	
//COLORS FF CB 95 00
int main() {
	unsigned long i;
	int size;
	FILE *fin;
	FILE *fout;
	unsigned char dataR;
	unsigned char dataG;
	unsigned char dataB;
	int array_w = 0;
	int counter = 0;
	fin = fopen("bitmap.dat", "rb");
	fout = fopen("palet.c", "w");
	
	if (!fin){
	printf("File opening error ocurred. Exiting program.\n");
	}
	
	//CONVERT TO Y SCANLINES in output file
	
	fprintf(fout,"//BITMAP to VGA Palette\n\n");
	fprintf(fout,"const unsigned char Bars_Anim[] = {\n");
	
	//CONVERT TO VGA Palette
	array_w = 0;

	for(i = 0; i < 144*5*30*4; i+=4) {
		dataR = fgetc(fin)>>2;
		dataG = fgetc(fin)>>2;
		dataB = fgetc(fin)>>2;
		fprintf(fout,"0x%02X, ",dataR);
		fprintf(fout,"0x%02X, ",dataG);
		fprintf(fout,"0x%02X, ",dataB);
		dataB = fgetc(fin);//Get alpha and discard it
		array_w++;
		if (array_w == 4){
			fprintf(fout,"\n\t");
			array_w = 0;
		}
		counter++;
	}

	fprintf(fout,"\n};");
	
	fclose(fin);
	fclose(fout);
	
	printf("filesize = %d ", size);
	
	return 0;
}

