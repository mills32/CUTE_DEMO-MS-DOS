/* 
####################
CUTE DEMO FOR MS_DOS
####################
*/

#include "cutedemo.h"

int sound_mode = 0;
word Scene = 0;
word Part = 0;
word timer = 0;
byte frame = 0;
word page = (84*244)+4; //Only for rotozoom
int A = 0;
int B = 0;
int C = 0;
int D = 0;
int E = 0;

//For waves
unsigned char sint[] = {
	16,17,17,18,19,20,20,21,22,22,23,24,24,25,25,26,
	27,27,28,28,29,29,29,30,30,30,31,31,31,31,32,32,
	32,32,32,32,32,32,32,32,32,31,31,31,31,30,30,30,
	29,29,29,28,28,27,27,26,25,25,24,24,23,22,22,21,
	20,20,19,18,17,17,16,15,15,14,13,12,12,11,10,10,
	9,8,8,7,7,6,5,5,4,4,3,3,3,2,2,2,
	1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,2,2,2,3,3,3,4,4,5,5,6,7,7,
	8,8,9,10,10,11,12,12,13,14,15,15,16,
	16,17,17,18,19,20,20,21,22,22,23,24,24,25,25,26,
	27,27,28,28,29,29,29,30,30,30,31,31,31,31,32,32,
	32,32,32,32,32,32,32,32,32,31,31,31,31,30,30,30,
	29,29,29,28,28,27,27,26,25,25,24,24,23,22,22,21,
	20,20,19,18,17,17,16,15,15,14,13,12,12,11,10,10,
	9,8,8,7,7,6,5,5,4,4,3,3,3,2,2,2,
	1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,2,2,2,3,3,3,4,4,5,5,6,7,7,
	8,8,9,10,10,11,12,12,13,14,15,15,16,
};

//van waves
unsigned char sint1[] = {
	0,0,0,0,1,1,1,2,2,3,4,4,5,5,5,6,6,6,6,5,5,5,4,4,3,2,2,1,1,1
};

//algae waves
unsigned char algae_sin[] = {
	8,9,10,11,12,13,14,15,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7
};

//Homer move
unsigned char Homer_Sin[] = {
	0,1,1,2,2,3,3,4,5,6,6,7,8,8,9,10,
	10,11,12,13,13,
	14,14,15,15,16,16,16,16,16,16,16,
	15,15,14,14,13,13,12,11,10,10,9,8,7,6,6,5,
	4,3,3,2,2,1,1,0,0,0,0,0,0,0,0,0
};

//Window ani positions 192,124
unsigned char Window_Ani[] = {
	192,188,185,182,180,177,174,171,165,156,146,137,129,122,116,112,108,105,105,105
};

//LI'L GOOD SHIP ANIM
unsigned char Ship_Ani[] = {
	0,1,2,3,3,2,1,0,4,5,6,6,5,4
};

//Rocket ANIM
word Rocket_Ani[] = {
	0,1872*1,1872*2,1872*2,1872*1,0,1872*3,1872*4,1872*4,1872*3
};

char introtext[] = {"\
                READ  ME                               ----------               \
                                            THIS IS A PORT OF \042CUTE DEMO\042,      \
    FIRST MADE FOR GAME BOY COLOR,          AND NOW FOR OLD AND SLOW 8086       \
    COMPUTERS.                                                                  \
                                            PROGRAM TECHNIQUES USED HERE ARE    \
    WELL KNOWN, SO THE GOAL OF THIS         DEMO IS JUST TO BE CUTE.            \
             PRESS ANY KEY                                                      \
                THANKS!                 \
               ---------                                                        \
   THANKS TO THE CREATORS OF RESOURCES     AND PROGRAM TECHNIQUES, I USED FOR   \
   THIS DEMO :).                                                                \
   SORRY IF I ABUSED VGA COLOR CYCLES      AND HARDWARE SCROLLING.              \
                                                                                \
                                        \
             - MILLS 2020 -             \
               LOADING...               \
           MAGICAL VGA CARD             \
           META CODE                    \
           WORM HOLES                   \
        ERROR:  ADDRESS OVERFLOW        \
          GENERATING  UNIVERSE          \
"
};

