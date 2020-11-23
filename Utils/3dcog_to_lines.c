// BitMap to Y scanlines for 8086 scanline effect

#include <string.h>
#include <stdio.h>

	
//COLORS FF CB 95 00
int main() {
	unsigned long i,j,k;
	int size;
	FILE *fin;
	FILE *fout;
	int VRAM = 0xBFF0 + (32*64);
	int VRAMB = 0xBFF0 + (141*64)+(11*4);
	unsigned char dataR;
	unsigned char dataG;
	unsigned char dataB;
	unsigned char dataA;
	int array_w = 0;
	int counter = 0;
	int counter2 = 0;
	int stop_counter = 0;
	fin = fopen("3dcog.data", "rb");
	fout = fopen("3dcog_lines.c", "w");
	
	if (!fin){
	printf("File opening error ocurred. Exiting program.\n");
	}
	
	//CONVERT TO Y SCANLINES in output file
	
	fprintf(fout,"//BITMAP to VGA Palette\n\n");
	fprintf(fout,"const unsigned char Bars_Anim[] = {\n");
	fprintf(fout,"dw ");
	//CONVERT TO VGA Palette
	array_w = 0;

	for (i = 0; i < 60; i++) {
		for (j = 0; j < 132; j++) {
			counter = 0;
			counter2 = 0;
			stop_counter = 0;
			for (k = 0; k < 16; k++) {
				dataR = fgetc(fin);
				dataG = fgetc(fin);
				dataB = fgetc(fin);
				dataA = fgetc(fin);//Get alpha and discard it
				
				if (stop_counter == 0){
					if (dataR > 0xEF) {
						counter2 = dataR & 0x0F;
						fprintf(fout,"0%04Xh, ",VRAM+0+(counter*20*16)+(counter2*20));
						stop_counter = 1;
					}
					if (dataG > 0xEF) {
						counter2 = dataG & 0x0F;
						fprintf(fout,"0%04Xh, ",VRAM+(128*20)+(counter*20*16)+(counter2*20));
						stop_counter = 1;
					}
					if (dataB > 0xEF) {
						counter2 = dataB & 0x0F;
						fprintf(fout,"0%04Xh, ",VRAM+(256*20)+((counter-3)*20*16)+(counter2*20));
						stop_counter = 1;
					}
					counter++;
				}
			}
			
			//if black line
			if (stop_counter == 0) fprintf(fout,"0%04Xh, ",VRAMB);
			
			array_w++;
			if (array_w == 16){
				fseek(fout, -2, SEEK_CUR);
				fprintf(fout,"\ndw ");
				array_w = 0;
			}
		}
	}

	fprintf(fout,"\n};");
	
	fclose(fin);
	fclose(fout);
	
	printf("filesize = %d ", size);
	
	return 0;
}

