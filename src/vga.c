/****
VGA hardware functions

****/

#include "cutedemo.h"

byte *VGA=(byte *)0xA0000000L;        /* this points to video memory. */
word TILE_VRAM = 0xBFF0; //
word FONT_VRAM = 0xBB90; //0xBFF0-(70*16); //
word SCR_X = 0;
word SCR_Y = 0;
word SCR_WY = 240*2;

extern unsigned int SINEX[];
extern unsigned int SINEY[];

extern unsigned char temp_palette[];
extern byte *music_sdata;
extern int sound_mode;

void Clearkb(){
	asm mov ah,00ch
	asm mov al,0
	asm int 21h
}

void VGA_mode_X(){
	union REGS regs;
	
	//Select sound menu 80x25 text mode
	//disable cursor
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, 0x20);
	system("cls");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                  SELECT SOUND                                  ");
	printf("                                                                                ");
	printf("                           0 => ADLIB / OPL2 Compatible                         ");
	printf("                           1 => Gravis Ultrasound                               ");

	while (kbhit() == 0);
	sound_mode = getch() -48;
	system("cls");
	
	if (sound_mode == 0)Adlib_Detect();
	if (sound_mode == 1)Init_GUS();
	if (sound_mode < 2) {
		system("cls");
		printf("\nNo sound card selected.\nSound is disabled.");
		sleep(2);
	}
	
	system("cls");
	VGA_Fade_out();
	system("cls");
	
	regs.h.ah = 0x00;
	regs.h.al = 0x13;
	int86(0x10, &regs, &regs);
	
	outp(SC_INDEX,  MEMORY_MODE);       // turn off chain-4 mode
	outp(SC_DATA,   0x06);
	outport(SC_INDEX, ALL_PLANES);      // set map mask to all 4 planes
	outp(CRTC_INDEX,UNDERLINE_LOCATION);// turn off long mode
	outp(CRTC_DATA, 0x00);
	outp(CRTC_INDEX,MODE_CONTROL);      // turn on byte mode
	outp(CRTC_DATA, 0xe3);
	
	//outp(MISC_OUTPUT, 0x00); //Memory map 64 Kb?
	
	// turn off write protect
	word_out(CRTC_INDEX, V_RETRACE_END, 0x2c);
	
	word_out(CRTC_INDEX, V_TOTAL, 0x0d);
	word_out(CRTC_INDEX, OVERFLOW, 0x3e);
	
	word_out(CRTC_INDEX, V_RETRACE_START, 0xea);
	word_out(CRTC_INDEX, V_RETRACE_END, 0xac);
	word_out(CRTC_INDEX, V_DISPLAY_END, 0xdf);
	word_out(CRTC_INDEX, V_BLANK_START, 0xe7);
	word_out(CRTC_INDEX, V_BLANK_END, 0x06);
	
	//LOGICAL WIDTH = 320 + 16
	word_out(0x03d4,OFFSET,42);
	
	// set vertical retrace back to normal
	word_out(0x03d4, V_RETRACE_END, 0x8e);	
	
	VGA_Set_palette_to_black();
	memset(tileset_palette,0x00,256*3);
	
	if ((tile_tempdata = farcalloc(65535,sizeof(byte))) == NULL){
		VGA_mode_text();
		printf("Not enough RAM to allocate tiles");
		sleep(2);
		system("cls");
		exit(1);
	}
	if ((sprite = farcalloc(13,sizeof(SPRITE))) == NULL){
		VGA_mode_text();
		printf("Not enough RAM to allocate sprites");
		sleep(2);
		system("cls");
		exit(1);
	}
	if ((music_sdata = farcalloc(65535,sizeof(byte))) == NULL){
		VGA_mode_text();
		printf("Not enough RAM to allocate music");
		sleep(2);
		system("cls");
		exit(1);
	}
	if ((vector_frame = farcalloc(4*1024,sizeof(byte))) == NULL){
		VGA_mode_text();
		printf("Not enough RAM to allocate vector_frame");
		sleep(2);
		system("cls");
		exit(1);
	}
	SCR_X = 0;
	SCR_Y = 0;
	
	VGA_Scroll_Vsync();
}

