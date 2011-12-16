#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>

#include "video_cs.h"

#include <linux/version.h>
#include <linux/dvb/video.h>
#include <linux/fb.h>
#ifndef AZBOX_GEN_1
	#include <linux/stmfb.h>
#endif

static const char * FILENAME = "video_cs.cpp";

//EVIL

cVideo * videoDecoder = NULL;
unsigned int system_rev = 0x07;



#ifndef VIDEO_FLUSH
#define VIDEO_FLUSH                     _IO('o',  82)
#endif

// this are taken from e2, the player converts them to falid values
#define VIDEO_STREAMTYPE_MPEG2 0
#define VIDEO_STREAMTYPE_MPEG4_H264 1
#define VIDEO_STREAMTYPE_VC1 3
#define VIDEO_STREAMTYPE_MPEG4_Part2 4
#define VIDEO_STREAMTYPE_VC1_SM 5
#define VIDEO_STREAMTYPE_MPEG1 6

//EVIL END



int cVideo::SelectAutoFormat()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return 0;
}

void cVideo::ScalePic()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
}

/* constructor & destructor */
//1
cVideo::cVideo(int mode, void * hChannel, void * hBuffer)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	privateData = (CS_VIDEO_PDATA*)malloc(sizeof(CS_VIDEO_PDATA));

	Open();
}

cVideo::~cVideo(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	Close();

	free(privateData);
}

bool cVideo::Open()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	privateData->m_fd = open("/dev/dvb/adapter0/video0", O_RDWR);
	if(privateData->m_fd > 0)  {

	if (ioctl(privateData->m_fd, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_DEMUX) < 0)
		printf("VIDEO_SELECT_SOURCE failed(%m)");

		return true;

	}

	return false;
}

bool cVideo::Close()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	close(privateData->m_fd);

	return true;
}

void * cVideo::GetDRM(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	return NULL;
}

//3
void * cVideo::GetTVEnc()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	return NULL;
}

//4
void * cVideo::GetTVEncSD()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	return NULL;
}

void * cVideo::GetHandle()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	return NULL;
}

//5
void cVideo::SetAudioHandle(void * handle)
{
	printf("%s:%s - handle=0x%08x\n", FILENAME, __FUNCTION__, (unsigned int) handle);
}

/* aspect ratio */
int cVideo::getAspectRatio(void) {
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	unsigned char buffer[2];
	int n, fd;
	int ratio = 0; // 0 = 4:3, 1 = 16:9
	fd = open("/proc/stb/vmpeg/0/aspect", O_RDONLY);
	n = read(fd, buffer, 2);
	close(fd);
	if (n > 0) {
	    ratio = atoi((const char*) buffer);
	}
	return ratio;
}

int cVideo::setAspectRatio(int aspect, int mode) 
{ 

	printf("%s:%s %d %d\n", FILENAME, __FUNCTION__, aspect, mode); 
	
	int fd;
	
	if (aspect != -1 && aspect != -2)
	{
		const char* sAspect[] =
		{
		   "-asp 4 3",
		   "-asp 16 9"
		};

		az_asp = sAspect[aspect];
	}
	
	const char* sMode[][2] =
	{
		{"0"	, "fullscreen"}, 
		{"1"    , "panscan"}, 
		{"2"	, "letterbox"},
		{"3"   	, "pillarbox"},
		{"4" 	, "vertical center"}
	};
	
	fd = open("/proc/scalingmode", O_WRONLY);
	write(fd, sMode[mode][0], strlen(sMode[mode][0]));
	close(fd);
	
	return 0;
#if 0
    int         n, fd;
    bool        is43 = true;
    unsigned char buffer[128];
    
    printf("%s:%s %d %d\n", FILENAME, __FUNCTION__, aspect, mode); 

    if (aspect != -1)
    {
        const char* sAspect[] =
	{
	   "4:3", /* 1 */
	   "14:9", //not supported
	   "16:9"
        }; 

        fd = open("/proc/stb/video/aspect", O_WRONLY);
	
//aspect starts with 1 :-/
	aspect--;
	
	is43 = (aspect == 1) ? true : false;
	
	write(fd, sAspect[aspect], strlen(sAspect[aspect]));

        close(fd);
    } else
    {
        fd = open("/proc/stb/video/aspect", O_RDONLY);
        n = read(fd, buffer, 128);
	
	if (n > 0)
	{
	   if (strcmp("4:3", (const char*) buffer) == 0)
	      is43 = true;
	   else
	      is43 = false;
	}

        close(fd);
    }

    if (mode != -1)
    {
	
        fd = open("/proc/stb/video/policy", O_WRONLY);
	
	if (is43)
	{     
          const char* sMode[][2] =
	  {
	     {"panscan"      , "letterbox"}, /* 0 */ 
	     {"letterbox"    , "panscan"}, /* fixme: former pillarbox */
	     {"fullscreen"   , "nonlinear"},
	     {"panscan 14:9" , "bestfit"} /* fixme: must change this to scale in menu */
          };
          printf("aspect43: set %s\n", sMode[mode][1]);  
	  write(fd, sMode[mode][1], strlen((const char*) sMode[mode][1]));
        } else
	{
          const char* sMode[][2] =
	  {
	     {"panscan"      , "panscan"}, /* 0 */ 
	     {"letterbox"    , "letterbox"},
	     {"fullscreen"   , "nonlinear"},
	     {"panscan 14:9" , "bestfit"} /* fixme: must change this to scale in menu */
          };
          printf("aspect169: set %s\n", sMode[mode][1]);  
	  write(fd, sMode[mode][1], strlen((const char*) sMode[mode][1]));
	}
        close(fd);
    }

    return 0;
#endif
}

