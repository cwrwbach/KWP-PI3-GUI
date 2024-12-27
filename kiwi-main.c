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
#include <ncurses.h>

#include "avc-lib.h"
#include "avc-colours.h"
#include "waterfall.h"
#include "qt_jet.h"

#define FFT_SIZE 1024
//#define FRAME_BUF_WIDTH 1366
#define FRAME_BUF_HEIGHT 768
//#define SPEC_WIDTH 1366
#define SPEC_HEIGHT 256
#define SPEC_BASE_LINE 255
//#define WFALL_WIDTH 1366
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
//int8_t fft_buf[FFT_SIZE];

uint8_t qtj[3];
void * setup_kiwi();
pthread_t callback_id;

void draw_spectrum(short);
void draw_waterfall();
void qt_jet(int);
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
    plot_dotted_line(spec_buf,0,i*h_gap,screen_size_x,i*h_gap,C_YELLOW);//YELLOW);

//for(i=1;i<n_verts-2;i++)
//    plot_dotted_line(spec_buf,i*v_gap,0,i*v_gap,SPEC_HEIGHT-5,C_YELLOW);//);
}

//------------------

void draw_spectrum(short colour)
{
int val;
int8_t temp;
int spec_base;
int xpos;
spec_base = SPEC_BASE_LINE;

//fill backround of SPEC
for(int b=0;b<SPEC_HEIGHT * screen_size_x;b++)
    spec_buf[b] = 0x0004;

draw_grid();
xpos = (screen_size_x - FFT_SIZE)/2;  //offset to centre

//kiwi_buf[300] = -25; //just a test/debug value

for(int n = 1; n < FFT_SIZE; n++)
    {
    temp = kiwi_buf[n];
    //if(temp > 0) { printf("Bin:%d Spike=%d \n",n,temp);} //temp = -100; } //Spike smuddger ? FIXME
    if(temp > 0) 
        temp = -100; //bug fudger FIXME
    val= 128 + temp; //kiwi_buf[n]; //fft_buf[n];
    val *=2; //scale up plot line (needed?)
  
    plot_line(spec_buf,xpos,spec_base , xpos,spec_base - val,colour); //Plots pos've from bottom left.
    xpos++;
    }
copy_surface_to_image(spec_buf,0,6,screen_size_x,SPEC_HEIGHT);
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

wfall_width = screen_size_x;

loc_x = 10;
loc_y = 10;

wf_ln++;
if(wf_ln > WFALL_HEIGHT)
    wf_ln = 1;

//Draw first line of waterfall
for(point=0;point<1024;point++) //FFT SIZE
    {
    //fiddle with thresholds here - just poking ??? *** ???
    if(kiwi_buf[point] < -80) kiwi_buf[point] = -127;
   
    inx = (int) 195+(kiwi_buf[point]); //adjusted to central
    //  inx = 190; /// 32; 
    red = (uint16_t) jet_col[inx][0]; //qtj[0];
    green=(uint16_t) jet_col[inx][1]; //qtj[1];
    blue =(uint16_t) jet_col[inx][2]; //qtj[2];
    colour = rgb565(red,green,blue);
    set_pixel(wfall_buf,point , 0, colour);
    }

//Scroll all lines down, starting from the bottom
for(int line = WFALL_HEIGHT; line >=0 ; line--)
    {
    for(int x = 0;x<wfall_width;x++)
        {
        wfall_buf[((line+1)*wfall_width)+wfall_width+x] = wfall_buf[(line * wfall_width)+x];
        }
    }

xpos = 170; // = (screen_size_x - FFT_SIZE)/2;  //offset to centre
copy_surface_to_image(wfall_buf,xpos,270,screen_size_x,WFALL_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)
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

spec_buf = malloc(screen_size_x*SPEC_HEIGHT*bytes_pp);
wfall_buf = malloc(screen_size_x*WFALL_HEIGHT*bytes_pp);

// map framebuffer to user memory 
frame_buf = (uint16_t * ) mmap(0, fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

clear_screen(rgb565(0,3,0));
plot_thick_rectangle(frame_buf,0,0,screen_size_x,264,C_BLUE);
plot_large_string(frame_buf,320,600,"WAITING FOR KIWI",C_WHITE);

//demo();
//while(1) sleep(1);

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
