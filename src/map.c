/***
Loading and drawing maps

****/

#include "cutedemo.h"

word map_offset = 0;
int map_offset_Endless = 21;
int map_Update_DownScroll = 0;
byte scene1_bug = 0;
int current_x = 0;
int last_x = 0;
int current_y = 0;
int last_y = 0;

// One map in ram stored at "LT_map"
// One tileset in ram stored at "LT_tileset"
// One map in ram stored at "LT_map"
word map_width;
word map_height;
word map_ntiles;
byte map_data[32*256] = {0};
FILE *Roto_Video;
unsigned char *video_file_name = "maps/rot_ani2.bmp";
word video_file_handle;
byte roto_video_data[72*56] = {0};	//Store one frame
unsigned char Roto_Video_frame = 0;

// One tileset 
word tileset_width;
word tileset_height;
word tileset_ntiles;
byte tileset_palette[256*3];
byte *tileset_data;
unsigned char far *tile_tempdata; //Temp storage of non tiled data.
word reduced_colunm_mode = 5;

extern word paloffset_A;
extern word paloffset_B;
extern word paloffset_C;
extern word Part;

void Load_Tiles(char *file, int window){
	word VGA_index = 0;
	word w = 0;
	word h = 0;
	word ty = 0;
	word tx = 0;
	int jx = 0;
	word x = 0;
	//word y = 0;
	word tileX = 0;
	word tileY = 0;
	word num_colors = 0;
	byte plane = 0;
	word index = 0;
	word _offset = 0;
	word pla[] = {0x0102,0x0202,0x0402,0x0802};
	
	FILE *fp = fopen(file,"rb");
	
	fgetc(fp);
	fgetc(fp);

	fseek(fp, 16, SEEK_CUR);
	fread(&tileset_width, sizeof(word), 1, fp);
	fseek(fp, 2, SEEK_CUR);
	fread(&tileset_height,sizeof(word), 1, fp);
	fseek(fp, 22, SEEK_CUR);
	fread(&num_colors,sizeof(word), 1, fp);
	fseek(fp, 6, SEEK_CUR);

	if (num_colors==0) num_colors=256;
	for(index=0;index<num_colors;index++){
		tileset_palette[(int)(index*3+2)] = fgetc(fp) >> 2;
		tileset_palette[(int)(index*3+1)] = fgetc(fp) >> 2;
		tileset_palette[(int)(index*3+0)] = fgetc(fp) >> 2;
		fgetc(fp);
	}

	tileset_ntiles = (tileset_width>>4) * (tileset_height>>4);
	//LOAD TO TEMP RAM
	if (tileset_ntiles > 240)
		fread(&tile_tempdata[0],sizeof(unsigned char), (256*256)-1, fp);
	else
		fread(&tile_tempdata[0],sizeof(unsigned char), tileset_width*tileset_height, fp);
	
	fclose(fp);

	//COPY TO VGA VRAM
	if (Scene == 27) TILE_VRAM = 0xBBB0; //This way I could load all credits tiles
	else TILE_VRAM = 0xBFF0;
	w = tileset_width>>4;
	if (window) h = 2;
	else h = tileset_height>>4;

	jx = tileset_width+16; 
	
	for (plane = 0; plane < 4; plane ++){
		word pl = pla[plane];
		// select plane
		asm mov dx, 03C4h
		asm mov ax, pl	//plane
		asm out dx, ax

		if (window) VGA_index = TILE_VRAM+224*64;
		else VGA_index = TILE_VRAM;
		//SCAN ALL TILES
		asm mov ax,h
		asm mov tileY,ax
		copyloop0:
			ty = (tileY<<4)-1;
			tx = 0;
			asm mov ax,w
			asm mov tileX,ax
			copyloop1:
				_offset = plane + (ty*tileset_width) + (tx<<4);
				tx++;
				//LOAD TILE
				asm mov [x],0
				asm mov cx,64
				copyloop2:
					VGA[VGA_index] = tile_tempdata[_offset];
					
					asm add [VGA_index],1
					asm add [_offset],4
					asm add [x],1
					asm cmp x,4
					asm jnz lskip
					asm mov x,0
					asm mov bx,jx
					asm sub _offset,bx
					lskip:
					asm loop copyloop2
					
				asm sub tileX,1
				asm mov ax,tileX
				asm jnz copyloop1
		asm sub tileY,1
		asm mov ax,tileY
		asm jnz copyloop0
	}
	
	paloffset_A = 0;
	if (Scene == 13) paloffset_A = 42;
	if (Scene == 27) paloffset_A = 64*3;
	paloffset_B = 16*3;
	paloffset_C = 0;
}