void cVideo::getPictureInfo(int &width, int &height, int &rate) {
	unsigned char buffer[10];
	int n, fd;

	printf("%s:%s\n", FILENAME, __FUNCTION__); 

	rate = 0;
	fd = open("/proc/stb/vmpeg/0/framerate", O_RDONLY);
	n = read(fd, buffer, 10);
	close(fd);

	if (n > 0) {
		sscanf((const char*) buffer, "%X", &rate);
		rate = rate/1000;
	}

	width = 0;
	fd = open("/proc/stb/vmpeg/0/xres", O_RDONLY);
	n = read(fd, buffer, 10);
	close(fd);

	if (n > 0) {
		sscanf((const char*) buffer, "%X", &width);
	}

	height = 0;
	fd = open("/proc/stb/vmpeg/0/yres", O_RDONLY);
	n = read(fd, buffer, 10);
	close(fd);

	if (n > 0) {
		sscanf((const char*) buffer, "%X", &height);
	}

	printf("%s:%s < w %d, h %d, r %d\n", FILENAME, __FUNCTION__, width, height, rate);
}

/* cropping mode */
int cVideo::getCroppingMode(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
int cVideo::setCroppingMode(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }

/* stream source */
int cVideo::getSource(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }

int cVideo::setSource(void) 
{ 
   printf("%s:%s\n", FILENAME, __FUNCTION__); 
   return 0; 
}

/* blank on freeze */
int cVideo::getBlank(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
int cVideo::setBlank(int enable) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }

/* get play state */
int cVideo::getPlayState(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }
void cVideo::HandleDRMMessage(int Event, void *pData) { printf("%s:%s\n", FILENAME, __FUNCTION__); }
void cVideo::HandleVideoMessage(void * hHandle, int Event, void *pData) { printf("%s:%s\n", FILENAME, __FUNCTION__); }

/* change video play state */
int cVideo::Start(void * PcrChannel, unsigned short PcrPid, unsigned short VideoPid, void * hChannel)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
#ifdef AZBOX_GEN_1
	if (ioctl(privateData->m_fd, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_DEMUX) < 0)
		printf("VIDEO_SELECT_SOURCE failed(%m)");
	
	if (ioctl(privateData->m_fd, VIDEO_CONTINUE) < 0)
			printf("VIDEO_CONTINUE failed(%m)");
#else
	if (ioctl(privateData->m_fd, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_DEMUX) < 0)
		printf("VIDEO_SELECT_SOURCE failed(%m)");
	
	if (ioctl(privateData->m_fd, VIDEO_PLAY, 1) < 0)
		printf("VIDEO_PLAY failed(%m)");
#endif

	return true;
}

int cVideo::Stop(bool blank)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	if (ioctl(privateData->m_fd, VIDEO_STOP, blank ? 1:0) < 0)
		printf("VIDEO_STOP failed(%m)");
	
	return true;
}

bool cVideo::Pause(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	if (ioctl(privateData->m_fd, VIDEO_FREEZE, 1) < 0)
		printf("VIDEO_FREEZE failed(%m)");
		
	return true;
}

bool cVideo::Resume(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	if (ioctl(privateData->m_fd, VIDEO_CONTINUE, 1) < 0)
		printf("VIDEO_CONTINUE failed(%m)");
		
	return true;
}


int cVideo::LipsyncAdjust() { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }

int64_t cVideo::GetPTS(void) { printf("%s:%s\n", FILENAME, __FUNCTION__); return 0; }