///////////////////////////////////////
void Run_Intro();
void Load_8086();void Run_8086();
void Load_Persp();void Run_Persp();
void Load_Land();void Run_Land();
void Load_Homer();void Run_Homer();
void Load_Cog2D();void Run_Cog2D();
void Load_Cog3D();void Run_Cog3D();
void Load_Lisa();void Run_Lisa();
void Load_Plasma();void Run_Plasma();
void Load_Tower();void Run_Tower();
void Load_Roto();void Run_Roto();
void Load_Chip();void Run_Chip();
void Load_Ship();void Run_Ship();
void Load_Sea();void Run_Sea();
void Load_Credits();void Run_Credits();
//////////////////////////////////////////

void main(int argc, char *argv[]){
	char key = 0;
	char sce = 0;
	if(argc > 1){
		sce = atoi(argv[1]);
		if (sce > 13) sce = 13;
		printf("Debug - Selected scene %i.\nProgram will run it after the intro.\n", sce);
	}
	printf("\nLoading...\r");
	Adlib_Detect();
	VGA_Fade_out_Text();
	VGA_mode_X();//320x240 60Hz
	VGA_Set_Window();
	memset(tileset_palette,0x00,256*3);//All colours black
	Load_Font("maps/0_font.bmp");
	
	Set_FontWin_Color();
	
	Run_Intro();
	timer = 0;
	
	Scene = 1;
	if(argc > 1) {
		Scene = atoi(argv[1]);
		if (Scene > 13) Scene = 13;
		Scene = 1+(Scene<<1);
	}
	reset_demo:
	while(key != 27){
		switch (Scene){
			case 1:	Load_8086(); Scene++;break;
			case 2:	Run_8086(); break;
			case 3:	Load_Persp(); Scene++;break;
			case 4:	Run_Persp(); break;
			case 5:	Load_Land(); Scene++;break;
			case 6:	Run_Land(); break;
			case 7:	Load_Homer(); Scene++;break;
			case 8:	Run_Homer(); break;
			case 9:	Load_Cog2D(); Scene++;break;
			case 10:Run_Cog2D(); break;
			case 11:Load_Cog3D(); Scene++;break;
			case 12:Run_Cog3D(); break;
			case 13:Load_Lisa(); Scene++;break;
			case 14:Run_Lisa(); break;
			case 15:Load_Plasma(); Scene++;break;
			case 16:Run_Plasma(); break;
			case 17:Load_Tower(); Scene++;break;
			case 18:Run_Tower(); break;
			case 19:Load_Roto(); Scene++;break;
			case 20:Run_Roto(); break;
			case 21:Load_Chip();Scene++;break;
			case 22:Run_Chip();break;
			case 23:Load_Ship();Scene++;break;
			case 24:Run_Ship(); break;
			case 25:Load_Sea();Scene++;break;
			case 26:Run_Sea(); break;
			case 27:Load_Credits();Scene++;break;
			case 28:Run_Credits(); break;
			case 29:
				asm jmp exit_loop 
			break;
		}
		if(kbhit()) key = getch();
	}
	
	exit_loop:
	
	//If ESC key not pressed, go back to scene 1 
	if (key != 27) {
		Music_Add_Interrupt();
		Clearkb();
		VGA_Set_palette_to_black();
		VGA_Set_Window();
		//reload font because credits tiles destroyed it
		Load_Font("maps/0_font.bmp");
		SCR_WY = 0;
		A = 0;B = 0;C = 0;D = 0;SCR_X = 0; SCR_Y = 0;
		VGA_Scroll_Vsync();
		VGA_MoveWindow();
		Set_FontWin_Color();
		Scene = 1;
		
		asm jmp reset_demo
	}
	VGA_mode_text();
}




