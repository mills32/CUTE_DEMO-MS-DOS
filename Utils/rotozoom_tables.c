// Precompute rotozoom tables

#include <string.h>
#include <stdio.h>
#include <math.h>

int main() {
	unsigned long i;
	float angle = 0;
	float angle1 = 0;
	float zoom = 0;
	unsigned char x = 0;
	unsigned char x1 = 0;
	unsigned char y = 0;
	unsigned char y1 = 0;
	unsigned short A_offset = 0;
	unsigned short B_offset = 0;
	unsigned char Ax = 0;
	unsigned char Ay = 0;
	unsigned char Bx = 0;
	unsigned char By = 0;
	unsigned char Bx1 = 0;
	unsigned char By1 = 0;
	int linelen = 0;
	int size;
	FILE *fout;
	fout = fopen("roto_tables.asm", "w");
	
	fprintf(fout,"\nt0	label	word\n");
	for (angle1 = 0; angle1<360;angle1+=2){
		linelen = 0;
		angle = angle1*3.14/180;
		zoom = (1.3+sin(angle))*2;
		fprintf(fout,"dw ");
		
		for (x = 0;x < 72; x++){
			unsigned short difference = 0;
			//Calculate B
			Bx = (x*cos(angle))*zoom;
			By = (x*sin(angle))*zoom;
			Bx1 = (x+(36*-cos(angle)))*zoom;
			By1 = (x+(36*-sin(angle)))*zoom;
			
			if (x == 0) {//Compute initial position to produce rotation around central point
				A_offset = (By<<8)+Bx;
				B_offset = (By1<<8)+Bx1;
			} else B_offset = (By<<8)+Bx;
	
			//Compute difference with previous point
			difference = (B_offset-A_offset)-1;
			linelen++;
			if (linelen == 15){
				fprintf(fout,"0%02Xh\ndw ",difference);
				linelen = 0;
			}
			else fprintf(fout,"0%02Xh, ",difference);
			
			if (x == 0) B_offset = (By<<8)+Bx;
			//"previous" point
			Ax = Bx;Ay=By;A_offset=B_offset;
		}
		fprintf(fout," 000h, 000h, 000h\n\n");
	}
	
	fprintf(fout,"\nt1	label	word\n");
	for (angle1 = 0; angle1<360;angle1+=2){
		linelen = 0;
		angle = angle1*3.14/180;
		zoom = (1.3+sin(angle))*2;
		fprintf(fout,"dw ");
		for (y = 0;y < 52; y++){
			//Calculate B
			Bx = (y*(-sin(angle)))*zoom;
			By = (y*cos(angle))*zoom;
			Bx1 = (y+(26*sin(angle)))*zoom;
			By1 = (y+(26*-cos(angle)))*zoom;

			if (y == 0){//Compute initial position to produce rotation around central point
				A_offset = (By<<8)+Bx;
				B_offset = (By1<<8)+Bx1;
			} else B_offset = (By<<8)+Bx;

			//Compute difference with previous point
			unsigned short difference = 0;
			difference = B_offset-A_offset;
			linelen++;
			if (linelen == 15){
				fprintf(fout,"0%02Xh\ndw ",difference);
				linelen = 0;
			}
			else fprintf(fout,"0%02Xh, ",difference);
			
			if (y == 0) B_offset = (By<<8)+Bx;
			//"previous" point
			Ax = Bx;Ay=By;A_offset=B_offset;
		}
		fprintf(fout," 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h\n");
		fprintf(fout,"dw 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h\n\n");
	}
	fclose(fout);

	return 0;
}