int cVideo::Flush(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	if (ioctl(privateData->m_fd, VIDEO_CLEAR_BUFFER, 1) < 0)
		printf("VIDEO_CLEAR_BUFFER failed(%m)");
	
	return 0;
}

/* set video_system */
//2
int cVideo::SetVideoSystem(int video_system, bool remember)
{

	const char *MehdmiModes[][2] = {
	{"VIDEO_STD_480P", 	"-digital -cs rgb_16_235 -f HDMI_480p60"},
	{"VIDEO_STD_576P", 	"-digital -cs rgb_16_235 -f HDMI_576p50"},
	{"VIDEO_STD_720P50", 	"-digital -cs rgb_16_235 -f HDMI_720p50"},
	{"VIDEO_STD_720P60", 	"-digital -cs rgb_16_235 -f HDMI_720p60"},
	{"VIDEO_STD_1080I50", 	"-digital -cs rgb_16_235 -f HDMI_1080i50"},
	{"VIDEO_STD_1080I60", 	"-digital -cs rgb_16_235 -f HDMI_1080i60"},
	{"VIDEO_STD_1080P50", 	"-digital -cs rgb_16_235 -f HDMI_1080p50"},
	{"VIDEO_STD_1080P60", 	"-digital -cs rgb_16_235- f HDMI_1080p60"},
	};
	
	const char *ComponentModes[][2] = {
	{"VIDEO_STD_480P", "-component -f 480p60"},
	{"VIDEO_STD_576P", "-component -f 576p50"},
	{"VIDEO_STD_720P50", 	"-component -f 720p50"},
	{"VIDEO_STD_720P60", "-component -f 720p60"},
	{"VIDEO_STD_1080I50", 	"-component -f 1080i50"},
	{"VIDEO_STD_1080I60", "-component -f 1080i60"},
	{"VIDEO_STD_1080P50", "-component -f 1080p50"},
	{"VIDEO_STD_1080P60", "-component -f 1080p60"},
	};
	
	const char *AnalogModes[][2] = {
	{"VIDEO_STD_480P", "-analog -f NTSC_M"},
	{"VIDEO_STD_576P", "-analog -f PAL_BG"},
	{"VIDEO_STD_720P50", 	"-analog -f PAL_BG"},
	{"VIDEO_STD_720P60", "-analog -f NTSC_M"},
	{"VIDEO_STD_1080I50", 	"-analog -f PAL_BG"},
	{"VIDEO_STD_1080I60", "-analog -f NTSC_M"},
	{"VIDEO_STD_1080P50", "-analog -f PAL_BG"},
	{"VIDEO_STD_1080P60", "-analog -f NTSC_M"},
	};

//	printf("%s:%s - video_system=%s remember=%s\n", FILENAME, __FUNCTION__, 
//		aVideoSystems[video_system][0], remember?"true":"false");
		
#ifdef AZBOX_GEN_1
	//Write vstring to file
	FILE * pFile;
	pFile = fopen ("/etc/.vstring","w");
	fprintf (pFile, "-null %s %s %s %s %s -audio_engine 0 -sf 48000\n", MehdmiModes[video_system][1], az_asp, ComponentModes[video_system][1], AnalogModes[video_system][1], az_asp);
	fclose (pFile);
//ComponentModes[video_system][1],
	printf("-null %s %s %s %s %s -audio_engine 0 -sf 48000\n", MehdmiModes[video_system][1], az_asp, ComponentModes[video_system][1], AnalogModes[video_system][1], az_asp);
//ComponentModes[video_system][1]
#else
	int fd = open("/proc/stb/video/videomode", O_RDWR);
	write(fd, aVideoSystems[video_system][1], strlen(aVideoSystems[video_system][1]));
	close(fd);
#endif
	return 0;
}

int cVideo::SetStreamType(VIDEO_FORMAT type) {

	const char *aVIDEOFORMAT[] = {
	"VIDEO_FORMAT_MPEG2",
	"VIDEO_FORMAT_MPEG4",
	"VIDEO_FORMAT_VC1",
	"VIDEO_FORMAT_JPEG",
	"VIDEO_FORMAT_GIF",
	"VIDEO_FORMAT_PNG"
	};

	printf("%s:%s - type=%s\n", FILENAME, __FUNCTION__, aVIDEOFORMAT[type]);
	
	int streamtype = VIDEO_STREAMTYPE_MPEG2;

	switch(type)
	{
	default:
	case VIDEO_FORMAT_MPEG2:
		streamtype = VIDEO_STREAMTYPE_MPEG2;
		break;
	case VIDEO_FORMAT_MPEG4:
		streamtype = VIDEO_STREAMTYPE_MPEG4_H264;
		break;
	case VIDEO_FORMAT_VC1:
		streamtype = VIDEO_STREAMTYPE_VC1;
		break;
	}

/*
	if(mode != 2) {
	
	if (ioctl(privateData->m_fd, VIDEO_SELECT_SOURCE, 0) < 0)
		printf("VIDEO_SELECT_SOURCE failed(%m)");
	} else {
	
	if (ioctl(privateData->m_fd, VIDEO_SELECT_SOURCE, 2) < 0)
		printf("VIDEO_SELECT_SOURCE failed(%m)");

	}*/

	if (ioctl(privateData->m_fd, VIDEO_SET_STREAMTYPE, streamtype) < 0)
		printf("VIDEO_SET_STREAMTYPE failed(%m)");

	return 0;
}