/////////////////
////FUNCTIONS////
/////////////////


//INTRO
void Run_Intro(){
	
	byte i = 0;
	word j = 0;
	//First message
	for (i = 0; i < 30;i++)Print(0,i,&introtext[80],0);
	for (i = 0; i < 12;i++){Print(0,i+8,&introtext[j],0);j+=40;}
	Print(0,27,&introtext[6*80],0);
	
	//wait key
	VGA_Fade_in(); Clearkb();
	while (!kbhit());
	Clearkb(); VGA_Fade_out();
	
	//Second message
	j = (7*80)-40;
	for (i = 0; i < 14;i++){Print(0,i+8,&introtext[j],0);j+=40;}

	//wait key
	VGA_Fade_in();Clearkb();
	while (!kbhit());
	Clearkb();
	
	//Loading
	Load_Sprite("sprites/loadbar.bmp",0,8);
	sprite[0].pos_x = 88;
	sprite[0].pos_y = 17*8;
	for (i = 0; i < 30;i++)Print(0,i,&introtext[80],0);
	Print(0,15,&introtext[(13*80)+40],0);
	while (timer < 120) {
		Draw_Sprite(0);
		sprite[0].pos_x = 88+timer;
		A++;
		if (A == 4){A=0;timer++;}
		if (timer == 0)Print(0,19,&introtext[(13*80)+80],0);
		if (timer == 30)Print(0,19,&introtext[(13*80)+120],0);
		if (timer == 70)Print(0,19,&introtext[(13*80)+160],0);
		VGA_Scroll_Vsync();
	}
	Unload_sprite(0);
	timer = 0;
	Print(0,19,&introtext[(13*80)+200],0);
	Print(0,20,&introtext[(13*80)+240],0);
	while (timer < 120) {timer++; VGA_Scroll_Vsync();}
	
	//Garbage screen
	VGA_Set_palette_to_black();
	timer = 0;
	Load_Tiles("maps/1_corrup.bmp",0);
	Load_Map("maps/1_corrup.tmx",0);
	while(timer < 150){Corruption_PaleteCycle();timer++;VGA_Scroll_Vsync();}
	timer = 0;
	
	VGA_Set_palette_to_black();
	Load_Tiles("maps/1_turtl.bmp",0);
	Load_Map("maps/1_turtl.tmx",0);
	
	SCR_Y = 0;
	
	VGA_Scroll_Vsync();
	Music_Load("music/atwist.imf");
	while (timer < 128){
		if (timer == 60) {
			Music_Add_Interrupt();
		}
		if (timer >60)Window_in();
		timer++;
		VGA_Scroll_Vsync();
	}
	Clearkb();
}

//////////////////////////////////////////
//8086 SCENE
void Load_8086(){
	Load_Tiles("maps/2_8086.bmp",0);
	Load_Map("maps/2_8086.tmx",0);
	A = 15*16;B = 0;C=0;D=0;SCR_X = 0; SCR_Y = 16; Part = 0;
	P8086_PaleteCycle();
	timer = 0;
}
void Run_8086(){
	if (timer == 70) {
		Load_Map("maps/2_8086.tmx",1); //Paste map at address 0
		SCR_Y -= 240;
		VGA_Scroll_Vsync();
		Part = 1;
		Music_Remove_Interrupt();
	}
	if (Part == 1){
		P8086_PaleteCycle();
		if (SCR_X < SINEX[A])SCR_X+=2; else Part = 2;
		if ((SCR_Y + 240 < SINEY[A]))SCR_Y++;
		Scroll_map();
		Music_Update();
	}
	if (Part == 2){
		P8086_PaleteCycle();
		SCR_X = SINEX[A];
		SCR_Y = SINEY[A]-240;
		Scroll_map();
		A++;
		if (A == 256) {A = 0;B++;}
		if (B == 4) Part = 3;
		Music_Update();
	}

	if (Part == 3){
		if (SCR_X > 0) SCR_X-=2;
		if (SCR_Y +240 > 0) {SCR_Y--; P8086_PaleteCycle();Music_Update();}
		else {
			Music_Add_Interrupt();
			Load_Map("maps/2_8086.tmx",0); //Paste map
			VGA_Scroll_Vsync();
			Load_Map("maps/0_win.tmx",1); //Paste window at address 0
			Part = 4;
		}
		Scroll_map();
		
	}

	VGA_Scroll_Vsync();
	if (timer < 70) Window_out();
	if (timer > 1000)Window_in();
	timer++;
}