void draw_map_column(word x, word y, word map_offset){
	word TILE_ADDRESS = TILE_VRAM;
	word screen_offset = (y<<6)+(y<<4)+(y<<2)+(x>>2);
	word width = map_width;
	unsigned char *mapdata = map_data;
	word tilesn = 16;
	asm{
		push ds
		push di
		push si
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 
		
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		di,screen_offset		//es:di screen address							
	}
	//UNWRAPPED LOOP
	//DRAW 16 TILES
	
	loopc0:
	asm{//COPY TILE
		mov 	ax,0A000h
		mov 	es,ax		//es:di screen address
		
		mov 	cx,4		//COPY TILE (16 LINES)
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80	//END COPY TILE
		
		//SET ADDRESS FOR NEXT TILE
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		mov		ax,map_offset
		add		ax,[width]
		mov		map_offset,ax
		
		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		ax,[tilesn]
		dec		ax
		mov		[tilesn],ax
		jz		loopc
		jmp		loopc0
		
	}	
	loopc:
	asm{//END
	
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax
		
		pop si
		pop di
		pop ds
	}
}

void draw_ship_map_column(word x, word y, word map_offset){
	word TILE_ADDRESS = TILE_VRAM;
	word screen_offset = (y<<6)+(y<<4)+(y<<2)+(x>>2);
	word width = map_width;
	unsigned char *mapdata = map_data;
	word tilesn = 6;
	asm{
		push ds
		push di
		push si
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 
		
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		di,screen_offset		//es:di screen address							
	}
	//UNWRAPPED LOOP
	//DRAW TILES
	loops0:
	asm{//COPY TILE
		mov 	ax,0A000h
		mov 	es,ax		//es:di screen address
		
		mov 	cx,4		//COPY TILE (16 LINES)
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80	//END COPY TILE
		
		//SET ADDRESS FOR NEXT TILE
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		mov		ax,map_offset
		add		ax,[width]
		mov		map_offset,ax
		
		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		ax,[tilesn]
		dec		ax
		mov		[tilesn],ax
		jz		loops
		jmp		loops0
	}
	loops:
	asm{//END
	
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax
		
		pop si
		pop di
		pop ds
	}
}

void draw_land_map_column(word x, word y, word map_offset){
	word TILE_ADDRESS = TILE_VRAM;
	word screen_offset = (y<<6)+(y<<4)+(y<<2)+(x>>2);
	word width = map_width;
	unsigned char *mapdata = map_data;
	word tilesn = 11;
	asm{
		push ds
		push di
		push si
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 
		
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		di,screen_offset		//es:di screen address							
	}
	//UNWRAPPED LOOP
	//DRAW TILES
	loopl0:
	asm{//COPY TILE
		mov 	ax,0A000h
		mov 	es,ax		//es:di screen address
		
		mov 	cx,4		//COPY TILE (16 LINES)
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80	//END COPY TILE
		
		//SET ADDRESS FOR NEXT TILE
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		mov		ax,map_offset
		add		ax,[width]
		mov		map_offset,ax
		
		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		ax,[tilesn]
		dec		ax
		mov		[tilesn],ax
		jz		loopl
		jmp		loopl0
	}
	loopl:
	asm{//END
	
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax
		
		pop si
		pop di
		pop ds
	}
}

void draw_good_ship(word x, word y, word first_tile){
	word TILE_ADDRESS = TILE_VRAM;
	word screen_offset = (y<<6)+(y<<4)+(y<<2) + (x>>2);
	
	asm{//SET ADDRESS
		push ds
		push di
		push si
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 
		
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		mov		al,byte ptr first_tile
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov 	ax,0A000h
		mov 	es,ax
		mov		di,screen_offset			//es:di screen address								
	}
	//UNWRAPPED LOOPS
	//Copy
	asm{//COPY TILE	
		mov 	cx,24		//COPY line
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
	}
	asm{//COPY TILE	
		mov 	cx,24		//COPY line
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
	}
	asm{//COPY TILE	
		mov 	cx,24		//COPY line
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
	}
	asm{//COPY TILE	
		mov 	cx,24		//COPY line
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
	}
	asm{//COPY TILE	
		mov 	cx,24		//COPY line
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
		mov 	cx,24
		rep		movsb				
		add 	di,60
	}
	asm{//END
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax 
	
		pop si
		pop di
		pop ds
	}
}

