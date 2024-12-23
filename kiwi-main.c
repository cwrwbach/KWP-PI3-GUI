#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>

#include "avc-lib.h"
#include "avc-colours.h"
#include "waterfall.h"
#include "qt_jet.h"

#define FFT_SIZE 1024
#define FRAME_BUF_WIDTH 1366
#define FRAME_BUF_HEIGHT 768
#define SPEC_WIDTH 1366
#define SPEC_HEIGHT 150
#define SPEC_BASE_LINE 125
#define WFALL_WIDTH 1366
#define WFALL_HEIGHT 400

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

int fft_buf[FFT_SIZE];

uint8_t qtj[3];

void * setup_kiwi();
//void read_kiwi_line();
pthread_t callback_id;
//================

void draw_grid()
{
int i,x,y;
int n_horiz;
int n_verts;
int h_gap,v_gap;

//plot_filled_rectangle(spec_buf, 0, 0, SPEC_HEIGHT-5, C_DARK_GREEN);

n_horiz=6;


//n_verts = 3;
//v_gap = SPEC_WIDTH/(n_verts-2);
h_gap = SPEC_HEIGHT/(n_horiz);

for(i=1;i<n_horiz;i++)
    plot_dotted_line(spec_buf,0,i*h_gap,SPEC_WIDTH,i*h_gap,C_YELLOW);//YELLOW);
/*
//for(i=1;i<n_verts-2;i++)
//    plot_dotted_line(spec_buf,i*v_gap,0,i*v_gap,SPEC_HEIGHT-5,C_YELLOW);//);
*/

}

//------------------

void draw_spectrum(short colour)
{
short val;
int spec_base;
int nv;

spec_base = SPEC_BASE_LINE;

//fill backround of SPEC
for(int b=0;b<SPEC_HEIGHT * SPEC_WIDTH;b++)
    spec_buf[b] = 0x0004;

draw_grid();
    
nv=100; //horiz od=ffset
for(int n = 0; n < FFT_SIZE; n++)
    {
    val= fft_buf[n] & 0x007f; //max 127
    plot_line(spec_buf, nv,spec_base , nv,spec_base - val,colour);
    nv++;
    }
copy_surface_to_image(spec_buf,0,50,SPEC_WIDTH,SPEC_HEIGHT);
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

loc_x = 10;
loc_y = 10;

wf_ln++;
if(wf_ln > WFALL_HEIGHT)
    wf_ln = 1;

uint8_t xxx = 0;

printf(" \n \n");
//Draw first line of waterfall
for(point=0;point<1024;point++) //FFT SIZE
    {
    //fiddle with thresholds here - just poking ??? *** ???
    if(kiwi_buf[point] < -80) kiwi_buf[point] = -130;
   
    inx = (int) 200+(kiwi_buf[point]); //adjusted to central 800 points !!! FIXME
    //inx = -1 * (kiwi_buf[point] + 100); //adjusted to central 800 points !!! FIXME
    //printf(" VP %d \n",fft_video_buf[point]);
    //printf(" %d \n",inx);
       
    //inx =  point/4; //inx +20;
    qt_jet(inx);

    red = (uint16_t) jet_col[inx][0]; //qtj[0];
    green=(uint16_t) jet_col[inx][1]; //qtj[1];
    blue =(uint16_t) jet_col[inx][2]; //qtj[2];
    //red = red<<16;
    //green = green <<8;
//printf("xxx %d \n");
    //colour = rgb565(red/8,green/8,blue/8);
    colour = rgb565(red,green,blue);
    xxx++;
    //colour = red ;
    //colour = colour | blue ;
    //colour = colour | green;
    set_pixel(wfall_buf,point , 0, colour);
    }
//copy_surface_to_image(wfall_buf,0,150,WFALL_WIDTH,WFALL_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)

//for(int b=0;b<1024;b++)
 //   printf(" %d",wfall_buf[b]);
//printf(" \n");

//Scroll all lines down, starting from the bottom
    for(int ll = WFALL_HEIGHT; ll >=0 ; ll--)
    {
    for(int pp = 0;pp<WFALL_WIDTH;pp++)
        {
        wfall_buf[((ll+1)*WFALL_WIDTH)+WFALL_WIDTH+pp] = wfall_buf[((ll)* WFALL_WIDTH)+pp];
        }
    }
    
copy_surface_to_image(wfall_buf,100,200,WFALL_WIDTH,WFALL_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)
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

plot_large_string(frame_buf,320,600,"WAITING FOR KIWI",C_WHITE);

int ret=pthread_create(&callback_id,NULL, (void *) setup_kiwi,NULL);

printf(" SETUP ==========================  \n");

//setup_kiwi();

//while(1) {printf(" LINE %d \n",__LINE__);sleep(1);}

while(1)
    {
 //   printf(" FAB \n");
   // sleep(2);

   //draw_grid();
    //printf("Main: %d : %d",moop++,__LINE__) ;   
    }

//copy_surface_to_image(scope_buf,0,0,SPEC_WIDTH,SPEC_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)

printf(" Debug at %d\n",__LINE__);
sleep(1);

while(1) sleep(1); //stopper
}