//Perspective
void Load_Persp(){
	Load_Tiles("maps/3_persp.bmp",0);
	Load_Map("maps/3_persp.tmx",0);
	A = 0;B = 0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	Music_Remove_Interrupt();
}
void Run_Persp(){
	Perspective_PaleteCycle();
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 600) Window_in();
	timer++;
	Music_Update();
}

//Landscape
void Load_Land(){
	Music_Add_Interrupt();
	Load_Sprite("sprites/van.bmp",9,32);
	Set_Sprite_Animation(9,0,4,8);
	Load_Tiles("maps/4_land.bmp",0);
	Load_Map("maps/4_land.tmx",0);
	A = 0;B = 0;C = 0;D = 0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	map_offset_Endless = 1;
	sprite[9].pos_x = 7*16;
	sprite[9].pos_y = -240;
	Music_Remove_Interrupt();
}
void Run_Land(){
	Road_PaleteCycle();
	
	if (timer < 70) {
		if (SCR_WY == 240*2){
			Music_Add_Interrupt();
			//Draw road on window
			draw_map_row(0,0,17*32);draw_map_row(0,16,18*32);
			draw_map_row(0,32,19*32);draw_map_row(0,48,20*32);
			draw_map_row(0,64,21*32);
			map_offset_Endless = 1;
			Music_Remove_Interrupt();
		}
		Window_out();
		Music_Update();
	}
	
	if ((timer > 69) && (timer < 900)){
		Clear_Sprite(9);
		Draw_Sprite(9);
		sprite[9].pos_x = 7*16;
		sprite[9].pos_y = -240+sint1[A];
		if(SCR_Y < (16*6)-4){
			SCR_Y++;
			if (SCR_Y < 16*2)Scroll_map();
			if ((SCR_Y > 16*2)&& (SCR_WY != 340)) {
				SCR_WY-=4;
				VGA_MoveWindow();
			}
		}
		else {
			Endless_SideScroll_Map(5);
			if (C == 4){C = 0;SCR_X++;}
			C++;
		}
		if (A == 29) A = 0;
		A++;
		Music_Update();
	}
	if ((timer > 899)&&(timer < 1000)){ 

		if (SCR_Y !=0){
			if (SCR_WY != 240*2){SCR_WY+=2;VGA_MoveWindow();}
			SCR_Y--;
			Scroll_Map();
		}
		if (A == 29) A = 0;
		A++;
		Music_Update();
	}
	
	VGA_Scroll_Vsync();
	if (timer > 1000) {Window_in();Music_Update();}
	if (timer == 1000){SCR_WY = 480;Music_Add_Interrupt();VGA_Set_Window();Music_Remove_Interrupt();}
	timer++;
}

