#include "kiwi-conf.h"
#include "kiwi-lib.h"
#include "kiwi-colours.h"
#include "kiwi-jet.h"
#include <linux/kd.h>

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int fbfd;
long int screensize ;
uint g_centre_freq;
uint g_zoom;
uint g_speed;
uint g_url;


int8_t kiwi_buf[FFT_SIZE];

uint g_screen_size_x;
uint screen_size_y;
uint bytes_pp;
uint status_pos;
uint16_t * frame_buf;
uint16_t * spec_buf;
uint16_t * wfall_buf;
uint16_t * cmd_buf;

uint8_t qtj[3];
void * setup_kiwi();
pthread_t callback_id;
void draw_spectrum(short);
void draw_waterfall();
uint16_t get_colour(int);
void shuttle();

int box_width = 275;
//================

void demo()
{
short clr;
uint16_t colour,red,green,blue;
int inx;

for(int x = 0; x<1024;x++)
    {
inx = x/4;
    red = (uint16_t) jet_col[inx][0]; //qtj[0];
    green=(uint16_t) jet_col[inx][1]; //qtj[1];
    blue =(uint16_t) jet_col[inx][2]; //qtj[2];
    clr = rgb565(red,green,blue);
    plot_line(frame_buf,x,0,x,50,clr);
    }
}

void demo2()
{
short clr;
uint16_t colour,red,green,blue;
int inx;

for(int x = 0; x<1024;x++)
    {
    inx = x/8;
    clr = get_colour(inx);
    plot_line(frame_buf,x,50,x,100,clr);
    }
}




void draw_home_cmd()
{
plot_thick_rectangle(cmd_buf,box_width*0,0,box_width,CMD_HEIGHT-6,C_YELLOW);
}

void draw_speed_cmd()
{
plot_thick_rectangle(cmd_buf,box_width*1,0,box_width,CMD_HEIGHT-6,C_OLIVE);
}

void draw_cf_cmd()
{
plot_thick_rectangle(cmd_buf,box_width*2,0,box_width,CMD_HEIGHT-6,C_OLIVE_DRAB);
}

void draw_zoom_cmd()
{
plot_thick_rectangle(cmd_buf,box_width*3,0,box_width,CMD_HEIGHT-6,C_YELLOW_GREEN);
}

void draw_uri_cmd()
{
plot_thick_rectangle(cmd_buf,box_width*4,0,box_width,CMD_HEIGHT-6,C_WHEAT);
}


void draw_grid()
{
int i,x,y;
int n_horiz;
int n_verts;
int h_gap,v_gap;

n_horiz=5;
h_gap = SPEC_HEIGHT/(n_horiz);
//n_verts = 3;
//v_gap = SPEC_WIDTH/(n_verts-2);

for(i=1;i<n_horiz;i++)
    plot_dotted_line(spec_buf,0,i*h_gap,g_screen_size_x,i*h_gap,C_YELLOW);//YELLOW);

//for(i=1;i<n_verts-2;i++)
//    plot_dotted_line(spec_buf,i*v_gap,0,i*v_gap,SPEC_HEIGHT-5,C_YELLOW);//);
}

//------------------

void draw_spectrum(short colour)
{
int val;
int8_t level;
int spec_base;
int xpos;
spec_base = SPEC_BASE_LINE;

//fill backround of SPEC
for(int b=0;b<SPEC_HEIGHT * g_screen_size_x;b++)
    spec_buf[b] = 0x0004;

draw_grid();
//xpos = (g_screen_size_x - FFT_SIZE)/2;  //offset to centre
xpos = 42;

kiwi_buf[10] = -40; //just a test/debug value
kiwi_buf[14] = -60; 
kiwi_buf[18] = -80; 

int xtra; //Experimental interpolation of 25% to make 1024 bins display as 1280 pixels
for(int n = 1; n < FFT_SIZE; n++)
    {
    level = kiwi_buf[n];
    val= 120 + level; 
    val *=2; //Scale up * 2
    plot_line(spec_buf,xpos,spec_base , xpos,spec_base - val,colour); //Plots pos've from bottom left.

    if(xtra > 3)
        {
        xtra = 0;
        xpos++;
        plot_line(spec_buf,xpos,spec_base , xpos,spec_base - val,colour); //Plots pos've from bottom left.
        }
    xpos++;
    xtra++;
    }
copy_surface_to_framebuf(spec_buf,0,6,g_screen_size_x,SPEC_HEIGHT);
}

