
//#include "shuttle.h"



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
		}
//usleep(100000);
//printf(" \n");
	}
}
