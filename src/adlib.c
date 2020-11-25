/*****************************************************************************
	Based on K1n9_Duk3's IMF Player - A simple IMF player for DOS
	And on Apogee Sound System (ASS) and Wolfenstein 3-D (W3D)
		ASS is Copyright (C) 1994-1995 Apogee Software, Ltd.
		W3D is Copyright (C) 1992 Id Software, Inc.
*******************************************************************************/

#include "cutedemo.h"

void interrupt (*old_time_handler)(void); 
void Start_Music();
void Stop_Music();

//ADLIB
const int opl2_base = 0x388;
word music_size;
word music_offset;
byte music_filetype; //0 1 - imf0 imf1
byte *music_sdata;
word imfwait;

void opl2_out(unsigned char reg, unsigned char data){
	asm mov ah,0
	asm mov dx, 00388h
	asm mov al, reg
	asm out dx, al
	asm inc dx
	asm mov al, data
	asm out dx, al
}

void opl2_clear(void){
	int i;
    for (i=1; i< 256; opl2_out(i++, 0));    //clear all registers
}
extern int sound_mode;
void Adlib_Detect(){ 
    int status1, status2, i;

    opl2_out(4, 0x60);
    opl2_out(4, 0x80);

    status1 = inp(ADLIB_PORT);
    
    opl2_out(2, 0xFF);
    opl2_out(4, 0x21);

    for (i=100; i>0; i--) inp(ADLIB_PORT);

    status2 = inp(ADLIB_PORT);
    
    opl2_out(4, 0x60);
    opl2_out(4, 0x80);

    if ( ( ( status1 & 0xe0 ) == 0x00 ) && ( ( status2 & 0xe0 ) == 0xc0 ) ){
        unsigned char i;
		for (i=1; i<=0xF5; opl2_out(i++, 0));    //clear all registers
		opl2_out(1, 0x20);  // Set WSE=1
		printf("\nAdLib card detected.\n");
		asm CLI
		old_time_handler = getvect(0x1C);
		asm STI
		sound_mode = 2;
		sleep(2);
        return;
    } else {
		printf("\nAdLib card not detected.\nSound is disabled.\n");
		sound_mode = 0;
		sleep(2);
        return;
    }
}

void interrupt play_music(void){
	//byte *ost = music_sdata + music_offset;
	while (!imfwait){
        imfwait = music_sdata[music_offset+2];
        opl2_out(music_sdata[music_offset], music_sdata[music_offset+1]);
		music_offset+=3;
	}
	//ending song loop
	if (music_offset > 0xBFAC) music_offset = 0;
	imfwait--;

	asm mov al,020h
	asm mov dx,020h
	asm out dx, al	//PIC, EOI
}

void Music_Update(){
    while (!imfwait){
        imfwait = music_sdata[music_offset+2];
        opl2_out(music_sdata[music_offset], music_sdata[music_offset+1]);
		music_offset+=3;
	}
	//ending song loop
	if (music_offset > 0xBFAC) music_offset = 0;
	imfwait--;
	asm mov al,020h
	asm mov dx,020h
	asm out dx, al	//PIC, EOI
}

void Music_Load(char *fname){
	word offset = 0;
	word offset1 = 0;
	FILE *imfile = fopen(fname, "rb");
	unsigned char rb[16];
	//struct stat filestat;
	
	opl2_clear();
	fread(rb, sizeof(char), 2, imfile);
	
	//IMF
	music_filetype = 0;
	music_offset = 0L;

    fseek(imfile, 0, SEEK_SET);

	fread(music_sdata, 1, 0xFF5C,imfile);
	fclose(imfile);
	// 0011 1111 0000
	//simplify even more the format, to get more speed for old 8086
	for (offset = 0; offset < 0xFF90; offset+= 4){
		music_sdata[offset1] = music_sdata[offset];
		music_sdata[offset1+1] = music_sdata[offset+1];
		music_sdata[offset1+2] = music_sdata[offset+2] | (music_sdata[music_offset+3]) << 8;
		offset1 += 3;
	}
}

void Music_Add_Interrupt(){
	//set interrupt and start playing music
	unsigned long spd = 1193182/60;
	asm CLI //disable interrupts
	setvect(0x1C, play_music); //interrupt 1C not available on NEC 9800-series PCs.
	outportb(0x43, 0x36);
	outportb(0x40, spd);	//lo-byte
	outportb(0x40, spd >> 8);	//hi-byte	
	asm STI  //enable interrupts
}

void Music_Remove_Interrupt(){
	asm CLI //disable interrupts
	//reset interrupt
	outportb(0x43, 0x36);
	outportb(0x40, 0xFF);	//lo-byte
	outportb(0x40, 0xFF);	//hi-byte
	setvect(0x1C, old_time_handler);
	//opl2_clear();
	asm STI  //enable interrupts
	//music_offset = 0;
}

void Music_Unload(){
	Music_Remove_Interrupt();
	music_size = NULL;
	music_offset = NULL;
	music_filetype = NULL;
	if (music_sdata){
		free(music_sdata); 
		music_sdata = NULL;
	}
}