byte p[4] = {0,2,4,6};
byte pix;

void VGA_Vsync(){
	asm mov		dx,INPUT_STATUS_0
	WaitNotVsync0:
	asm in      al,dx
	asm test    al,08h
	asm jnz		WaitNotVsync0
	WaitVsync0:
	asm in      al,dx
	asm test    al,08h
	asm jz		WaitVsync0
}

void VGA_Scroll_Vsync(){
	byte _ac;
	word x = SCR_X;
	word y = SCR_Y+240;
	
	//This is as optimized as it can get wen compiled to asm (I checked)
	y = (y<<6)+(y<<4)+(y<<2) + (x>>2);	//(y*64)+(y*16)+(y*4) = y*84; + x/4
	
	//change scroll registers: LOW_ADDRESS 0x0D; HIGH_ADDRESS 0x0C;
	asm mov dx,003d4h //VGA PORT
	asm mov	cl,8
	asm mov ax,y
	asm	shl ax,cl
	asm	or	ax,00Dh	
	asm out dx,ax	//(y << 8) | 0x0D to VGA port
	asm mov ax,y
	asm	and ax,0FF00h
	asm	or	ax,00Ch
	asm out dx,ax	//(y & 0xFF00) | 0x0C to VGA port
	
	//The smooth panning magic happens here
	
	//Wait Vsync
	asm mov		dx,INPUT_STATUS_0
	WaitNotVsync:
	asm in      al,dx
	asm test    al,08h
	asm jnz		WaitNotVsync
	WaitVsync:
	asm in      al,dx
	asm test    al,08h
	asm jz		WaitVsync
	
	//disable interrupts because we are doing crazy things
	//asm cli
	asm mov		dx,INPUT_STATUS_0 //Read input status, to Reset the VGA flip/flop
	_ac = inp(AC_INDEX);//Store the value of the controller
	pix = p[x & 3]; //VGA

	//AC index 0x03c0
	asm mov dx,003c0h
	asm mov ax,033h
	asm out dx,ax
	asm mov al,byte ptr pix
	asm out dx,al
	//enable interrupts because we finished doing crazy things
	//asm sti
	//Restore controller value
	asm mov ax,word ptr _ac
	asm out dx,ax
	
}

void VGA_SplitScreen(int line){
	line = line<<1;
    asm{
	push    ax
    push    cx
    push    dx
	//Set the split screen scan line.
    //cli		// make sure all the registers get set at once	
	
	mov		dx,CRTC_INDEX
	mov		ax,line
	mov    	cl,8
	shl		ax,cl
	add		ax,LINE_COMPARE
	out    	dx,ax		// set bits 7-0 of the split screen scan line

    mov    ah,byte ptr [line+1]
    and    ah,1
    mov    cl,4
    shl    ah,cl                      // move bit 8 of the split split screen scan line into position for the Overflow reg
    mov    al,OVERFLOW

	//The Split Screen, Overflow, and Line Compare registers all contain part of the split screen start scan line on the VGA
	out    dx,al                      //set CRTC Index reg to point to Overflow
    inc    dx                         //point to CRTC Data reg
    in     al,dx                      //get the current Overflow reg setting
    and    al,not 10h                 //turn off split screen bit 8
    or     al,ah                      //insert the new split screen bit 8 (works in any mode)
    out    dx,al                      //set the new split screen bit 8

	dec    dx                         //point to CRTC Index reg
    mov    ah,byte ptr [line+1]
    and    ah,2
    mov    cl,3
    ror    ah,cl                      // move bit 9 of the split split screen scan line into position for the Maximum Scan Line register
    mov    al,MAX_SCAN_LINE
    out    dx,al                      //set CRTC Index reg to point to Maximum Scan Line
    inc    dx                         //point to CRTC Data reg
    in     al,dx                      //get the current Maximum Scan Line setting
    and    al,not 40h                 //turn off split screen bit 9
    or     al,ah                      //insert the new split screen bit 9 (works in any mode)
    out    dx,al                      //set the new split screen bit 9

	
	//Turn on split screen pel panning suppression, so the split screen
	//won't be subject to pel panning as is the non split screen portion.
	mov  dx,INPUT_STATUS_0
	in   al,dx                  	//Reset the AC Index/Data toggle to index state
	mov  al,AC_MODE_CONTROL+20h 	//Bit 5 set to prevent screen blanking
	mov  dx,AC_INDEX				//Point AC to Index/Data register
	out  dx,al
	inc  dx							//Point to AC Data reg (for reads only)
	in   al,dx						//Get the current AC Mode Control reg
	or   al,20h						//Enable split scrn Pel panning suppress.
	dec  dx							//Point to AC Index/Data reg (for writes only)
	out  dx,al						//Write the new AC Mode Control setting with split screen pel panning suppression turned on	
	
    //sti
    pop    dx
    pop    cx
    pop    ax
    }
}

