#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <math.h>
#include <unistd.h>
#include <inttypes.h>

#include "kiwi-conf.h"
//#include "kiwi-lib.h"
#include "kiwi-colours.h"
//#include "waterfall.h"
//#include "qt_jet.h"

extern uint g_centre_freq;
extern uint16_t * cmd_buf;
extern uint g_screen_size_x;


void update_cmd()
{
char freq_string[16];

memset( cmd_buf,0x00,CMD_HEIGHT * g_screen_size_x * 2); //this kills rectangle FIXME
sprintf(freq_string,"CF: %d ",g_centre_freq);

plot_large_string(cmd_buf,670,40,freq_string,C_YELLOW);

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);

}

void shittle()
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

update_cmd();

/*
char freq_string[8];

sprintf(freq_string,"CF: %d ",g_centre_freq);

plot_large_string(cmd_buf,670,40,freq_string,C_YELLOW);

copy_surface_to_framebuf(cmd_buf,0,CMD_POS,g_screen_size_x,CMD_HEIGHT);
*/


while(1)
	{
//printf("looping \n");
n=read(fds,&ev,sizeof (ev));
//printf (" n= %d\n",n);
if(n > 0)
		{
//printf(" Data is rxd from the Shuttle N: %d \n",n);
//printf(" Data recd: *** %d \n",n);
printf(" Type: %d Code: %d  Value: %d \n",ev.type,ev.code,ev.value);

g_centre_freq +=1234;
update_cmd();
		}
//usleep(100000);
//printf(" \n");
	}
}
