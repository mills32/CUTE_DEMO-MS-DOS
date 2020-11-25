#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h> //use stat to get correct file size
#include <dos.h>
#include <alloc.h>
#include <mem.h>
#include <math.h>
#undef outp

#define SC_INDEX            0x03c4    // VGA sequence controller
#define SC_DATA             0x03c5

#define GC_INDEX            0x03ce    // VGA graphics controller
#define GC_DATA             0x03cf

#define MAP_MASK            0x02
#define ALL_PLANES          0xff02
#define MEMORY_MODE         0x04
#define CRTC_DATA           0x03d5

#define VIDEO_INT           0x10      // the BIOS video interrupt.
#define SET_MODE            0x00      // BIOS func to set the video mode.
#define VGA_256_COLOR_MODE  0x13      // use to set 256-color mode. 
#define TEXT_MODE           0x03      // use to set 80x25 text mode. 

#define SCREEN_WIDTH        320       // width in pixels of mode 0x13
#define SCREEN_HEIGHT       240       // height in pixels of mode 0x13
#define NUM_COLORS          256       // number of colors in mode 0x13

#define AC_HPP              0X20 | 0X13    // Horizontal Pel Panning Register

#define DE_MASK				0x01     //display enable bit in status register 1

#define MISC_OUTPUT         0x03c2    // VGA misc. output register
#define S_MEMORY_MODE		0x03C4	
#define INPUT_STATUS_0		0x03da
#define AC_MODE_CONTROL		0x10	  //Index of Mode COntrol register in AC
#define AC_INDEX			0x03c0	  //Attribute controller index register
#define CRTC_INDEX			0x03d4	  // CRT Controller Index
#define H_TOTAL             0x00      // CRT controller registers
#define H_DISPLAY_END       0x01
#define H_BLANK_START       0x02
#define H_BLANK_END         0x03
#define H_RETRACE_START     0x04
#define H_RETRACE_END       0x05
#define V_TOTAL             0x06
#define OVERFLOW            0x07
#define PRESET_ROW_SCAN     0x08 
#define MAX_SCAN_LINE       0x09
#define V_RETRACE_START     0x10
#define V_RETRACE_END       0x11
#define V_DISPLAY_END       0x12
#define OFFSET              0x13
#define UNDERLINE_LOCATION  0x14
#define V_BLANK_START       0x15
#define V_BLANK_END         0x16
#define MODE_CONTROL        0x17
#define LINE_COMPARE		0x18 

#define ADLIB_PORT 			0x388

/* macro to write a word to a port */
#define word_out(port,register,value) \
  outport(port,(((word)value<<8) + register))

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

typedef struct tagSPRITEFRAME{			// structure for a sprite frame
	char *compiled_code;
} SPRITEFRAME;

typedef struct tagSPRITE{				// structure for a sprite
	word width;
	word height;
	byte palette[256*3];
	word init;	//init sprite to captute bak data
	word animate;
	word speed;
	word anim_counter;
	word anim_speed;
	word baseframe; //first frame
	word aframes;
	word ground;	//platform touch ground
	byte get_item;	//For player
	byte ntile_item;
	byte col_item;
	byte mode; //FOR AI
	word jump;
	word jump_frame;
	word climb;
	word tile_x;
	word tile_y;
	word pos_x;	
	word pos_y;
	word last_x;
	word last_y;
	int mspeed_x;
	int mspeed_y;
	int speed_x;
	int speed_y;
	word s_x;		//To control speed
	word s_y;
	word misc;
	word state;		//0 no speed
	word frame;
	word nframes;
	word bkg_data;	//restore bkg variables
	word size;
	word siz;
	word next_scanline;
	word s_delete;
	SPRITEFRAME *frames;	
} SPRITE;


extern byte *VGA;        /* this points to video memory. */
extern word TILE_VRAM;
extern word FONT_VRAM;

extern byte tileset_palette[];
extern unsigned char *tile_tempdata; //Temp storage of non tiled data.
extern char *compiled_code;
extern byte *vector_frame;
extern word SCR_X;
extern word SCR_Y;
extern word SCR_WX;
extern word SCR_WY;
extern unsigned int SINEX[];
extern unsigned int SINEY[];
extern word Scene;

extern word map_offset;
extern int map_offset_Endless;
extern int current_x;
extern int last_x;
extern int current_y;
extern int last_y;
extern word reduced_colunm_mode;

extern SPRITE *sprite;

void Clearkb();

//VGA
void VGA_mode_text();
void VGA_mode_X();
void VGA_Stretch_VRAM();
void VGA_UnStretch_VRAM();
void VGA_Scroll_Vsync();
void VGA_Set_Window();
void VGA_Window_in();
void VGA_Window_out();

//MAPS
void Load_Tiles(char *file, int window);
void draw_map_column(word x, word y, word map_offset);
void draw_map_row( word x, word y, word map_offset);
void Load_Map(char *file,int window);
void Reset_Map_Vars(word width, word height, word offset);
void Scroll_map();
void Endless_SideScroll_Map(int y);
void Endless_DownScroll_map();
void draw_good_ship(word x, word y, word map_offset);
void Load_Image(char *file);

//FONTS
void Load_Font(char *file);
void Print(word x, word y, char *string, byte win);
void Set_FontWin_Color();

//SPRITES
void Load_Sprite(char *file, int sprite_number, byte size);
void Clone_Sprite(int sprite_number_c,int sprite_number);
void Set_Sprite_Animation(int sprite_number, byte baseframe, byte frames, byte speed);
void Draw_Sprite(int sprite_number);
void Clear_Sprite(int sprite_number);
void Unload_sprite(int sprite_number);
void Draw_Big_Rocket(word x, word y, word frame);
void Draw_GoodShip();
void Draw_Window(word x,word frame);

//PALETTES
void VGA_Set_palette_to_black();
void VGA_Set_palette(unsigned char *palette);
void VGA_Fade_in();
void VGA_Fade_out();
void VGA_Fade_out_text();
void Corruption_PaleteCycle();
void P8086_PaleteCycle();
void Perspective_PaleteCycle();
void Road_PaleteCycle();
void Homer_PaleteCycle();
void Cog2D_PaleteCycle();
void Cog3D_PaleteCycle();
void Lisa_PaleteCycle();
void Sea_PaleteCycle();
void Tower_PaleteCycle();
void Chip_PaleteCycle();
void Credits_PaleteCycle();

//SCANLINE AND ROTOZOOM EFFECTS
void Cog3D_Move(word frame);
void Roto_Zoom(byte x, byte y, word frame);
void Algae_Move(byte x, word frame);

//VECTORS
void Very_fast_line(int x1, int y1, int x2, int y2, byte color);
void Load_Shape(char *file, int sprite_number);
void Draw_Shape(int sprite_number);
//ADLIB
void Adlib_Detect();
void Load_Music(char *fname);
void Start_Music();
void do_play_music();
void Unload_Music();