//Reduced split screen function, just to move the position in 320x240 mode
void VGA_MoveWindow(){
    asm{
	push    ax
    push    cx
    push    dx
	
	//rep		movsw
	mov		dx,CRTC_INDEX
	mov		ax,SCR_WY
	mov    	cl,8
	shl		ax,cl
	add		ax,LINE_COMPARE
	out    	dx,ax

    mov    ah,byte ptr [SCR_WY+1]
    and    ah,1
    mov    cl,4
    shl    ah,cl
    mov    al,OVERFLOW

	out    dx,al                      
    inc    dx                         
    in     al,dx                      
    and    al,not 10h                 
    or     al,ah
    out    dx,al                      

    pop    dx
    pop    cx
    pop    ax
    }
}

void Window_in(){
	if (SCR_WY != 0){
		if (SCR_WY == 240*2) Print(0,6, "LOADING",1);
		SCR_WY-=8;
		VGA_MoveWindow();
	} else {
		//Window can't be at line 0, so we move the vram scroll to 0 
		//to hide line 0
		SCR_X = 0;
		SCR_Y = -240;
		VGA_Scroll_Vsync();
		SCR_WY = 0;
		VGA_MoveWindow();
		Scene++;
	}
}

void Window_out(){
	asm cli 
	//vram scroll was set to 0 to show the entire window, now we reset it to 240
	//to show cute things
	if (SCR_WY == 0) SCR_Y = 0;
	if (SCR_WY !=240*2){
		SCR_WY+=8;
		VGA_MoveWindow();
	}
	asm sti
}

void VGA_Set_Window(){
	VGA_SplitScreen(0);
	SCR_WY= 240*2;
	VGA_MoveWindow();
	Load_Tiles("maps/0_win.bmp",1);
	Load_Map("maps/0_win.tmx",1);
}

//stretches VRAM verticaly untill every scanline is repeated 4 times
void VGA_Stretch_VRAM(){
	byte stretch = 1;
	byte stretch_timer = 0;
	//Stretch vram
	while (stretch != 7){
		if (stretch_timer == 8) {stretch_timer= 0;stretch++;}
		word_out(CRTC_INDEX, V_RETRACE_END, 0x2c);
		word_out(CRTC_INDEX, MAX_SCAN_LINE, stretch);//Repeat scanline 4 times
		word_out(0x03d4, V_RETRACE_END, 0x8e);
		stretch_timer++;
		VGA_Vsync();
	}
}

void VGA_UnStretch_VRAM(){
	byte stretch = 7;
	byte stretch_timer = 0;
	//Stretch vram
	while (stretch != 1){
		if (stretch_timer == 8) {stretch_timer= 0;stretch--;}
		word_out(CRTC_INDEX, V_RETRACE_END, 0x2c);
		word_out(CRTC_INDEX, MAX_SCAN_LINE, stretch);//Repeat scanline 4 times
		word_out(0x03d4, V_RETRACE_END, 0x8e);
		stretch_timer++;
		VGA_Vsync();
	}
}

void VGA_mode_text(){
	union REGS regs;
	
	free(tile_tempdata); tile_tempdata = NULL;
	free(sprite); sprite = NULL;
	free(music_sdata); music_sdata = NULL;
	regs.h.ah = 0x00;
	regs.h.al = 0x03;
	int86(0x10, &regs, &regs);
	VGA_SplitScreen(0);
	
	if (sound_mode == 2)Unload_Music();
	if (sound_mode == 3)StopMOD();
	
	//Enable cursor
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, (inportb(0x3D5) & 0xC0) | 14);
	outportb(0x3D4, 0x0B);
	outportb(0x3D5, (inportb(0x3D5) & 0xE0) | 15);
}