//Homer
void Load_Homer(){
	Music_Add_Interrupt();
	Unload_sprite(9);
	Load_Sprite("sprites/h_cloud.bmp",4,16);
	Clone_Sprite(5,4);
	Load_Tiles("maps/5_homer.bmp",0);
	Load_Map("maps/5_homer.tmx",0);
	A = 0;B = 0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	reduced_colunm_mode = 0;
	sprite[4].pos_x = 0;
	sprite[4].pos_y = 40;
	sprite[5].pos_x = 14*16;
	sprite[5].pos_y = 7*16;
	Music_Remove_Interrupt();
}
void Run_Homer(){
	Clear_Sprite(4);
	Clear_Sprite(5);
	Draw_Sprite(4);
	Draw_Sprite(5);
	Homer_PaleteCycle();
	if (A == 60) {A = 0;}
	if (B == 40) {B = 0;sprite[4].pos_x++;sprite[5].pos_x++;}
	SCR_Y = Homer_Sin[A];
	
	sprite[4].pos_y = 40+Homer_Sin[A];
	sprite[5].pos_y = (7*16)+Homer_Sin[A];
	
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 1000) Window_in();
	timer++;A++;B++;
	Music_Update();
}

//Cogs2D
void Load_Cog2D(){
	Music_Add_Interrupt();
	Unload_sprite(4);
	Load_Tiles("maps/6_cog2d.bmp",0);
	Load_Map("maps/6_cog2d.tmx",0);
	A = 0;B = 0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	Music_Remove_Interrupt();
}
void Run_Cog2D(){
	Cog2D_PaleteCycle();
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 600) Window_in();
	timer++;
	Music_Update();
}

///cog3D
void Load_Cog3D(){
	Music_Add_Interrupt();
	Load_Tiles("maps/7_cog3d.bmp",0);
	Load_Map("maps/7_cog3d.tmx",0);
	A = 0;B = 0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	Music_Remove_Interrupt();
}
void Run_Cog3D(){
	Cog3D_Move(A);
	Cog3D_PaleteCycle();
	//SCR_X+=2;
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 1000) Window_in();
	timer++;
	A+=132*2;
	if (A == 132*2*60) A = 0;
	Music_Update();
}

///Lisa
void Load_Lisa(){
	Music_Add_Interrupt();
	Load_Tiles("maps/8_lisa.bmp",0);
	Load_Map("maps/8_lisa.tmx",0);
	A = 0;B = 0;SCR_X = 0; SCR_Y = 0;
	map_offset_Endless = 1;
	timer = 0;
	Music_Remove_Interrupt();
}
void Run_Lisa(){
	Lisa_PaleteCycle();
	Endless_SideScroll_Map(0);
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 600) Window_in();
	timer++;
	SCR_X+=2;
	Music_Update();
}

//Plasma
//No real plasmas were harmed during the making of this demo
void Load_Plasma(){
	Music_Add_Interrupt();
	Load_Tiles("maps/9_plasma.bmp",0);
	Load_Map("maps/9_plasma.tmx",0);
	A = 0;B = 0;C=0;frame = 0;SCR_X = 0; SCR_Y = 0;
	map_offset_Endless = 1;
	timer = 0;
}
void Run_Plasma(){
	Plasma_PaleteCycle();
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 1000) Window_in();
	timer++;
}

//////////////////////////////////////////
//TOWER SCENE
void Load_Tower(){
	Load_Sprite("sprites/bat.bmp",9,32);
	Set_Sprite_Animation(9,0,4,8);
	Clone_Sprite(10,9);
	Set_Sprite_Animation(10,4,4,16);
	Load_Tiles("maps/10_tower.bmp",0);
	Load_Map("maps/10_tower.tmx",0);
	A = 0;B = 60;C = 0; D = 0;E = 0;SCR_X = 0; SCR_Y = 0;
	
	sprite[9].pos_x = 8+sint[A];
	sprite[9].pos_y = 64+sint[A];
	sprite[10].pos_x = 200+(sint[B]<<1);
	sprite[10].pos_y = 90+sint[B];
	timer = 0;
}
void Run_Tower(){
	Clear_Sprite(10);
	Draw_Sprite(9);
	Draw_Sprite(10);
	
	sprite[9].pos_x = 8+sint[A];
	sprite[9].pos_y = 64+sint[B];
	sprite[10].pos_x = 200+(sint[B]<<1);
	sprite[10].pos_y = 90+(sint[A]<<1);
	
	Tower_PaleteCycle();
	
	
	B+=2;
	if (B==140)B = 0;
	A++;
	if (A==140)A = 0;

	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 1000) Window_in();
	timer++;
}