//7
void cVideo::SetSyncMode(AVSYNC_TYPE mode)
{
        int clock;
	
	const char *aAVSYNCTYPE[] = {
	"AVSYNC_DISABLED",
	"AVSYNC_ENABLED",
	"AVSYNC_AUDIO_IS_MASTER"
	};

        const char* av_modes[] = {
	"disapply",
	"apply"
	};

        const char* master_clock[] = {
	"video",
	"audio"
	};
      
	printf("%s:%s - mode=%s\n", FILENAME, __FUNCTION__, aAVSYNCTYPE[mode]);

	int fd = open("/proc/stb/stream/policy/AV_SYNC", O_RDWR);

        if (fd > 0)  
        {
           if ((mode == 0) || (mode == 1))
	   {
	      write(fd, av_modes[mode], strlen(av_modes[mode]));
	      clock = 0;
	   } else
           {
	      write(fd, av_modes[1], strlen(av_modes[1]));
	      clock = 1;
	   }
	   close(fd);
        }
	else
	   printf("error %m\n");
	
        printf("set master clock = %s\n", master_clock[clock]);

	fd = open("/proc/stb/stream/policy/MASTER_CLOCK", O_RDWR);
        if (fd > 0)  
        {
	   write(fd, master_clock[clock], strlen(master_clock[clock]));
	   close(fd);
        }
	else
	   printf("error %m\n");
}

//8
void cVideo::ShowPicture(const char * fname)
{
	printf("%s:%s - fname=%s\n", FILENAME, __FUNCTION__, fname);
	
	int f = open(fname, O_RDONLY);
	if (f >= 0)
	{
		struct stat s;
		size_t written=0;
		fstat(f, &s);
		
		if ( privateData->m_fd == -1)
			privateData->m_fd = open("/dev/dvb/adapter0/video0", O_WRONLY);
		if (privateData->m_fd >= 0)
		{
			bool seq_end_avail = false;
			size_t pos=0;
			unsigned char pes_header[] = { 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x80, 0x00, 0x00 };
			unsigned char seq_end[] = { 0x00, 0x00, 0x01, 0xB7 };
			unsigned char iframe[s.st_size];
			unsigned char stuffing[8192];
			int streamtype = VIDEO_STREAMTYPE_MPEG2;
			memset(stuffing, 0, 8192);
			read(f, iframe, s.st_size);
			
			if (ioctl(privateData->m_fd, VIDEO_SELECT_SOURCE, 2) < 0)
				printf("VIDEO_SELECT_SOURCE MEMORY failed");
			if (ioctl(privateData->m_fd, VIDEO_SET_STREAMTYPE, streamtype) < 0)
				printf("VIDEO_SET_STREAMTYPE failed");
			if (ioctl(privateData->m_fd, VIDEO_PLAY) < 0)
				printf("VIDEO_PLAY failed");
			
			while(pos <= (s.st_size-4) && !(seq_end_avail = (!iframe[pos] && !iframe[pos+1] && iframe[pos+2] == 1 && iframe[pos+3] == 0xB7)))
				++pos;
			if ((iframe[3] >> 4) != 0xE) // no pes header
				write(privateData->m_fd, pes_header, sizeof(pes_header));
			else
				iframe[4] = iframe[5] = 0x00;
			write(privateData->m_fd, iframe, s.st_size);
			if (!seq_end_avail)
				write(privateData->m_fd, seq_end, sizeof(seq_end));
			write(privateData->m_fd, stuffing, 8192);
		}
		close(f);
	}
	else
	{
		printf("couldnt open %s", fname);
	}
}

void cVideo::StopPicture()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
}

