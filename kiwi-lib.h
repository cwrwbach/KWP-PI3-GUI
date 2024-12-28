#pragma once 




//short rgb565(short red,short green,short blue);



void set_pixel(uint16_t * buf,int x, int y, uint16_t colour);

void clear_screen(uint16_t pixval);

short rgb565(short red,short green,short blue);

void plot_line (uint16_t * buf,int x0, int y0, int x1, int y1,uint16_t colour);

void plot_dotted_line (uint16_t * buf, int x0, int y0, int x1, int y1,uint16_t colour);

void plot_thick_line (uint16_t * buf, int x0, int y0, int x1, int y1,uint16_t colour);


void plot_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y, uint16_t colour);

void plot_filled_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y, uint16_t colour);

void plot_thick_rectangle(uint16_t * buf, int x0, int y0,int sz_x, int sz_y,uint16_t colour);

/*
void plot_button(int16_t * buf,int x0,int y0,int sz_x, int sz_y,uint16_t backround_col,uint16_t border_col,uint16_t text_col,char text[40])
*/


void plot_large_character(uint16_t * buf,int x, int y,uint8_t char_num,uint16_t colour);

void plot_large_string(uint16_t * buf , int x, int y,uint8_t * string ,uint16_t colour);

void copy_surface_to_framebuf(uint16_t *,uint,uint,uint,uint); // (buf,loc_x,lox_y,sz_x,sz_y)
