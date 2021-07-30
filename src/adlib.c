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
	asm mov dx, 00388h
	asm mov al, reg
	asm out dx, al
	
	//Wait at least 3.3 microseconds, this makes it work on all opl2 compatible cards.
	asm mov cx,6
	wait:
		asm in ax,dx
		asm loop wait	//for (i = 0; i < 6; i++) inp(lpt_ctrl);
	
	asm inc dx
	asm mov al, data
	asm out dx, al
	
	//for( i = 35; i ; i-- )inport(0x388);
}

void opl2_clear(){
	unsigned char i, slot1, slot2;
    static unsigned char slotVoice[9][2] = {{0,3},{1,4},{2,5},{6,9},{7,10},{8,11},{12,15},{13,16},{14,17}};
    static unsigned char offsetSlot[18] = {0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21};
    
    opl2_out(   1, 0x20);   // Set WSE=1
    opl2_out(   8,    0);   // Set CSM=0 & SEL=0
    opl2_out(0xBD,    0);   // Set AM Depth, VIB depth & Rhythm = 0
    
    for(i=0; i<9; i++){
        slot1 = offsetSlot[slotVoice[i][0]];
        slot2 = offsetSlot[slotVoice[i][1]];
        
        opl2_out(0xB0+i, 0);    //turn note off
        opl2_out(0xA0+i, 0);    //clear frequency

        opl2_out(0xE0+slot1, 0);
        opl2_out(0xE0+slot2, 0);

        opl2_out(0x60+slot1, 0xff);
        opl2_out(0x60+slot2, 0xff);
        opl2_out(0x80+slot1, 0xff);
        opl2_out(0x80+slot2, 0xff);

        opl2_out(0x40+slot1, 0xff);
        opl2_out(0x40+slot2, 0xff);
    }
	for (i=1; i == 255; i++) opl2_out(i, 0);    //clear all registers
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
	asm CLI
	//byte *ost = music_sdata + music_offset;
	while (!imfwait){
        imfwait = music_sdata[music_offset+2];
        opl2_out(music_sdata[music_offset], music_sdata[music_offset+1]);
		music_offset+=3;
	}
	//ending song loop
	if (music_offset > 0xBFAC) music_offset = 0;
	imfwait--;

	asm STI
	asm mov al,020h
	asm mov dx,020h
	asm out dx, al	//PIC, EOI
}

void Music_Update(){
	if (sound_mode!=0){
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
	asm out dx, al	//PIC, EOI*/
	}
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
	if (sound_mode!=0){
	asm CLI //disable interrupts
	setvect(0x1C, play_music); //interrupt 1C not available on NEC 9800-series PCs.
	outportb(0x43, 0x36);
	outportb(0x40, spd);	//lo-byte
	outportb(0x40, spd >> 8);	//hi-byte	
	asm STI  //enable interrupts
	}
}

void Music_Remove_Interrupt(){
	if (sound_mode!=0){
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
