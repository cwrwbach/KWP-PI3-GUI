#include <linux/input.h>
#include "kiwi-conf.h"
#include "kiwi-colours.h"
#include "kiwi-lib.h"

#include <vws/websocket.h>

extern uint g_centre_freq;

extern int g_zoom;
extern int g_speed;
extern uint g_url;
extern uint g_tab_width;
extern uint g_tab_height;

typedef struct input_event EV;
EV ev;

extern uint16_t * cmd_buf;
extern uint g_screen_size_x;
extern vws_cnx* cnx;

int cmd_select;
char cmd_string[16];

#define SYSTEM 0
#define ZOOM 1
#define CF 2
#define SPEED 3

void clear_tab(int tab,uint16_t colour)
{
clear_rectangle(cmd_buf,(tab * g_tab_width)+3, 4, (tab * g_tab_width)+g_tab_width-4,g_tab_height,colour);
}



void update_zoom()
{

	if(ev.code ==263 && ev.value ==1)
		{
			g_zoom +=1;
			if(g_zoom > 14) 
				g_zoom = 14;
		clear_tab(1,C_BLACK);
			sprintf(cmd_string,"Zoom: %d ",g_zoom);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
	copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
	
		}
	
	if(ev.code ==261 && ev.value ==1)
		{
			g_zoom -=1;
			if(g_zoom < 0 ) 
				g_zoom = 0;
		
			clear_tab(1,C_BLACK);
			sprintf(cmd_string,"Zoom: %d ",g_zoom);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
			}
char sz[32];

sprintf(sz,"SET zoom=%d cf=15000",g_zoom);


vws_frame_send_text(cnx,sz);
//vws_frame_send_text(cnx,"SET zoom=5 cf=5505"); 
usleep(100000);

}

void update_speed()
{
 	if(ev.code ==263 && ev.value ==1)
		{
			g_speed +=1;
			if(g_speed > 3) 
				g_speed = 3;
		clear_tab(3,C_BLACK);
			sprintf(cmd_string,"Speed: %d ",g_speed);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
	copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
	
		}
	
	if(ev.code ==261 && ev.value ==1)
		{
			g_speed -=1;
			if(g_speed < 0 ) 
				g_speed = 0;
		
			clear_tab(3,C_BLACK);
			sprintf(cmd_string,"Speed: %d ",g_speed);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
			}

char ss[32];

sprintf(ss,"SET wf_speed=%d",g_speed);


vws_frame_send_text(cnx,ss);
//vws_frame_send_text(cnx,"SET zoom=5 cf=5505"); 
usleep(100000);


}

void update_cf ()
{
}

//---

void shuttle()
{
int fds;
int n, shuttle;
char dev_name[256];
//typedef struct input_event EV;
//EV ev;

//open the shuttleXpress device
strcpy(dev_name,"/dev/input/by-id/usb-Contour_Design_ShuttleXpress-event-if00");
fds = open(dev_name, O_RDONLY); //O_RDONLY| O_NONBLOCK);

if (fds < 0) 
	{
	printf(" Failed to open shuttle device\n");
	return;
    }            
// Flag it as exclusive access
if(ioctl( fds, EVIOCGRAB, 1 ) < 0) 
	{
	printf( "evgrab ioctl\n" );
	return;
    }
printf("Shuttle device connected. dv: %d\n",fds);;

cmd_select = 0;

int deb = 0;
//https://github.com/matthew-wolf-n4mtt/shuttlexpress-mpd/blob/master/shuttlexpress-mpd.c

while(1)
	{
	n=read(fds,&ev,sizeof (ev));
	//printf (" n= %d\n",n);
	if(n > 0)
		{
		//printf(" Data is rxd from the Shuttle N: %d \n",n);
		//printf(" Data recd: *** %d \n",n);
		//if(ev.code ==260)
		printf(" Type: %d Code: %d  Value: %d \n",ev.type,ev.code,ev.value);

		printf(" Count =%d \n",deb++);

		//check for command change
		if(ev.code ==264 && ev.value ==1)
			{
			cmd_select +=1;
			if(cmd_select > 4) cmd_select = 4;
			}
	
		if(ev.code ==260 && ev.value ==1)
			{
			cmd_select -=1;
			if(cmd_select < 0 ) cmd_select = 0;
			}

		//undo all select colours
		for(int cs = 0 ; cs < 5; cs++)
		{
		plot_thick_rectangle(cmd_buf,g_tab_width*cs,0,g_tab_width,CMD_HEIGHT-6,C_YELLOW);
		}

//show the chosen one
plot_thick_rectangle(cmd_buf,g_tab_width*cmd_select,0,g_tab_width,CMD_HEIGHT-6,C_RED);

switch (cmd_select)
	{
	case 0:
        sprintf(cmd_string,"System. ");
        plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
        break;
	case 1:
		sprintf(cmd_string,"Zoom: %d ",g_zoom);
        plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
        break;
	case 2:
		sprintf(cmd_string,"CF: %d ",g_centre_freq);
        plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
        break;
	case 3:
		sprintf(cmd_string,"Speed: %d ",g_speed);
        plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
        break;
	case 4:
		sprintf(cmd_string,"URL: %d ",g_url);
        plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
        break;
	}

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);


// 111
if(cmd_select == ZOOM)
	{
	update_zoom();
	}
/*
	if(ev.code ==263 && ev.value ==1)
		{
			g_zoom +=1;
			if(g_zoom > 14) 
				g_zoom = 14;
		clear_tab(1,C_BLACK);
			sprintf(cmd_string,"Zoom: %d ",g_zoom);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
	copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
	
		}
	
	if(ev.code ==261 && ev.value ==1)
		{
			g_zoom -=1;
			if(g_zoom < 0 ) 
				g_zoom = 0;
		
			clear_tab(1,C_BLACK);
			sprintf(cmd_string,"Zoom: %d ",g_zoom);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
			}
char sz[32];

sprintf(sz,"SET zoom=%d cf=15000",g_zoom);


vws_frame_send_text(cnx,sz);
//vws_frame_send_text(cnx,"SET zoom=5 cf=5505"); 
usleep(100000);
*/
 
//111

// 333
if(cmd_select == SPEED)
	{
update_speed();
/*
	if(ev.code ==263 && ev.value ==1)
		{
			g_speed +=1;
			if(g_speed > 3) 
				g_speed = 3;
		clear_tab(3,C_BLACK);
			sprintf(cmd_string,"Speed: %d ",g_speed);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
	copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
	
		}
	
	if(ev.code ==261 && ev.value ==1)
		{
			g_speed -=1;
			if(g_speed < 0 ) 
				g_speed = 0;
		
			clear_tab(3,C_BLACK);
			sprintf(cmd_string,"Speed: %d ",g_speed);
			plot_large_string(cmd_buf,(cmd_select * 275) +50,40,cmd_string,C_YELLOW);
copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
			}

char ss[32];

sprintf(ss,"SET wf_speed=%d",g_speed);


vws_frame_send_text(cnx,ss);
//vws_frame_send_text(cnx,"SET zoom=5 cf=5505"); 
usleep(100000);
*/


//333

}


printf(" ACT %d \n",cmd_select);

g_centre_freq +=1234;
//update_cmd();
		}
//usleep(100000);
//printf(" \n");
	
}
}