void draw_map_row( word x, word y, word map_offset){
	word TILE_ADDRESS = TILE_VRAM;
	unsigned char *mapdata = map_data;
	word screen_offset = (y<<6)+(y<<4)+(y<<2) + (x>>2);
	word tiles = 21;
	asm{//SET ADDRESS
		push ds
		push di
		push si
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 
		
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		les		bx,[mapdata]
		add		bx,map_offset
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		mov		di,screen_offset		//es:di screen address								
	}
	//UNWRAPPED LOOPS
	//Copy 21 tiles
	loopr0:
	asm{//COPY TILE	
		mov 	ax,0A000h
		mov 	es,ax
		
		mov 	cx,4		//COPY 16 LINES
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		mov 	cx,4
		rep		movsb				
		add 	di,80
		
		//SET ADDRESS FOR NEXT TILE
		mov 	ax,0A000h
		mov 	ds,ax
		mov		si,TILE_ADDRESS				//ds:si Tile data VRAM address = FIXED VRAM AT scan line 590; 

		mov		ax,map_offset
		inc		ax
		mov		map_offset,ax
		
		les		bx,[mapdata]
		add		bx,ax
		mov		al,byte ptr es:[bx]
		mov		ah,0
		mov		cl,6
		shl		ax,cl
		add		si,ax
		
		sub		di,84*16
		add		di,4						//next horizontal tile position
		
		mov		ax,[tiles]
		dec		ax
		mov		[tiles],ax
		jz		loopr
		jmp		loopr0
	}
	loopr:
	asm{//END
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax 
	
		pop si
		pop di
		pop ds
	}
}

void Load_Map(char *file,int window){ 
	word tile = 0;
	word index = 0;
	char line[128];
	char name[64]; //name of the layer in TILED
	int i = 0;
	int j = 0;
	
	FILE *f = fopen(file, "rb");
	
	//read file
	fseek(f, 0, SEEK_SET);	
	//skip 6 lines
	for (i = 0; i < 7; i++) fgets(line, 128, f);
	sscanf(line," <layer id=\"%i\" name=\"%24[^\"]\" width=\"%i\" height=\"%i\"",&map_width,&name,&map_width,&map_height);	
	map_ntiles = map_width*map_height;
	
	fgets(line, 64, f); //skip line: <data encoding="csv">

	//read tile array
	for (index = 0; index < map_ntiles; index++){
		fscanf(f, "%i,", &tile);
		map_data[index] = tile-1;
	}
	
	fclose(f);
	
	if (!window){
		map_offset = 0;
		map_offset_Endless = 21;
		SCR_X = 0;
		SCR_Y = 0;
		current_x = 0;
		current_y = 0;
		if (Scene == 1){
			for (i = 0;i<336;i+=16){draw_map_column(SCR_X+i,SCR_Y+240,map_offset+(15*map_width)+j);j++;}
			scene1_bug = 240;
		} else {
			for (i = 0;i<336;i+=16){draw_map_column(SCR_X+i,SCR_Y+240,map_offset+j);j++;}
			scene1_bug = 0;
		}
		if (Part != 3)VGA_Set_palette(tileset_palette);//in 8086 scene, part 3, leave palette alone
	}else{
		map_offset = 0;
		for (i = 0;i<240;i+=16){draw_map_row(0,i,map_offset+(j*map_width));j++;}
	}
}

void Scroll_map(){
	
	current_x = (SCR_X>>4)<<4;
	current_y = ((SCR_Y+240)>>4)<<4;
	
	if (current_x != last_x) {
		map_offset = (((SCR_Y+scene1_bug)>>4)*map_width)+(SCR_X>>4);
		if (current_x < last_x) 
			draw_map_column(current_x,current_y,map_offset); 
		else  
			draw_map_column(current_x+320,current_y,map_offset+20);
	}
	if (current_y != last_y) {
		map_offset = (((SCR_Y+scene1_bug)>>4)*map_width)+(SCR_X>>4);
		if (current_y < last_y)
			draw_map_row(current_x,current_y,map_offset);
		else 
			draw_map_row(current_x,current_y+240,map_offset+(15*map_width));
	}
	last_x = current_x;
	last_y = current_y;
}

