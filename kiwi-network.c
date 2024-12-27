#include <stdbool.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <vws/websocket.h>
#include "avc-colours.h"

#define FFT_SIZE 1024

extern int8_t kiwi_buf[FFT_SIZE];
vws_cnx* cnx;
int debug;
int watch_dog;
extern uint8_t qtj[3];
void * read_kiwi_line();
extern pthread_t callback_id;
void draw_spectrum(short);
void draw_waterfall();

//============================

int qt_jet(int i)
{        
uint8_t col[3] = {255,255,255};
//uint8_t shade[3] = {255,255,255} ;  
  
    if (i<43)
        {col[0] = 0; col[1] = 0; col[2] = 255*i/43;}
        //col[3] = { 0,0, 255*(i)/43};

    if( (i>=43) && (i<87) )
         {col[0] = 0; col[1] = 255*(i-43)/43; col[2] = 255;}
        //col = ( 0, 255*(i-43)/43, 255 );

    if( (i>=87) && (i<120) )
         {col[0] = 0; col[1] = 255; col[2] = 255-(255*(i-87)/32);}
        //col = ( 0,255, 255-(255*(i-87)/32));

    if( (i>=120) && (i<154) )
        {col[0] = (255*(i-120)/33); col[1] = 255; col[2] = 0;}
        //col = ( (255*(i-120)/33), 255, 0);

    if( (i>=154) && (i<217) )
        {col[0] = 255; col[1] = 255 - (255*(i-154)/62); col[2] = 0;}
        //col = ( 255, 255 - (255*(i-154)/62), 0);

    if (i>=217)
        {col[0] = 255; col[1] = 0; col[2] = (128*(i-217)/38);}
           //col = ( 255, 0, 128*(i-217)/38);
 qtj[0] = col[0]; 
 qtj[1] = col[1]; 
 qtj[2] = col[2]; 
}

void  * setup_kiwi()
{
cnx = vws_cnx_new();    
char uri_string[256];
//pthread_t callback_id,audio_cb_id;
int ret;
char fft_video_buf[1536];

// Set connection timeout to 5 seconds (the default is 10). This applies
// both to connect() and to read operations (i.e. poll()).
vws_socket_set_timeout((vws_socket*)cnx, 5);
time_t utc_now = time( NULL );
printf(" utc %d \n" , utc_now);

//http://81.168.1.206:8073/
//http://shack2.ddns.net:8073/
//http://80m.live:8079/
//Complete 'GET' header string is:
//sprintf(uri_string,"ws://81.168.1.206:8073/%d/W/F",utc_now);
sprintf(uri_string,"ws://norsom.proxy.kiwisdr.com:8073/%d/W/F",utc_now);
printf("Header string: %s\n",uri_string);

if (vws_connect(cnx, uri_string) == false)
    {
    printf("Failed to connect to the WebSocket server\n");
    vws_cnx_free(cnx);
    //return; // 1;
    }

// Can check connection state this way. 
assert(vws_socket_is_connected((vws_socket*)cnx) == true);

// Enable tracing - dump frames to the console in human-readable format.
//vws.tracelevel = VT_PROTOCOL;

//Commands to the KIWISDR to set up a waterfall
// Send a TEXT frame
vws_frame_send_text(cnx, "SET auth t=kiwi p=");
usleep(100000);
vws_frame_send_text(cnx,"SET zoom=4 cf=17586");
usleep(100000);
vws_frame_send_text(cnx,"SET maxdb=-50 mindb=-110");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_speed=3");
usleep(100000);
vws_frame_send_text(cnx,"SET wf_comp=0");
usleep(100000);
vws_frame_send_text(cnx,"SET ident_user=Squire");
//printf(" Line %d \n",__LINE__);

debug = 0;
watch_dog=0;    

int8_t temp;
char show[32];
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
        //printf(" Received: %d \n",debug++);
        if(watch_dog++ > 30)
            {
            watch_dog = 0;
            vws_frame_send_text(cnx,"SET keepalive");
            }

        if(strncmp("W/F",reply->data->data,3)==0)
            {
            for(int i = 0; i< 1024;i++)
                {
                temp = reply->data->data[i+16]; 
                kiwi_buf[i] = temp ; //signed dB
                }

strncpy(show,reply->data->data,16);
//for(int n=3;n<16;n++)
//    printf("0x%x ",show[n]);
//printf("\n");


            }
        vws_msg_free(reply);   
 
        draw_spectrum(C_WHITE);
        draw_waterfall();      
        }
    }   
}
