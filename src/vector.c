/***********************************************************************
Very_fast_line
Really... an 8088 4.77 can draw lines at 60 fps
I took a sample from Michael Abrash's Graphics Programming Black Book
And I optimized it just to draw a wavy sea surface.

Only works if the line is more horizontal than vertical, so in mode x,
as it draws 4 pixels at a time, lines can only have 45/2 inclination.

*/

#include "cutedemo.h"

void Very_fast_line(int x1, int y1, int x2, int y2, byte color){
	int i,_offset;
	int _dx=x2-x1;      /* the horizontal distance of the line */
	int _dy=y2-y1;      /* the vertical distance of the line */
	int sdy=84;
	int y=_dx>>1;
	
	//calculate this outside the loop
	//y1+=240;
	_offset = (y1<<6)+(y1<<4)+(y1<<2) + x1;
	
	//I only check if dy is negative
	asm mov ax,_dy
	asm test ax,0x8000
	asm je is_positive
	asm neg ax
	asm mov _dy,ax
	asm mov sdy,-84
	is_positive:

	//First point
	VGA[_offset]=color;

	//Draw the line, super fast!
	asm mov cx,_dx
	asm shr cx,1 //cx/2
	//Start address
	asm les	si, VGA
	asm mov si,[_offset] 
	asm mov dx, y
	//GO
	lineloop:
		asm add	dx,[_dy]	//y+=_dy;
		asm cmp	dx,[_dx]	//if (y>=_dx){y-=_dx;_offset+=sdy;}
		asm jl lloop
			asm sub dx,[_dx]
			asm add si,sdy
		lloop:
		asm inc si
		asm mov	al,byte ptr [color]//VGA[offset]=color;
		asm mov byte ptr es:[si],al
		
		//Draw another "pixel"
		asm add	dx,[_dy]
		asm cmp	dx,[_dx]
		asm jl lloop1
			asm sub dx,[_dx]
			asm add si,sdy
		lloop1:
		asm inc si
		asm mov	al,byte ptr [color]
		asm mov byte ptr es:[si],al
		
	asm loop lineloop
}


//This is the load sprite function but optimized to load simple images consisting of lines/vectors.
//The x_compile_bitmap function will create machine code for the simple shapes so it will be very fast for 
//the 8086, as long as the image is really simple, and has only lines with color 0 transparent background.

byte *vector_frame;
int alga_framebin;
void Load_Shape(char *file, int sprite_number){
	SPRITE *s = &sprite[sprite_number];
	long index,offset;
	word num_colors;
	word x,y;
	word i,j;
	word frame = 0;
	word fsize = 0;
	byte tileX;
	byte tileY;
	FILE *fp;
    int code_size;
	
	fp = fopen(file,"rb");
	
	//if(!fp) LT_Error("Can't find ",file);
	
	fseek(fp, 2, SEEK_CUR);
	fseek(fp, 16, SEEK_CUR);
	fread(&s->width, sizeof(word), 1, fp);
	fseek(fp, 2, SEEK_CUR);
	fread(&s->height,sizeof(word), 1, fp);
	fseek(fp, 22, SEEK_CUR);
	fread(&num_colors,sizeof(word), 1, fp);
	fseek(fp, 6, SEEK_CUR);

	if (num_colors==0) num_colors=256;

	for(index=0;index<num_colors;index++){
		s->palette[(int)(index*3+2)] = fgetc(fp) >> 2;
		s->palette[(int)(index*3+1)] = fgetc(fp) >> 2;
		s->palette[(int)(index*3+0)] = fgetc(fp) >> 2;
		fsize=fgetc(fp);
	}

	//fread(tile_tempdata,1,s->height*s->width,fp);
	
	fclose(fp);
	
	s->nframes = 16;
	//Estimated size
	fsize = ((32 * 120 * 7) >> 1) + 25;
	fsize = fsize>>2;
	//calculate frames size
	if ((s->frames = farcalloc(s->nframes,sizeof(SPRITEFRAME))) == NULL) ;//LT_Error("Error loading ",file);
	
	for (frame = 0; frame < s->nframes; frame++){
		char buffer[32];
		if ((s->frames[frame].compiled_code = farcalloc(fsize,sizeof(unsigned char))) == NULL){
			VGA_mode_text();
			printf("Not enough RAM to allocate vector frames");
			sleep(3);
			exit(1);
		}
		
		//This code creates machine code to draw the lines
		//it was so slow to load on the 8086, so I stored compiled code in files
		/*vector_frame[0] = 32;
		vector_frame[1] = 120;
		memcpy(&vector_frame[2],&tile_tempdata[frame*(32*120)],32*120);
		//COMPILE LINES TO X86 MACHINE CODE		
		code_size = x_compile_bitmap(84,&vector_frame[0],s->frames[frame].compiled_code);
		s->frames[frame].compiled_code = farrealloc(s->frames[frame].compiled_code, code_size);
	
		sprintf(buffer,"sprites/f-%i-%i.bin",alga_framebin,frame);
		fp = fopen(buffer,"wb");
		fwrite(s->frames[frame].compiled_code,1,code_size,fp);
		fclose(fp);*/
		
		//Read the compiled codes (much faster)
		sprintf(buffer,"sprites/alga/f-%i-%i.bin",alga_framebin,frame);
		fp = fopen(buffer,"rb");
		fread(s->frames[frame].compiled_code,1,fsize,fp);
		fclose(fp);
	}
	alga_framebin++;
	s->frame = 0;
	s->baseframe = 0;
	s->animate = 0;
	s->anim_speed = 0;
	s->anim_counter = 0;
	s->s_x = 0;
	s->s_y = 0;
	s->misc = 0;
}


//Draw
void Draw_Shape(int sprite_number){
	//DRAW SPRITE
	SPRITE *s = &sprite[sprite_number];
	int x = s->pos_x;
	int y = s->pos_y+240;
	//s->frame = 0;

	run_compiled_sprite(x,y,s->frames[s->frame].compiled_code);
	
	s->last_x = x;
	s->last_y = y;
}