//=========

void draw_waterfall()
{
uint16_t colour,red,green,blue;
int point;
unsigned char fft_val;
int loc_x,loc_y;
unsigned int wf_ln;
int inx;
int xpos;
int wfall_width;

wfall_width = g_screen_size_x;

loc_x = 10;
loc_y = 10;

wf_ln++;
if(wf_ln > WFALL_HEIGHT)
    wf_ln = 1;

xpos = 170; //FIXME X POS WFALL

//Draw first line of waterfall
for(point=0;point<1024;point++) //FFT SIZE
    {
    //fiddle with thresholds here - just poking ??? *** ???
    //if(kiwi_buf[point] < -80) kiwi_buf[point] = -127;
   
    inx = (int) 130+(kiwi_buf[point]); //adjusted
    colour = get_colour(inx);

    set_pixel(wfall_buf,point + xpos , 0, colour);
    }

//colour = get_colour(inx);

//Scroll all lines down, starting from the bottom
for(int line = WFALL_HEIGHT; line >=0 ; line--)
    {
    for(int x = 0;x<wfall_width;x++)
        {
        wfall_buf[((line+1)*wfall_width)+wfall_width+x] = wfall_buf[(line * wfall_width)+x];
        }
    }

//xpos = 170; // = (screen_size_x - FFT_SIZE)/2;  //offset to centre
copy_surface_to_framebuf(wfall_buf,xpos,WFALL_Y_POS,g_screen_size_x,WFALL_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)
}

//======

void main()
{
unsigned int red,green,blue;
short rgba;
int screenbytes;
int quit_request;
int err;
int moop;
__u32 dummy = 0;

g_centre_freq = 10000;

fbfd = open("/dev/fb0", O_RDWR); // Open the framebuffer device file for reading and writing
if (fbfd == -1) 
    printf("Error: cannot open framebuffer device.\n");
 
if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) // Get variable screen information
	    printf("Error reading variable screen info.\n");
printf("Display info %dx%d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

//https://www.eevblog.com/forum/microcontrollers/(linux)-inhibit-kernel-messages-and-cursor/
  // hide cursor
char *kbfds = "/dev/tty";
int kbfd = open(kbfds, O_WRONLY);
if (kbfd >= 0) {
    ioctl(kbfd, KDSETMODE, KD_GRAPHICS);
    }
    else {
        printf("Could not open %s.\n", kbfds);
    }

g_screen_size_x = vinfo.xres;
screen_size_y = vinfo.yres;
bytes_pp = vinfo.bits_per_pixel/8;

int fb_data_size = g_screen_size_x * screen_size_y * bytes_pp;
printf (" FB data size = %d \n",fb_data_size);

spec_buf = malloc(g_screen_size_x*SPEC_HEIGHT*bytes_pp);
wfall_buf = malloc(g_screen_size_x*WFALL_HEIGHT*bytes_pp);
cmd_buf = malloc(g_screen_size_x*CMD_HEIGHT*bytes_pp);

// map framebuffer to user memory 
frame_buf = (uint16_t * ) mmap(0, fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

clear_screen(rgb565(0,3,0));
plot_thick_rectangle(frame_buf,0,0,g_screen_size_x,210,C_BLUE);

plot_large_string(frame_buf,320,600,"WAITING FOR KIWI",C_WHITE);

printf(" STOP AT DEMO - Main Line: %d \n",__LINE__);
//demo();
//demo2();

//while(1) sleep(1);

int ret=pthread_create(&callback_id,NULL, (void *) setup_kiwi,NULL);

printf(" SETUP ==========================  \n");

//plot_thick_rectangle(frame_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT,C_YELLOW);
plot_thick_rectangle(cmd_buf,0,0,g_screen_size_x,CMD_HEIGHT-6,C_YELLOW);

draw_home_cmd();
draw_speed_cmd();
draw_cf_cmd();
draw_zoom_cmd();
draw_uri_cmd();
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT); 

//printf(" Halted at %d \n",__LINE__);
//while(1) sleep(1);

while(1)
    {
    shuttle();
  //waiting for C2C commands etc.
    
    }

printf(" Debug at %d\n",__LINE__);
sleep(1);

while(1) sleep(1); //stopper
}
