/***
Loading and drawing sprites

****/

#include "cutedemo.h"

SPRITE *sprite;
word sprite_bkg[13] = {	
	//8x8
	0xB4E8,0xB538,0xB588,0xB5A0,
	//16x16
	0xB5B8,0xB608,0xB658,0xB6A8,
	//32x32
	0xB6F8,0xB818,0xB938,0xBA58,
	//64x64 Not used, too slow for 8086
	0x0000
};
	
//load sprites with transparency (size = 8,16,32,64!)
void Load_Sprite(char *file, int sprite_number, byte size){
	SPRITE *s = &sprite[sprite_number];
	long index,offset;
	word num_colors;
	word x,y;
	word i,j;
	word frame = 0;
	word fsize = 0;
	byte tileX;
	byte tileY;
	int siz = 0;
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
		x=fgetc(fp);
	}

	for(index=(s->height-1)*s->width;index>=0;index-=s->width){
		for(x=0;x<s->width;x++){
			tile_tempdata[index+x]=(byte)fgetc(fp);
		}
	}
	fclose(fp);
	
	index = 16384*2; //use a chunk of temp allocated RAM to rearrange the sprite frames
	//Rearrange sprite frames one after another in temp memory
	for (tileY=0;tileY<s->height;tileY+=size){
		for (tileX=0;tileX<s->width;tileX+=size){
			offset = (tileY*s->width)+tileX;
			tile_tempdata[index] = size;
			tile_tempdata[index+1] = size;
			index+=2;
			for(x=0;x<size;x++){
				memcpy(&tile_tempdata[index],&tile_tempdata[offset+(x*s->width)],size);
				index+=size;
			}
		}
	}
	
	if (size == 8) siz = 3;
	if (size == 16) siz = 4;
	if (size == 32) siz = 5;
	s->nframes = (s->width>>siz) * (s->height>>siz);
	//Estimated size
	fsize = ((size * size * 7) >> 1) + 25;
	
	//calculate frames size
	if ((s->frames = farcalloc(s->nframes,sizeof(SPRITEFRAME))) == NULL) ;//LT_Error("Error loading ",file);
	
	for (frame = 0; frame < s->nframes; frame++){
		if ((s->frames[frame].compiled_code = farcalloc(fsize,sizeof(unsigned char))) == NULL){
			VGA_mode_text();
			printf("Not enough RAM to allocate sprite frames");
			sleep(3);
			exit(1);
		}
		//COMPILE SPRITE FRAME TO X86 MACHINE CODE		
		code_size = x_compile_bitmap(84, &tile_tempdata[(16384*2)+(frame*2)+(frame*(size*size))],s->frames[frame].compiled_code);
		s->frames[frame].compiled_code = farrealloc(s->frames[frame].compiled_code, code_size);
	}
	//IINIT SPRITE
	//s->bkg_data set in init function (sys.c)
	s->width = size;
	s->height = size;
	s->init = 0;
	s->frame = 0;
	s->baseframe = 0;
	s->ground = 0;
	s->jump = 2;
	s->jump_frame = 0;
	s->climb = 0;
	s->animate = 0;
	s->anim_speed = 0;
	s->anim_counter = 0;
	s->speed_x = 0;
	s->speed_y = 0;
	s->s_x = 0;
	s->s_y = 0;
	s->s_delete = 0;
	s->misc = 0;
	s->mspeed_x = 1;
	s->mspeed_y = 1;
	s->size = s->height>>3;
	s->siz = (s->height>>2) + 1;
	s->next_scanline = 84 - s->siz;
	s->get_item = 0;
	s->mode = 0;
}

void Clone_Sprite(int sprite_number_c,int sprite_number){
	SPRITE *c = &sprite[sprite_number_c];
	SPRITE *s = &sprite[sprite_number];
	int j;
	c->nframes = s->nframes;
	c->width = s->width;
	c->height = s->height;
	c->frames = s->frames;
	for(j=0;j<c->nframes;j++) c->frames[j].compiled_code = s->frames[j].compiled_code;
	//sprite[sprite_number_c].bkg_data  //allocated in lt_sys
	c->init = 0;
	c->frame = 0;
	c->baseframe = 0;
	c->ground = 0;
	c->jump = 2;
	c->jump_frame = 0;
	c->climb = 0;
	c->animate = 0;
	c->anim_speed = 0;
	c->anim_counter = 0;
	c->mspeed_x = 1;
	c->mspeed_y = 1;
	c->speed_x = 0;
	c->speed_y = 0;
	c->s_x = 0;
	c->s_y = 0;
	c->s_delete = 0;
	c->misc = 0;
	c->size = c->height>>3;
	c->siz = (c->height>>2) + 1;
	c->next_scanline = 84 - c->siz;
	c->get_item = 0;
	c->mode = 0;
}

byte CMask[] = {0x11,0x22,0x44,0x88};
void run_compiled_sprite(word XPos, word YPos, char *Sprite){
	asm{
		push si
		push ds
	
		mov ax, 84 //width
		mul word ptr [YPos] // height in bytes
		mov si, [XPos]
		mov bx, si
		mov	cl,2
		shr	si,cl
		add si,ax
		add si, 128         // (Xpos / 4) + 128 ==> starting pos
		and bx, 3
		mov ah,byte ptr CMask[bx]
		mov dx, 03c4h
		mov al, 02h
		out dx, ax
		inc dx              //ready to send out other masks as bytes
		mov al, ah
	
		mov bx, 0a000h
		mov ds, bx          // We do this so the compiled shape won't need  segment overrides.
	
		call dword ptr [Sprite] //the business end of the routine
		
		pop ds
		pop si
	}
}

