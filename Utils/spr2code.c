// BitMap to asm and / or scanlines for 8086 vga fast animation
// Only 256 colors, and no color space info

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

unsigned char *pixeldata;
unsigned char *buffer;
unsigned char *buffer2;
unsigned char *buffer3;

int main(int argc, char **argv) {
	int i,width,height,num_colors,x,index,index0;
	int linelen = 0;
	int linenum = 0;
	int counting = 0;
	int counter = 0;
	int di = 0;
	unsigned char color = 0;
	unsigned char color1 = 0;
	unsigned char notrans = 0;
	
	FILE *fin;
	FILE *fout;
	FILE *converted;
	fin = fopen("test.bmp", "rb");
	fout = fopen("code.asm", "w");
	converted = fopen("sprite.bmp", "wb");
	
	if (fin == (FILE *)0){
		printf("File opening error ocurred. Exiting program.\n");
		exit(0);
	}

	fseek(fin, 18, SEEK_SET);
	fread(&width, 4, 1, fin);
	fread(&height,4, 1, fin);
	fseek(fin, 20, SEEK_CUR);
	fread(&num_colors,4, 1, fin);
	fseek(fin, 4, SEEK_CUR);

	if (num_colors==0) num_colors=256;

	for(index=0;index<num_colors;index++){
		x = fgetc(fin);
		x = fgetc(fin);
		x = fgetc(fin);
		x = fgetc(fin);
	}
	fprintf(fout,"width %i\nheight %i\ncolors %i\n\n",width,height,num_colors);
	pixeldata = (unsigned char*) calloc(width*height,sizeof(int));
	buffer = (unsigned char*) calloc(1078 + (width*height),sizeof(int));
	buffer2 = (unsigned char*) calloc(width*height,sizeof(int));
	buffer3 = (unsigned char*) calloc(width*height,sizeof(int));
	if (pixeldata == NULL){
		printf("Error allocating memory.\n");
		exit(0);
	}
	
	for(index=(height-1)*width;index>=0;index-=width){
		for(x=0;x<width;x++){
			pixeldata[index+x]=(unsigned char)fgetc(fin);
		}
	}
	
	counting = 1;
	index = 0;
	while (counting){
		while (!pixeldata[index]) {index++;di++;}
		if (di < 1024)fprintf(fout,"add 	di,%i\n",di);
		else fprintf(fout,"\n\n\nEND");
		di = 0;
		while (pixeldata[index]){
			if (pixeldata[index]!=pixeldata[index+1]){
				if (counter == 0) fprintf(fout,"movsb\n");
				else {
					counter++;
					if ((counter/2) != 1)fprintf(fout,"mov	cx,%i\n",counter/2);
					if ((counter/2) != 1)fprintf(fout,"rep	movsw\n");
					if ((counter/2) == 1)fprintf(fout,"movsw\n");
					if(counter % 2)fprintf(fout,"movsb\n");
					counter = 0;
				}
			}
			else counter++;
			index++;
		}
		counter =0;
		if (index > (width*height)) counting = 0;
	}
	fclose(fout);
	fclose(fin);
	free(pixeldata);
}