void Endless_DownScroll_map(){
	current_y = (SCR_Y>>4)<<4;
	if (current_y != last_y) map_Update_DownScroll = 1;
	
	//Update in two frames to give time to poor 8086
	if (map_Update_DownScroll == 2){
		draw_map_row(0,current_y+240+240,map_offset+20);
		/*if (current_y !=0) */map_offset += 20;
		map_Update_DownScroll = 0;
	}
	if (map_Update_DownScroll == 1){
		if (map_offset == 20*60) map_offset = 0;
		draw_map_row(0,current_y+240-16,map_offset);
		map_Update_DownScroll = 2;
	}
	last_y = current_y;
}

void Endless_SideScroll_Map(int y){
	current_x = (SCR_X>>4)<<4;
	current_y = ((SCR_Y+240)>>4)<<4;
	if (current_x > last_x) { 
		if (Scene == 23)draw_ship_map_column(current_x+320,current_y,((map_offset_Endless+20)%map_width)+(y*map_width));
		else if (Scene == 6) draw_land_map_column(current_x+320,current_y,((map_offset_Endless+20)%map_width)+(y*map_width));
		else draw_map_column(current_x+320,current_y,((map_offset_Endless+20)%map_width)+(y*map_width));
		map_offset_Endless++;
	}
	last_x = current_x;
}

// load_8x8 fonts to VRAM (31 characters)
void Load_Font(char *file){
	word VGA_index = 0;
	word w = 0;
	word h = 0;
	word ty = 0;
	int jx = 0;
	word x = 0;
	word y = 0;
	word tileX = 0;
	word tileY = 0;
	word num_colors = 0;
	byte plane = 0;
	dword index = 0;
	dword offset = 0;
	FILE *fp;
	
	fp = fopen(file,"rb");
	
	//if(!fp)LT_Error("Can't find ",file);
	
	fgetc(fp);
	fgetc(fp);

	fseek(fp, 16, SEEK_CUR);
	fread(&tileset_width, sizeof(word), 1, fp);
	fseek(fp, 2, SEEK_CUR);
	fread(&tileset_height,sizeof(word), 1, fp);
	fseek(fp, 22, SEEK_CUR);
	fread(&num_colors,sizeof(word), 1, fp);
	fseek(fp, 6, SEEK_CUR);

	if (num_colors==0) num_colors=256;
	for(index=0;index<num_colors;index++){
		fgetc(fp);
		fgetc(fp);
		fgetc(fp);
		fgetc(fp);
	}

	//LOAD TO TEMP RAM
	fread(&tile_tempdata[0],sizeof(unsigned char), tileset_width*tileset_height, fp);
	
	//COPY TO VRAM
	w = tileset_width>>3;
	h = tileset_height>>3;
	jx = tileset_width+8; 
	
	for (plane = 0; plane < 4; plane ++){
		// select plane

		outp(SC_INDEX, MAP_MASK);          
		outp(SC_DATA, 1 << plane);
		VGA_index = FONT_VRAM;	//VRAM FONT ADDRESS
		
		//SCAN ALL TILES
		for (tileY = h; tileY > 0 ; tileY--){
			ty = (tileY<<3)-1;
			for (tileX = 0; tileX < w; tileX++){
				offset = plane + (ty*tileset_width) + (tileX<<3);
				//LOAD TILE
				x=0;
				for(y = 0; y < 16; y++){
					VGA[VGA_index] = tile_tempdata[offset];
					VGA_index++;
					offset +=4;
					x++;
					if (x == 2){
						x = 0;
						offset -= jx;
					}
				}
			}
		}
	}
	map_offset = 0;
		map_offset_Endless = 20;
		SCR_X = 0;
		SCR_Y = 0;
		current_x = 0;
		current_y = 0;
}

void Set_FontWin_Color(){
	char *pal = &tileset_palette[250*3];
	VGA_Set_palette_to_black();
	pal[0] = 23; pal[1] = 23; pal[2] = 63;
	pal[3] = 63; pal[4] = 63; pal[5] = 63;
	pal[6] = 49; pal[7] = 49; pal[8] = 49;
	pal[9] = 35; pal[10] = 35; pal[11] = 35;
	pal[12] = 17; pal[13] = 17; pal[14] = 17;
	pal[15] = 0; pal[16] = 0; pal[17] = 0;
}

