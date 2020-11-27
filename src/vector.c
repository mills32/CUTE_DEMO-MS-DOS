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

