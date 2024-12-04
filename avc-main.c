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

#define FFT_SIZE 1024

#define FRAME_BUF_WIDTH 800
#define FRAME_BUF_HEIGHT 480
#define SCOPE_WIDTH 800
#define SCOPE_HEIGHT 120

#define WFALL_WIDTH 800
#define WFALL_HEIGHT 200

#define BTN_WIDTH 70
#define BTN_HEIGHT 70

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int fbfd; // framebuffer file descriptor

long int screensize ;
char fft_video_buf[FFT_SIZE];

uint screen_size_x;
uint screen_size_y;
uint status_pos;

uint32_t * frame_buf;
uint32_t * scope_buf;
uint32_t * wfall_buf;
uint32_t * btn_buf[10];

//================

void draw_grid()
{
int i,x,y;
int n_horiz;
int n_verts;
int h_gap,v_gap;

n_horiz=6;
n_verts = 12;

v_gap = SCOPE_WIDTH/(n_verts-2);
h_gap = SCOPE_HEIGHT/(n_horiz);

for(i=1;i<n_horiz;i++)
    plot_dotted_line(scope_buf,0,i*h_gap,SCOPE_WIDTH,i*h_gap,0x00808000);//YELLOW);

for(i=1;i<n_verts-2;i++)
    plot_dotted_line(scope_buf,i*v_gap,0,i*v_gap,SCOPE_HEIGHT,0x00008000);//);

plot_thick_rectangle(scope_buf,0,0,SCOPE_WIDTH-2,SCOPE_HEIGHT-2,BLUE);
}

//------------------

void draw_fill_fft()
{
int bins_n;
uint8_t val;
int nv;

nv=0;

bins_n = 800;


for(int n = 0; n<bins_n; n++)
    {
    val = 117 - rand() % 50;

    if(n==400) 
        val = 20;
    val = fft_video_buf[n]/2;
    plot_line(frame_buf, nv, val, nv, 117, MAGENTA);
    
    nv++;
    }
}



void draw_trace_fft()
{
int bins_n;
uint8_t val;
int nv;
uint8_t db_lev,last;

nv=0;
bins_n = 800;

for(int iii = 0; iii<800;iii+=1)
    {
    db_lev=fft_video_buf[iii]/2;
   // plot_line(frame_buf,iii,120,iii,db_lev+5,BLUE);

    plot_line(frame_buf,iii,last,iii,db_lev,WHITE);
    last = db_lev;
    }

for(int n = 0; n<bins_n; n++)
    {
    val = 117 - rand() % 50;

if(n==400) val = 20;
   // val = fft_video_buf[n];
    plot_line(frame_buf, nv, val, nv, 117, MAGENTA);
    
   // fft_video_buf[n] = val;
    
    
    nv++;
    }
    
}
//=========

void draw_waterfall()
{
uint32_t colour,red,green,blue;
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

//Draw first line of waterfall
for(point=0;point<800;point++) //FFT SIZE
    {
    inx = 255-(fft_video_buf[point]);
    
    //printf(" VP %d \n",fft_video_buf[point]);

    red = (uint32_t) turbo[inx][0];
    green=(uint32_t) turbo[inx][1];
    blue =(uint32_t) turbo[inx][2];
    red = red<<16;
    green = green <<8;

    colour = red ;
    colour = colour | blue ;
    colour - colour | green;

    set_pixel(wfall_buf,point , 0, colour);
    }

//Scroll all lines up, starting from the bottom
    for(int ll = WFALL_HEIGHT; ll >=0 ; ll--)
    {
    for(int pp = 0;pp<WFALL_WIDTH;pp++)
        {
        wfall_buf[((ll+1)*WFALL_WIDTH)+WFALL_WIDTH+pp] = wfall_buf[((ll)* WFALL_WIDTH)+pp];
        }
    }

copy_surface_to_image(wfall_buf,0,150,WFALL_WIDTH,WFALL_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)
}

//=========

void main()
{
unsigned int red,green,blue;
short rgba;
int screenbytes;
int quit_request;
int err;
int nv;
int moop;
//int dummy;
__u32 dummy = 0;

//ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);

nv=0;

fbfd = open("/dev/fb0", O_RDWR); // Open the framebuffer device file for reading and writing
if (fbfd == -1) 
    printf("Error: cannot open framebuffer device.\n");
 
if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) // Get variable screen information
	    printf("Error reading variable screen info.\n");
printf("Display info %dx%d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

screen_size_x = vinfo.xres;
screen_size_y = vinfo.yres;

int fb_data_size = 800 * 480 * 4;

scope_buf = malloc(SCOPE_WIDTH*SCOPE_HEIGHT*4);
wfall_buf = malloc(WFALL_WIDTH*WFALL_HEIGHT*4);

for(int b=0;b<10;b++)
    btn_buf[b] = malloc(BTN_WIDTH*BTN_HEIGHT*4);

// map framebuffer to user memory 
frame_buf = mmap(0, fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

clear_screen(0x00000000);
printf(" LINE %d \n",__LINE__);
plot_large_string(frame_buf,360,300,"TESTING",WHITE);

//while(1) sleep(1);
printf(" LINE %d \n",__LINE__);

//plot_line(scope_buf,350,230,350,250,RED);
printf(" LINE %d \n",__LINE__);

printf(" LINE %d \n",__LINE__);
for(int b=0;b<10;b++)
    {
    plot_filled_rectangle(frame_buf,5+(b*80),390,BTN_WIDTH,BTN_HEIGHT,DARK_GREEN);
    }
printf(" LINE %d \n",__LINE__);

//Plot line is buggy (crashing) if valuse out of range! ! ! FIXME
//plot_line(scope_buf,350,30,360,30,RED);


setup_kiwi();


moop=0;
while(1)
    {

    for(int w=0;w<10;w++)
        err= ioctl(fbfd, FBIO_WAITFORVSYNC, &dummy); // Wait for frame sync

    draw_grid();
    //copy grid
    copy_surface_to_image(scope_buf,0,0,SCOPE_WIDTH,SCOPE_HEIGHT); // (buf,loc_x,lox_y,sz_x,sz_y)

read_kiwi_line();


   //do chosen FFT
  //draw_fill_fft();
 draw_trace_fft();
    //then waterfall
    draw_waterfall();
        usleep(500000);
        
    printf(" MAIN LOOPING %d : %d\n",moop++,__LINE__) ;   
    }

printf(" Debug at %d\n",__LINE__);
sleep(1);

while(1) sleep(1); //stopper
}