unsigned int SINEX[] = {
	160, 163, 167, 171, 175, 179, 183, 187, 191, 195, 198, 202, 206, 210, 213, 217, 
	221, 224, 228, 231, 235, 238, 242, 245, 248, 252, 255, 258, 261, 264, 267, 270, 
	273, 275, 278, 281, 283, 286, 288, 290, 293, 295, 297, 299, 301, 302, 304, 306, 
	307, 309, 310, 311, 313, 314, 315, 316, 316, 317, 318, 318, 319, 319, 319, 319, 
	319, 319, 319, 319, 319, 318, 318, 317, 316, 316, 315, 314, 313, 311, 310, 309, 
	307, 306, 304, 302, 301, 299, 297, 295, 293, 290, 288, 286, 283, 281, 278, 275, 
	273, 270, 267, 264, 261, 258, 255, 252, 248, 245, 242, 238, 235, 231, 228, 224, 
	221, 217, 213, 210, 206, 202, 198, 195, 191, 187, 183, 179, 175, 171, 167, 163, 
	160, 156, 152, 148, 144, 140, 136, 132, 128, 124, 121, 117, 113, 109, 106, 102, 
	98, 95, 91, 88, 84, 81, 77, 74, 71, 67, 64, 61, 58, 55, 52, 49, 
	46, 44, 41, 38, 36, 33, 31, 29, 26, 24, 22, 20, 18, 17, 15, 13, 
	12, 10, 9, 8, 6, 5, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 8, 9, 10, 
	12, 13, 15, 17, 18, 20, 22, 24, 26, 29, 31, 33, 36, 38, 41, 44, 
	46, 49, 52, 55, 58, 61, 64, 67, 71, 74, 77, 81, 84, 88, 91, 95, 
	98, 102, 106, 109, 113, 117, 121, 124, 128, 132, 136, 140, 144, 148, 152, 156
};

unsigned int SINEY[] = {
	100, 104, 109, 114, 119, 124, 129, 133, 138, 142, 147, 151, 155, 159, 163, 167, 
	170, 174, 177, 180, 183, 185, 188, 190, 192, 194, 195, 197, 198, 198, 199, 199, 
	199, 199, 199, 198, 198, 197, 195, 194, 192, 190, 188, 185, 183, 180, 177, 174, 
	170, 167, 163, 159, 155, 151, 147, 142, 138, 133, 129, 124, 119, 114, 109, 104, 
	100, 95, 90, 85, 80, 75, 70, 66, 61, 57, 52, 48, 44, 40, 36, 32, 
	29, 25, 22, 19, 16, 14, 11, 9, 7, 5, 4, 2, 1, 1, 0, 0, 
	0, 0, 0, 1, 1, 2, 4, 5, 7, 9, 11, 14, 16, 19, 22, 25, 
	29, 32, 36, 40, 44, 48, 52, 57, 61, 66, 70, 75, 80, 85, 90, 95, 
	99, 104, 109, 114, 119, 124, 129, 133, 138, 142, 147, 151, 155, 159, 163, 167, 
	170, 174, 177, 180, 183, 185, 188, 190, 192, 194, 195, 197, 198, 198, 199, 199, 
	199, 199, 199, 198, 198, 197, 195, 194, 192, 190, 188, 185, 183, 180, 177, 174, 
	170, 167, 163, 159, 155, 151, 147, 142, 138, 133, 129, 124, 119, 114, 109, 104, 
	100, 95, 90, 85, 80, 75, 70, 66, 61, 57, 52, 48, 44, 40, 36, 32, 
	29, 25, 22, 19, 16, 14, 11, 9, 7, 5, 4, 2, 1, 1, 0, 0, 
	0, 0, 0, 1, 1, 2, 4, 5, 7, 9, 11, 14, 16, 19, 22, 25, 
	29, 32, 36, 40, 44, 48, 52, 57, 61, 66, 70, 75, 80, 85, 90, 95
};