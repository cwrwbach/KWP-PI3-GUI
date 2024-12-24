#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include "avc-lib.h"
#include "avc-colours.h"
#include "waterfall.h"
#include "qt_jet.h"

#define FFT_SIZE 1024
#define FRAME_BUF_WIDTH 1366
#define FRAME_BUF_HEIGHT 768
#define SPEC_WIDTH 1366
#define SPEC_HEIGHT 256
#define SPEC_BASE_LINE 255
#define WFALL_WIDTH 1366
#define WFALL_HEIGHT 500

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int fbfd;

long int screensize ;
int8_t kiwi_buf[FFT_SIZE];
uint screen_size_x;
uint screen_size_y;
uint bytes_pp;
uint status_pos;
uint16_t * frame_buf;
uint16_t * spec_buf;
uint16_t * wfall_buf;
int8_t fft_buf[FFT_SIZE];

uint8_t qtj[3];
void * setup_kiwi();
pthread_t callback_id;

void draw_spectrum(short);
void draw_waterfall();
void qt_jet(int);
//================

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
    plot_dotted_line(spec_buf,0,i*h_gap,SPEC_WIDTH,i*h_gap,C_YELLOW);//YELLOW);

//for(i=1;i<n_verts-2;i++)
//    plot_dotted_line(spec_buf,i*v_gap,0,i*v_gap,SPEC_HEIGHT-5,C_YELLOW);//);
}

//------------------

void draw_spectrum(short colour)
{
int val;
int spec_base;
int xpos;
spec_base = SPEC_BASE_LINE;

//fill backround of SPEC
for(int b=0;b<SPEC_HEIGHT * SPEC_WIDTH;b++)
    spec_buf[b] = 0x0004;

draw_grid();

//Test values
/* 
for(int n = 0; n < FFT_SIZE; n++)
    {
    fft_buf[n]=0;
    }
fft_buf[256] = 100;
fft_buf[257] = 100;
fft_buf[260] = 150;
fft_buf[261] = 150;
fft_buf[264] = 200;
fft_buf[265] = 200;
*/

xpos = (screen_size_x - FFT_SIZE)/2;  //offset to centre
//nv=100; //horiz offset
for(int n = 0; n < FFT_SIZE; n++)
    {
    val= fft_buf[n];
    if (val > 100 || val < 0) 
        printf(" Val error at %d \n", val);
    plot_line(spec_buf,xpos,spec_base , xpos,spec_base - val,colour);
    xpos++;
    }
copy_surface_to_image(spec_buf,0,6,SPEC_WIDTH,SPEC_HEIGHT);
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

loc_x = 10;
loc_y = 10;

wf_ln++;
if(wf_ln > WFALL_HEIGHT)
    wf_ln = 1;

//Draw first line of waterfall
for(point=0;point<1024;point++) //FFT SIZE
    {
    //fiddle with thresholds here - just poking ??? *** ???
    //if(kiwi_buf[point] < -80) kiwi_buf[point] = -130;
   
    inx = (int) 200+(kiwi_buf[point]); //adjusted to central
       
    qt_jet(inx); //look-up colour

    red = (uint16_t) jet_col[inx][0]; //qtj[0];
    green=(uint16_t) jet_col[inx][1]; //qtj[1];
    blue =(uint16_t) jet_col[inx][2]; //qtj[2];
    colour = rgb565(red,green,blue);
    set_pixel(wfall_buf,point , 0, colour);
    }

//Scroll all lines down, starting from the bottom
    for(int ll = WFALL_HEIGHT; ll >=0 ; ll--)
    {
    for(int pp = 0;pp<WFALL_WIDTH;pp++)
        {
        wfall_buf[((ll+1)*WFALL_WIDTH)+WFALL_WIDTH+pp] = wfall_buf[((ll)* WFALL_WIDTH)+pp];
        }
    }
xpos = (screen_size_x - FFT_SIZE)/2;  //offset to centre
copy_surface_to_image(wfall_buf,xpos,270,WFALL_WIDTH,WFALL_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)
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

fbfd = open("/dev/fb0", O_RDWR); // Open the framebuffer device file for reading and writing
if (fbfd == -1) 
    printf("Error: cannot open framebuffer device.\n");
 
if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) // Get variable screen information
	    printf("Error reading variable screen info.\n");
printf("Display info %dx%d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

screen_size_x = vinfo.xres;
screen_size_y = vinfo.yres;
bytes_pp = vinfo.bits_per_pixel/8;

int fb_data_size = screen_size_x * screen_size_y * bytes_pp;
printf (" FB data size = %d \n",fb_data_size);

spec_buf = malloc(SPEC_WIDTH*SPEC_HEIGHT*bytes_pp);
wfall_buf = malloc(WFALL_WIDTH*WFALL_HEIGHT*bytes_pp);

// map framebuffer to user memory 
frame_buf = (uint16_t * ) mmap(0, fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

clear_screen(rgb565(0,3,0));
plot_thick_rectangle(frame_buf,0,0,screen_size_x,264,C_BLUE);
plot_large_string(frame_buf,320,600,"WAITING FOR KIWI",C_WHITE);

int ret=pthread_create(&callback_id,NULL, (void *) setup_kiwi,NULL);

printf(" SETUP ==========================  \n");

while(1)
    {
    //waiting for C2C commands etc.
    sleep(1);
    }

printf(" Debug at %d\n",__LINE__);
sleep(1);

while(1) sleep(1); //stopper
}
