#include <linux/input.h>
#include "kiwi-conf.h"
#include "kiwi-colours.h"
#include "kiwi-lib.h"

extern uint g_centre_freq;
extern uint16_t * cmd_buf;
extern uint g_screen_size_x;
extern int box_width;

int cmd_select;

void update_cmd()
{
char cmd_string[16];

for(int cs = 0 ; cs < 5; cs++)
	{
	plot_thick_rectangle(cmd_buf,box_width*cs,0,box_width,CMD_HEIGHT-6,C_YELLOW);
	}

plot_thick_rectangle(cmd_buf,box_width*cmd_select,0,box_width,CMD_HEIGHT-6,C_RED);

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

switch (cmd_select)
	{
	case 0:
        printf("CASE 0.");
        break;
	case 1:
		sprintf(cmd_string,"Zoom: %d ",g_centre_freq);
        plot_large_string(cmd_buf,(cmd_select * 275) +100,40,cmd_string,C_YELLOW);
        break;
	case 2:
		sprintf(cmd_string,"CF: %d ",g_centre_freq);
        plot_large_string(cmd_buf,(cmd_select * 275) +100,40,cmd_string,C_YELLOW);
        break;
	case 3:
		sprintf(cmd_string,"Speed: %d ",g_centre_freq);
        plot_large_string(cmd_buf,(cmd_select * 275) +100,40,cmd_string,C_YELLOW);
        break;
	case 4:
		sprintf(cmd_string,"URL: %d ",g_centre_freq);
        plot_large_string(cmd_buf,(cmd_select * 275) +100,40,cmd_string,C_YELLOW);
        break;
	}

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);




//memset( cmd_buf,0x00,CMD_HEIGHT * g_screen_size_x * 2); //this kills rectangle FIXME
//sprintf(cmd_string,"CF: %d ",g_centre_freq);

//plot_large_string(cmd_buf,670,40,cmd_string,C_YELLOW);

//copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

}

void shuttle()
{
int xx;
printf(" Hello world \n");


int fds;
int n, shuttle;
char dev_name[256];
typedef struct input_event EV;
EV ev;

//open the shuttleXpress device
strcpy(dev_name,"/dev/input/by-id/usb-Contour_Design_ShuttleXpress-event-if00");
//fds = open(dev_name, O_RDONLY| O_NONBLOCK);
fds = open(dev_name, O_RDONLY);

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

// if we get to here, we're connected to shuttleXpress
printf("Shuttle device connected. dv: %d\n",fds);;

cmd_select = 0;
update_cmd();

/*
char freq_string[8];

sprintf(freq_string,"CF: %d ",g_centre_freq);

plot_large_string(cmd_buf,670,40,freq_string,C_YELLOW);

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
*/


int deb = 0;
//https://github.com/matthew-wolf-n4mtt/shuttlexpress-mpd/blob/master/shuttlexpress-mpd.c

while(1)
	{
//printf("looping \n");
n=read(fds,&ev,sizeof (ev));
//printf (" n= %d\n",n);
if(n > 0)
	{
//printf(" Data is rxd from the Shuttle N: %d \n",n);
//printf(" Data recd: *** %d \n",n);
//if(ev.code ==260)
printf(" Type: %d Code: %d  Value: %d \n",ev.type,ev.code,ev.value);

printf(" Count =%d \n",deb++);

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
printf(" ACT %d \n",cmd_select);

g_centre_freq +=1234;
update_cmd();
		}
//usleep(100000);
//printf(" \n");
	}
}
