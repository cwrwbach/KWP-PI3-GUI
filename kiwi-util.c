#include <stdbool.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <vws/websocket.h>

#define FFT_SIZE 1024
#define PAK_LEN 1280
#define HEADER_LEN 256

extern char fft_video_buf[FFT_SIZE];
bool stream_flag;

vws_cnx* cnx;
int debug;
int watch_dog;

//============================

setup_kiwi()
{
cnx = vws_cnx_new();    
char uri_string[256];
pthread_t callback_id,audio_cb_id;
int ret;


printf(" LINE %d \n",__LINE__);

// Set connection timeout to 2 seconds (the default is 10). This applies
// both to connect() and to read operations (i.e. poll()).
vws_socket_set_timeout((vws_socket*)cnx, 5);

printf(" LINE %d \n",__LINE__);

time_t utc_now = time( NULL );
printf(" utc %d \n" , utc_now);

//Complete 'GET' header string is:
sprintf(uri_string,"ws://norsom.proxy.kiwisdr.com:8073/%d/W/F",utc_now);
printf("Header string: %s\n",uri_string);

if (vws_connect(cnx, uri_string) == false)
    {
    printf("Failed to connect to the WebSocket server\n");
    vws_cnx_free(cnx);
    return 1;
    }

// Can check connection state this way. 
assert(vws_socket_is_connected((vws_socket*)cnx) == true);

// Enable tracing - dump frames to the console in human-readable format.
//vws.tracelevel = VT_PROTOCOL;

//Commands to the KIWISDR to set up a waterfall
// Send a TEXT frame
vws_frame_send_text(cnx, "SET auth t=kiwi p=");
usleep(100000);
vws_frame_send_text(cnx,"SET zoom=8 cf=15000");
usleep(100000);
vws_frame_send_text(cnx,"SET maxdb=0 mindb=-100");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_speed=1");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_comp=0");
usleep(100000);
vws_frame_send_text(cnx,"SET ident_user=Lowa Wather");
printf(" Line %d \n",__LINE__);

debug = 0;
watch_dog=0;    

//ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);

/*
while(1)
    {   
   vws_msg* reply = vws_msg_recv(cnx);

    if (reply == NULL)
        {
        printf(" No Message  recd. Line: %d \n",__LINE__);
        // There was no message received and it resulted in timeout
        }
    else
        {
        // Free message
        printf(" Received: %d \n",debug++);
        if(watch_dog++ > 30)
            {
            watch_dog = 0;
            vws_frame_send_text(cnx,"SET keepalive");
            }

        for(int i = 0; i< 800;i++)
            {
              fft_video_buf[i] = reply->data->data[i]; //signed dB
            }
        vws_msg_free(reply);   
        
        stream_flag = true; //if I don't flag the FFT the CPU usage becomes 100% FIXME
        //draw_trace_fft();
        
    printf(" LOOPIN RXD %d \n",debug++);
	}
	
    }   
    * */ 
}



read_kiwi_line()
    {   
   vws_msg* reply = vws_msg_recv(cnx);

    if (reply == NULL)
        {
        printf(" No Message  recd. Line: %d \n",__LINE__);
        // There was no message received and it resulted in timeout
        }
    else
        {
        // Free message
        printf(" Rxd: %d \n",debug++);
        if(watch_dog++ > 30)
            {
            watch_dog = 0;
            vws_frame_send_text(cnx,"SET keepalive");
            }

        for(int i = 0; i< 800;i++)
            {
              fft_video_buf[i] = reply->data->data[i]; //signed dB
            }
        vws_msg_free(reply);   
        
        stream_flag = true; //if I don't flag the FFT the CPU usage becomes 100% FIXME
        //draw_trace_fft();
        
    printf("Kiwi-line %d ",debug++);
	}
	
 }



























/*
void * server_callback(void)
{
    
int i;
unsigned int fft_count;
int rxd_pak_len;
float audio;
int rbi;
static int local_count;
int rxd_count;
char id_type;
char temp_audio[1024];
   
//return;   
   
}
*/

//---
/*
void start_server_stream()
{
pthread_t callback_id;
int err;
//int num_stages;
int ret;
//int  freq;


//setup_network();
printf(" Network started \n");
usleep(10000);	

//Create a callback thread
ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);


if(ret==0)
	printf("Network Thread created successfully.\n");
else
	printf("NW Thread not created.\n");
}

*/
