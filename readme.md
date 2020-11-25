CUTE DEMO FOR MS_DOS
--------------------

I wanted to port my GBC demo to an 8086 at 8 MHz, (It was my first PC, and I hated when a game refused to work because it used 286 instructions). 
I wanted to use the VGA card to do something cute, but VGA is not really designed with games or demos in mind, so it is difficult to make cool things whith it, if the main cpu is slow.

I think the main problem with VGA, is games were made using the vga as a simple frame buffer, so the cpu had to do all the work. When EGA/VGA appeared, 286 CPUS were already popular, and they were so fast compared to 8086, that few games used tricks and functions the VGA had to speed up things. 

REQUIREMENTS
------------

CPU: 8086 8 MHz

GPU: VGA (256 KB VRAM)

RAM: 512KB

AUDIO: ADLIB/OPL2 compatible

Does it work on an 8088 at 4.77?... We don't wanna know, it's a market we can do without... Of course it works, but it is slower and has more glitches.

SVGA will show glitches when scrolling, so use just vga.

VGA HARDWARE
------------

So Let's see what the VGA has to offer for the poor and slow 8086.

  -Hardware scroll:

What?... Yes, it has hardware scroll but most games only use it to change "video page" to use the VGA as two frame buffers to draw. So it is possible to scroll a backgrond with nearly 0 CPU usage.

  -16 colors tile map mode

VGA(EGA and CGA) has a mode with character cells just like consoles, but it has only 2 colours per cell. Everybody calls it "text mode". As the hardware scroll works also in this mode, you can create maps made of characters and scroll them whith 0 CPU usage. It has only one sprite (cursor). I only found a demo showing a map in text mode. I was going to use it in this demo, but I wanted more than 16 colours. 

  -32 bit transfers (VRAM to VRAM):

VGA can transfer 4 pixels at a time from vram to vram, this is usefull to update a column or a row when scrolling maps in mode x. I guess this was used a lot, but to fill the entire screen. In this demo all the scrolling maps you see, are the result of hardware scrolling + updates of 1 column and 1 row of tiles using this 32 bit transfers.

  -Split screen (Window):

A separate part of VRAM that can be used to show things at the bottom of the screen.

  -Palette cycles:

This was abused in games, thanks to palette cycling I faked huge animations for the demo.

Also I created a software "blitter" made in assembly, it uses the 32 bit transfer to draw lines very fast.

Sprites are also fake, the CPU has to draw/clean/redraw all data.

SCENES
------

So what is this? Just a bunch of cute scenes:

  -8086 Scroll: it uses hardware scroll and a software asm blitter to update one row + one column of the map.
  Also uses assembly code to update colours, so that it fakes a second layer.
  
  -Mode 7: completely fake 3d effect made ising a prerendered image and palette cycles.
  
  -Road: uses Hardware scrolling and hardware window to show two (real) layers. Road movement is faked using Palette cycles.
  
  -Driving Alien (Homer): Harwdare vertical scroll + faked second layer (palette cycles).
  
  -2D rotation: Completely fake (palette cycles).
  
  -3D rotation: The smaller cogs are palette cycles. The bigger one is a software blitter that draws lines with different textures.
  
  -Dental Plan: Again two faked layers (palette cycles + hardware scroll).
  
  -Under the sea: I couldn't make a scanline wave effect so I just compiled some line/vector animations in assembly.
  
  -Castle: Just a pre rendered image with colours placed carefully to fake the rotation.
  
  -Rotozoom: A real software rotozoom in assembly, thanks to people in Vintage Computer Federation.
 
  -Boat: A cute boat drawn using software blitter + assembly vector drawing + hardware scrolling/window. 

  -Chips: a neat hardware scroll with two sprites and some palette cycling.
  
  -Parallax: This draws a big low res sprite, and fakes parallax using palette cycles + hardware scrolling.
  
  -Credits: Nothing special, palette cycles + hardware scrolling.


NOTES
-----

I'm not a programmer, I learned everything by testing and reading, so I might do things in ways people won't like. For example, I prefer the C files to have as few lines as possible, to read the files in a way I understand them better, so I'll fit "while's", "if's" and "for's" in long lines, or I'll paste several functions in the same line, because they are one program block in my mind.