void cVideo::Standby(unsigned int bOn)
{
	printf("%s:%s - bOn=%d\n", FILENAME, __FUNCTION__, bOn);

	char filename[] = "/tmp/.standby";

	if (bOn)
	{
		system("touch /tmp/.standby");
	}
	else
	{
		unlink("/tmp/.standby");
	}
#ifndef AZBOX_GEN_1
	int fd_avs = open("/proc/stb/avs/0/standby", O_RDWR);
	int fd_hdmi  = open("/dev/fb0",   O_RDWR);

#if defined(PLAYER179)
	struct stmfbio_output_configuration outputConfig = {STMFBIO_OUTPUTID_MAIN};
#elif defined(PLAYER131)
	struct stmfbio_output_configuration outputConfig = {0};
	outputConfig.outputid = 1;
#endif

	if(ioctl(fd_hdmi, STMFBIO_GET_OUTPUT_CONFIG, &outputConfig)<0)
		printf("Getting current output configuration failed\n");
  
	outputConfig.caps = 0;
	outputConfig.activate = STMFBIO_ACTIVATE_IMMEDIATE;
	outputConfig.analogue_config = 0;

	outputConfig.caps |= STMFBIO_OUTPUT_CAPS_HDMI_CONFIG;

	if (bOn) {
		outputConfig.hdmi_config |= STMFBIO_OUTPUT_HDMI_DISABLED;
	} else {
		outputConfig.hdmi_config &= ~STMFBIO_OUTPUT_HDMI_DISABLED;
	}

	if(outputConfig.caps != STMFBIO_OUTPUT_CAPS_NONE)
	{
		if(ioctl(fd_hdmi, STMFBIO_SET_OUTPUT_CONFIG, &outputConfig)<0)
			printf("setting output configuration failed\n");
	}

	int fd_avs_input = open("/proc/stb/avs/0/input", O_RDWR);

	if (bOn)
	{
		write(fd_avs, "on", 2);
		write(fd_avs_input, "encoder", 7);
	}
	else
	{
		write(fd_avs, "off", 3);
		write(fd_avs_input, "scart", 5);
	}
	close(fd_avs_input);
	close(fd_avs);
	close(fd_hdmi);
#endif
}

void cVideo::Pig(int x, int y, int w, int h, int osd_w, int osd_h)
{
	printf("%s:%s - x=%d y=%d w=%d h=%d osd_w=%d osd_h=%d\n", FILENAME, __FUNCTION__, 
		x, y, w, h, osd_w, osd_h);
}

void cVideo::setContrast(int val)
{
	printf("%s:%s - val=%d\n", FILENAME, __FUNCTION__, val);
}


void cVideo::SetFastBlank(bool onoff)
{
	printf("%s:%s - onoff=%s\n", FILENAME, __FUNCTION__, onoff?"true":"false");
}

void cVideo::SetTVAV(bool onoff)
{
	printf("%s:%s - onoff=%s\n", FILENAME, __FUNCTION__, onoff?"true":"false");
}

void cVideo::SetWideScreen(bool onoff)
{
	printf("%s:%s - onoff=%s\n", FILENAME, __FUNCTION__, onoff?"true":"false");

	const char *wss_auto_off="auto(4:3_off)";
	const char *wss_auto="auto";

	int fd = open("/proc/stb/denc/0/wss", O_RDWR);
	if(onoff)
		write(fd, wss_auto, strlen(wss_auto));
	else
		write(fd, wss_auto_off, strlen(wss_auto_off));
	close(fd);
}

//9
void cVideo::SetVideoMode(analog_mode_t mode)
{
	const char *aANALOGMODESCART[] = {
	"ANALOG_SD_RGB_SCART",
	"ANALOG_SD_YPRPB_SCART",
	"ANALOG_HD_RGB_SCART",
	"ANALOG_HD_YPRPB_SCART"
	};
	
	const char *aANALOGMODECINCH[] = {
	"ANALOG_SD_RGB_CINCH",
	"ANALOG_SD_YPRPB_CINCH",
	"ANALOG_HD_RGB_CINCH",
	"ANALOG_HD_YPRPB_CINCH"
	};

	if(mode && 0x80 != 0x80)
		printf("%s:%s - mode=%s\n", FILENAME, __FUNCTION__, aANALOGMODESCART[mode&0x7F]);
	else	
		printf("%s:%s - mode=%s\n", FILENAME, __FUNCTION__, aANALOGMODECINCH[mode&0x7F]);
}

//6
void cVideo::SetDBDR(int dbdr)
{
	const char *aDBDR[] = {
	"VIDEO_DB_DR_NEITHER",
	"VIDEO_DB_ON",
	"VIDEO_DB_DR_BOTH"
	};
	
	printf("%s:%s - dbdr=%s\n", FILENAME, __FUNCTION__, aDBDR[dbdr]);
}