void Set_Sprite_Animation(int sprite_number, byte baseframe, byte frames, byte speed){
	SPRITE *s = &sprite[sprite_number];
	s->baseframe = baseframe;
	s->aframes = frames;
	s->speed = speed;
	s->animate = 1;
}

//Draw
void Draw_Sprite(int sprite_number){
	//DRAW SPRITE
	SPRITE *s = &sprite[sprite_number];
	int x = s->pos_x;
	int y = s->pos_y+240;
	
	//animation
	if (s->animate == 1){
		s->frame = s->baseframe + s->anim_counter;
		if (s->anim_speed == s->speed){
			s->anim_speed = 0;
			s->anim_counter++;
			if (s->anim_counter == s->aframes) s->anim_counter = 0;
		}
		s->anim_speed++;
	}
	
	//draw sprite and destroy bkg
	run_compiled_sprite(x,y,s->frames[s->frame].compiled_code);
	
	s->last_x = x;
	s->last_y = y;
}

void Clear_Sprite(int sprite_number){
	//RESTORE SPRITE BKG
	SPRITE *s = &sprite[sprite_number];
	int lx = s->last_x;
	int ly = s->last_y;
	int x = s->pos_x;
	int y = s->pos_y+240;
	word screen_offset1 = (y<<6)+(y<<4)+(y<<2)+(x>>2);
	word screen_offset0 = (ly<<6)+(ly<<4)+(ly<<2)+(lx>>2);
	word bkg_data = sprite_bkg[sprite_number];
	word init = s->init;
	word size = s->size;
	word siz = s->siz;
	word next_scanline = s->next_scanline;
	
	///restore destroyed bkg chunk in last frame
	asm{
		
		push bx
		push ds
		push si
		push di
		
		mov dx,SC_INDEX //dx = indexregister
		mov ax,0F02h	//INDEX = MASK MAP, 
		out dx,ax 		//write all the bitplanes.
		mov dx,GC_INDEX //dx = indexregister
		mov ax,008h		
		out dx,ax 		
		
		mov		bx,next_scanline
		mov		dx,siz
		
		cmp	byte ptr [init],1 //if (sprite.init == 1)
		jne	short rle_noinit
		
		mov 	ax,0A000h
		mov 	es,ax						
		mov		di,screen_offset0	//es:di destination vram				
		
		mov		ds,ax						
		mov		si,bkg_data			//ds:si source vram				
		mov 	ax,size
	}
	bkg_scanline1:
	asm{
		mov 	cx,dx				//
		rep		movsb				// copy bytes from ds:si to es:di
		add 	di,bx
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		
		mov 	cx,dx
		rep		movsb
		add 	di,bx
		
		dec 	ax
		jnz		bkg_scanline1
	}
	
	rle_noinit:

	//Copy bkg chunk to a reserved VRAM part, before destroying it
	asm{
		mov 	ax,0A000h
		mov 	ds,ax						
		mov		si,screen_offset1	//ds:si source vram			
		
		mov 	es,ax						
		mov		di,bkg_data			//es:di destination
		
		mov		ax,size				//Scanlines
	}	
	asm{
		mov 	cx,dx				// copy width + 4 pixels
		rep		movsb				// copy bytes from ds:si to es:di
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		
		dec 	ax
		jz		bkg_scanline2
		
		mov 	cx,dx				// copy width + 4 pixels
		rep		movsb				// copy bytes from ds:si to es:di
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx	

		dec 	ax
		jz		bkg_scanline2
		
		mov 	cx,dx				// copy width + 4 pixels
		rep		movsb				// copy bytes from ds:si to es:di
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx	
		
		mov 	cx,dx				// copy width + 4 pixels
		rep		movsb				// copy bytes from ds:si to es:di
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
		add 	si,bx
		mov 	cx,dx
		rep		movsb
	}
	bkg_scanline2:
	asm{
		mov dx,GC_INDEX +1 //dx = indexregister
		mov ax,00ffh		
		out dx,ax 
		
		pop di
		pop si
		pop ds
		pop bx

	}
	s->init = 1;
}

//65 675
void Unload_sprite(int sprite_number){
	SPRITE *s = &sprite[sprite_number];
	int i;
	s->init = 0;
	for (i=0;i<s->nframes;i++){
		farfree(s->frames[i].compiled_code);
		s->frames[i].compiled_code = NULL;
	}
	farfree(s->frames); s->frames = NULL;
}	

//Draw Big Rocket
void Draw_Big_Rocket(word x, word y, word frame){
	word screen_offset = (y<<6)+(y<<4)+(y<<2)+(x>>2);
	asm{
	push ds
	push di
	push si
	
	mov dx,003c4h
	mov ax,0F02h
	out dx,ax 		//write all the bitplanes.
	//mov dx,003ceh
	//mov ax,008h		
	//out dx,ax
	
	lds		si,dword ptr tile_tempdata //ds:si RAM address;
	add		si,frame
	
	mov 	ax,0A000h
	mov 	es,ax
	mov		di,84*240			//es:di screen address
	add		di,screen_offset
	
	mov 	ax,78
	}
	rocket_loop0:
	asm{
		mov	cx,12
		rep movsw
		add di,84-24
		dec	ax
		jnz rocket_loop0		

	pop si
	pop di
	pop ds
	}
}







