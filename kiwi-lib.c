#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "avc-lib.h"
#include "avc-font.h"
#include "avc-colours.h"
#include "ArialNum32x50.h"

extern int screen_size_x;
extern int screen_size_y;
extern uint16_t * frame_buf;

//---

void set_pixel(uint16_t * buf,int x, int y, uint16_t colour)
{
int sz_x,sz_y;
int32_t location;

sz_x = screen_size_x;
sz_y = screen_size_y;

if(x<0) x=1;
if(x > sz_x-1) x=sz_x-1;
if(y<0) y=0;
if(y>sz_y-1) y=sz_y-1;

location = x + (y*sz_x) ; 

buf[location] = colour;
}

void clear_screen(uint16_t pixval)
{
for(int p=0;p<(screen_size_x*screen_size_y);p++)
    frame_buf[p] = pixval;
}

/*
short rgb565(short red,short green,short blue)
{
short colour= 0;
colour = (red & 0x1f) <<11;
colour = colour | (green & 0x3f) <<6;
colour = colour | blue & 0x1f;
return colour;
}
*/

short rgb565(short red,short green,short blue)
{
short colour= 0;
colour = (red & 0xf8) <<8;
colour = colour | ((green & 0xfc) <<3);
colour = colour | (blue >>3);
return colour;
}



void plot_line (uint16_t * buf,int x0, int y0, int x1, int y1,uint16_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 

for (;;) //evah
    { 
    set_pixel (buf,x0,y0,colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}

void plot_dotted_line (uint16_t * buf, int x0, int y0, int x1, int y1,uint16_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 
int count;
count = 0;
for (;;) //evah
    { 
    if(count++ & 0x04)// miss out spaces
        set_pixel (buf,x0,y0,colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}


void plot_thick_line (uint16_t * buf, int x0, int y0, int x1, int y1,uint16_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 
for (;;) //evah
    { 
    set_pixel (buf,x0,y0,colour);
    set_pixel (buf,x0+1,y0,colour);
    set_pixel (buf,x0,y0+1,colour);
    set_pixel (buf,x0+1,y0+1,colour);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}



void plot_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y, uint16_t colour)
{
plot_line(buf, x0, y0, x0+sz_x, y0,colour); //top horiz
plot_line(buf, x0, y0+sz_y, x0+sz_x, y0+sz_y,colour); //bot horiz
plot_line(buf, x0, y0, x0, y0+sz_y,colour);
plot_line(buf, x0+sz_x, y0, x0+sz_x, y0+sz_y,colour);
}

void plot_thick_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y,uint16_t colour)
{
plot_thick_line(buf, x0, y0, x0+sz_x, y0,colour);
plot_thick_line(buf, x0, y0+sz_y, x0+sz_x, y0+sz_y,colour);
plot_thick_line(buf, x0, y0, x0, y0+sz_y,colour);
plot_thick_line(buf, x0+sz_x, y0, x0+sz_x, y0+sz_y,colour);
}

void xxxplot_filled_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y, uint16_t colour)
{
for(int n = 0; n< sz_y;n++)
    {
    plot_line(buf, x0, y0+n, x0+sz_x, y0+n,colour);
    plot_line(buf, x0, y0+sz_y, x0+sz_x, y0+sz_y,colour);
    plot_line(buf, x0, y0, x0, y0+sz_y,colour);
    plot_line(buf, x0+sz_x, y0, x0+sz_x, y0+sz_y,colour);
    }
}


/*
void plot_button(int16_t * buf,int x0,int y0,int sz_x, int sz_y,uint16_t backround_col,uint16_t border_col,uint16_t text_col,char text[40])
{
fill_surface(buf,backround_col);
plot_rectangle(buf,0,0,sz_x,sz_y,border_col);
plot_large_string(buf,10,20,text,text_col);
}


*/


void plot_large_character(uint16_t * buf,int x, int y,uint8_t char_num,uint16_t colour)
{
int horiz,vert;
int xx,yy;
unsigned short test_word;

for(vert=0,yy=0; vert<24;vert++,yy+=1)
    {
    test_word = ASCII_16x24[((char_num-32) * 24)+vert]; 
    for(horiz =0,xx=0; horiz<16;horiz++,xx++)
        {
        if(test_word & 0x0001)
            { 
            set_pixel(buf,x+xx,y+yy,colour);
            }
        test_word >>=1; 
        }
    }
}

void plot_large_string(uint16_t * buf , int x, int y,uint8_t * string ,uint16_t colour)
{
int len = strlen(string);
char char_num;
int xp = 0;

for(int s=0;s<len;s++,xp++)
    {
    char_num = string[s];
    plot_large_character(buf,x+xp*15,y,char_num,colour);
    }
}


void copy_surface_to_image(uint16_t *buf,uint loc_x,uint loc_y,uint sz_x,uint sz_y)
{
int x,y;
int y_offset;
int src_ptr;

src_ptr=0;

for(y=0;y<sz_y;y++) //height
    {
    y_offset = (loc_y + y) * sz_x;
    for(x=0;x<sz_x;x++) //width
        {
        frame_buf[y_offset +x +loc_x] = buf[src_ptr++]; 
        }
    }
}
