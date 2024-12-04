

#include <stdbool.h>
#include <unistd.h> 
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>
//#include <liquid.h>
#include <alsa/asoundlib.h>
#include <math.h>




#define SERVER "192.168.2.222" //222"
#define PORT 11361	
#define FFT_SIZE 1024
#define PAK_LEN 1280
#define HEADER_LEN 256

char in_pak_buf[PAK_LEN];
extern char fft_video_buf[FFT_SIZE];

int sock_fd;

int g_audio_sample_rate;
int g_sample_rate;
int g_fft_size;
int g_center_frequency;

bool stream_flag;

int control_packet[32];
struct sockaddr_in si_other;
int slen=sizeof(si_other);








//============================





void die(char *s)
{
	perror(s);
	exit(1);
}


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
   
//get incoming samples from stream 
while(1) 
    {
    //rxd_pak_len=recv(sock_fd, in_pak_buf ,PAK_LEN ,0); //CPX_DATA_SIZE
    //id_type = in_pak_buf[0];//simple type for now

    //if(id_type ==0x42) //needs defining in new header structure
   if(1) 
        {

//printf(" GOT A TYPE 42 \n");

        for(int i=0; i<1024;i++)
            fft_video_buf[i] = i/4; //in_pak_buf[i+HEADER_LEN];
  
        stream_flag = true; //if I don't flag the FFT the CPU usage becomes 100% FIXME
        }
 // usleep(1000);  
/*       
    if(id_type == 0x69)

        { //needs defining in new header structure
        for(int i=0 ;i<1024;i++)
            temp_audio[i] = in_pak_buf[i+ HEADER_LEN];

        for(int i=0 ;i<1024;i++)
            {
            g711_xfer_buf[i] = alaw2linear(temp_audio[i]);
//printf(" t: 0x%x",temp_audio);
            }
//printf(" %d 0x%x \n",__LINE__,in_pak_buf[513]);
        audio_flag = true;
        }
  */          
    usleep(1000);
    }
}



void send_control_packet(int type, int val)
{
control_packet[type] = val;

if (sendto(sock_fd, control_packet, sizeof(control_packet) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    die("control message");
}



//---

int setup_network() 
{ 
int new_data;

char message[80];
int pkt_size;

strcpy(message,"CLIENT calling ");
if ( (sock_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	die("socket");
	
memset((char *) &si_other, 0, sizeof(si_other));
si_other.sin_family = AF_INET;
si_other.sin_port = htons(PORT);

//convert ip address to network byte order	
if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

if (sendto(sock_fd, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    die("sendto()");

printf(" Connected to the server:\n"); 
return 0;
} 

void start_server_stream()
{
pthread_t callback_id,audio_cb_id;
int err;
int num_stages;
int ret;
int  freq;

//read_conf();

//g_sample_rate = 8000;
g_fft_size = FFT_SIZE;

//freq = 198000;
//audio_flag = false;
//audio_sr = 8000; //g_audio_sample_rate;
//audio_sr_delta = AR_DELTA; //correction to let audio run a tad slower to keep its buffer filled.

//err = snd_pcm_open(&audio_device, alsa_device, SND_PCM_STREAM_PLAYBACK, 0);
//if(err !=0)
 //  printf("Error opening Sound Device\n");
//err = snd_pcm_set_params(audio_device,SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,1,audio_sr,1,400000); //latency in uS - Could be dynamic to reduce (unwanted) latency?, 400 ok, 200 ng.
//if(err !=0)
 //  printf("Error with Audio parameters\n"); //audio 

setup_network();
printf(" Network started \n");
usleep(10000);	

//Create a callback thread
ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);

//ret=pthread_create(&audio_cb_id,NULL, (void *)do_audio_pak,NULL);

if(ret==0)
	printf("Network Thread created successfully.\n");
else
	printf("NW Thread not created.\n");
}