//Print 8x8 tiles, it is a bit slow on 8086, but it works for text boxes
void Print(word x, word y, char *string, byte win){
	word FONT_ADDRESS = FONT_VRAM;
	word screen_offset;
	byte datastring;
	word size = strlen(string);
	byte i = 0;
	if (!win) y = (y<<3)+240;
	else y = (y<<3);
	screen_offset = (y<<6)+(y<<4)+(y<<2);
	if (size > 40) size = 40;
	asm{
		push ds
		push di
		push si
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 
		//
		mov	di,screen_offset
		mov ax,x
		shl	ax,1
		add di,ax
		mov ax,0A000h
		mov ds,ax
		mov bx,size
	}
	printloop3:
	asm push bx
	datastring = string[i];
	asm{
		mov		dx,word ptr datastring
		sub		dx,32
		
		mov		si,FONT_ADDRESS;			//ds:si VRAM FONT TILE ADDRESS
		
		//go to desired tile
		mov		cl,4						//dx*16
		shl		dx,cl
		add		si,dx
		
		mov 	ax,0A000h
		mov 	es,ax						//es:di destination address	

		//UNWRAPPED COPY 8x8 TILE LOOP
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82
		mov 	cx,2
		rep		movsb				
		add 	di,82 //END LOOP
		
		sub		di,670
	}
		i++;
	asm pop 	bx
	asm dec		bx
	asm jnz		printloop3
		
	asm{//END
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax 
		
		pop si
		pop di
		pop ds
	}
}

//Load an inage to RAM "as is" (for the rotozomm and the big rocket)
void Load_Image(char *file){
	int i;
	word num_colors = 0;
	word index = 0;
	byte A = 0;
	FILE *Image = fopen(file,"rb");
	
	fgetc(Image);
	fgetc(Image);
	fseek(Image, 16, SEEK_CUR);
	fread(&tileset_width, sizeof(word), 1, Image);
	fseek(Image, 2, SEEK_CUR);
	fread(&tileset_height,sizeof(word), 1, Image);
	fseek(Image, 22, SEEK_CUR);
	fread(&num_colors,sizeof(word), 1, Image);
	fseek(Image, 6, SEEK_CUR);
	if (num_colors==0) num_colors=256;
	//Do not get colors in rocket scene
	for(index=0;index<num_colors;index++){
		tileset_palette[(int)(index*3+2)] = fgetc(Image) >> 2;
		tileset_palette[(int)(index*3+1)] = fgetc(Image) >> 2;
		tileset_palette[(int)(index*3+0)] = fgetc(Image) >> 2;
		fgetc(Image);
	}
	fseek(Image,1078, SEEK_SET);
	
	if ((tileset_width == 256) && (tileset_height == 256)) A = 1;
	
	//ROTOZOOM SCENE
	if (Scene == 19){
		VGA_Set_palette(tileset_palette);
		asm mov dx,SC_INDEX //dx = indexregister
		asm mov ax,0F02h	//INDEX = MASK MAP, 
		asm out dx,ax 		//write all the bitplanes.
		for (i = 0; i<(84*60);i++) VGA[(240*84)+i] = 0;
		tile_tempdata[256*256] = 0x3f;
	}

	fread(tile_tempdata,1,(tileset_width*tileset_height)-A,Image);


	fclose(Image);	
	
	
/*
	//OPEN BMP FILE AGAIN, NOW to be read at run time
	asm mov ah, 3Dh
	asm mov al, 2			// open attribute: 0 - read-only, 1 - write-only, 2 -read&write
	asm mov dx,word ptr video_file_name 	// ASCIIZ filename to open
	asm int 21h
	asm mov video_file_handle,ax
	
	//POINT TO PIXEL DATA
	asm mov ah,42h
	asm mov al,0
	asm mov bx,video_file_handle
	asm mov cx,0
	asm mov dx,0436h
	asm int 21h
	
	//READ FRAME
	asm mov ah, 3Fh
	asm mov bx, video_file_handle 	//handle we get when opening a file
	asm mov cx, 72*56 				//number of bytes to read
	asm lds dx, dword ptr rdata		//were to put read data
	asm int 21h
	
	asm mov ah,3eh
	asm mov bx,video_file_handle
	asm int 21h
	*/

}














