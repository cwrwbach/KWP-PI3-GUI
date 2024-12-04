#pragma once 


void clear_screen(uint32_t pixval);


short rgb565(short red,short green,short blue);



void set_pixel(uint32_t * buf,int x, int y, uint32_t colour);

void clear_screen(uint32_t pixval);

short rgb565(short red,short green,short blue);

void plot_line (uint32_t * buf,int x0, int y0, int x1, int y1,uint32_t colour);

void plot_dotted_line (uint32_t * buf, int x0, int y0, int x1, int y1,uint32_t colour);

void plot_thick_line (uint32_t * buf, int x0, int y0, int x1, int y1,uint32_t colour);


void plot_rectangle(uint32_t * buf, int x0, int y0,int sz_x, int sz_y, uint32_t colour);

void plot_thick_rectangle(uint32_t * buf, int x0, int y0,int sz_x, int sz_y,uint32_t colour);

/*
void plot_button(int16_t * buf,int x0,int y0,int sz_x, int sz_y,uint32_t backround_col,uint32_t border_col,uint32_t text_col,char text[40])
*/


void plot_large_character(uint32_t * buf,int x, int y,uint8_t char_num,uint32_t colour);

void plot_large_string(uint32_t * buf , int x, int y,uint8_t * string ,uint32_t colour);

void copy_surface_to_image(uint32_t *,uint,uint,uint,uint); // (buf,loc_x,lox_y,sz_x,sz_y)