//////////////////////////////////////////
//ROTO
void Load_Roto(){
	Unload_sprite(9);
	Load_Image("maps/11_rotoz.bmp");
	memset(&VGA[84*480],0,84*32);
	VGA_Stretch_VRAM();
	A = 0;B = 0;C=0;frame = 0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
}
void Run_Roto(){
	//This is the only part of the demo in wich I used double buffer
	//(there were some glitches on the 8086)
	if (C == 2) C = 0;
	if (C == 0) {page = (84*484)+4;SCR_Y = 0;}
	if (C == 1) {page = (84*244)+4;SCR_Y = 240;}
	C++;
	
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 600) Window_in();
	
	if (B == 180*150) B = 0;
	Roto_Zoom(SINEX[A&255],SINEY[A&255],B,page);
	A++;
	B+=150;
	
	timer++;
}

//////////////////////////////////////////
//CHIP
void Load_Chip(){
	VGA_UnStretch_VRAM();
	Load_Sprite("sprites/chipspr2.bmp",4,16);
	Load_Sprite("sprites/chipspr.bmp",9,32);
	Load_Tiles("maps/12_chip.bmp",0);
	Load_Map("maps/12_chip.tmx",0);
	Set_Sprite_Animation(9,0,28,1);
	Set_Sprite_Animation(4,0,28,2);
	A = 0;B = 70;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	Music_Remove_Interrupt();
}
void Run_Chip(){
	Clear_Sprite(9);
	Clear_Sprite(4);
	Draw_Sprite(9);
	Draw_Sprite(4);
	Chip_PaleteCycle();
	
	Endless_SideScroll_Map(0);
	
	
	sprite[9].pos_x = 180+SCR_X+sint[A];
	sprite[9].pos_y = 104+sint[B+70];
	sprite[4].pos_x = 80+SCR_X+sint[B];
	sprite[4].pos_y = 112+sint[A+70];
	
	SCR_X++;A++;B++;
	
	if (A == 140) A = 0;
	if (B == 140) B = 0;
	
	VGA_Scroll_Vsync();
	if (timer < 80) Window_out();
	if (timer > 800) Window_in();
	timer++;
	Music_Update();
}

//////////////////////////////////////////
//SHIP SCENE
void Load_Ship(){
	Music_Add_Interrupt();
	Unload_sprite(9);Unload_sprite(4);
	Load_Tiles("maps/13_ship.bmp",0);
	Load_Map("maps/13_ship.tmx",0);
	A = 0;B = 0;C = 8;D=0;SCR_X = 0; SCR_Y = 0;
	timer = 0;
	Music_Remove_Interrupt();
}

//This was too much for the 8086, I had to remove the scrolling
void Run_Ship(){
	byte col1 = 38;
	byte col2 = 1;
	byte col3 = 4;
	byte height = 80;
	
	if ((timer > 60)&&(timer < 951)){
		draw_good_ship((16*7)-4,10+sint[A+39],29+(Ship_Ani[C]*30));
		
		Very_fast_line(0,sint[A+28]+height, 16, -1+sint[A+15]+height, col1);
		Very_fast_line(0,4+sint[A+28]+height, 16, 3+sint[A+15]+height, col2);
		Very_fast_line(0,8+sint[A+28]+height, 16, 7+sint[A+15]+height, col3);
		
		Very_fast_line(16,-1+sint[A+18]+height, 27,-2+sint[A+32]+height, col1);
		Very_fast_line(16,3+sint[A+18]+height, 27, 2+sint[A+32]+height, col2);
		Very_fast_line(16,7+sint[A+18]+height, 27, 5+sint[A+32]+height, col3);
		
		Very_fast_line(51,sint[A+46]+height, 64, sint[A+26]+height, col1);
		Very_fast_line(51,4+sint[A+46]+height, 64, 4+sint[A+26]+height, col2);
		Very_fast_line(51,8+sint[A+46]+height, 64, 8+sint[A+26]+height, col3);
		
		Very_fast_line(64,sint[A+26]+height, 80, sint[A+15]+height, col1);
		Very_fast_line(64,4+sint[A+26]+height, 80, 4+sint[A+15]+height, col2);
		Very_fast_line(64,8+sint[A+26]+height, 80, 8+sint[A+15]+height, col3);
		A++;
		VGA_Scroll_Vsync();
		if (A==140)A = 0;
		if (B==140)B = 0;
		if (timer < 130) if (SCR_WY > 176) {SCR_WY-=4;VGA_MoveWindow();}
	}
	D++;
	if (D == 10) {D = 0; C++;}
	if (C == 14) C = 0;

	if ((timer > 950) && (timer < 1000)){
		if (SCR_WY < 240*2) {SCR_WY +=8; VGA_Scroll_Vsync(); VGA_MoveWindow();}
	}

	if (timer < 60) {
		Music_Update();
		VGA_Scroll_Vsync();
		Window_out();
	}
	if (timer == 60){
		Music_Add_Interrupt();
		memset(VGA,38,100*84);
		memset(VGA+(100*84),4,80*84);
	}
	
	if (timer == 1000) {
		Music_Add_Interrupt();
		SCR_WY = 240*2; 
		Load_Tiles("maps/0_win.bmp",1);
		Load_Map("maps/0_win.tmx",1);
		Music_Remove_Interrupt();
	}
	if (timer > 1000) {Music_Update(); VGA_Scroll_Vsync(); Window_in();}
	timer++;
	
}

//SEA SCENE
void Load_Sea(){
	Music_Add_Interrupt();
	Load_Tiles("maps/14_parlx.bmp",0);
	Load_Image("sprites/rkt_ani.bmp");
	Load_Map("maps/14_parlx.tmx",0);
	A = 0;B = 0;C = 0;D = 0; SCR_X = 0; SCR_Y = 0;
	map_offset_Endless = 1;
	timer = 0;
	VGA_MoveWindow();
	Music_Remove_Interrupt();
}
void Run_Sea(){
	draw_big_rocket(120+SCR_X+sint[A],49,Rocket_Ani[C]);
	SCR_X+=2;
	Endless_SideScroll_Map(0);
	Sea_PaleteCycle();
	A++;
	B++;
	if (A == 140) A = 0;
	if (B == 4){ B = 0;C++;}
	if (C == 10) C = 0;
	
	VGA_Scroll_Vsync();
	if (timer < 60) Window_out();
	if (timer > 1200) Window_in();
	timer++;
	Music_Update();
}

//CREDITS
void Load_Credits(){
	Music_Add_Interrupt();
	Load_Tiles("maps/15_cred.bmp",0);
	Load_Map("maps/15_cred.tmx",0);
	A = 0;B = 0;SCR_X = 0; SCR_Y = 0;Part = 0;
	timer = 0;
}
void Run_Credits(){
	if(Part == 0){
		if (timer < 80) Window_out();
		if (timer == 80){
			Load_Map("maps/15_cred.tmx",1);
			map_offset = 14*20;
			SCR_Y-=240;
			Part = 1;
			Music_Remove_Interrupt();
		}
	}
	if (Part == 1){
		if (B == 4){B = 0;Credits_PaleteCycle();}
		A++;
		B++;
		if (A == 2){A = 0;SCR_Y++;}
		Endless_DownScroll_map();
		if (SCR_Y == 0) SCR_Y -= 240;
		Music_Update();
	}
	timer++;
	VGA_Scroll_Vsync();
	
	if (timer == 5000){timer = 0; Scene++;}
}


