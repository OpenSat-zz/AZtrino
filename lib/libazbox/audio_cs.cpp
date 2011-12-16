#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>

#include "audio_cs.h"
#include <linux/dvb/audio.h>

static const char * FILENAME = "audio_cs.cpp";

//EVIL

cAudio * audioDecoder = NULL;

#define AUDIO_BYPASS_ON 0
#define AUDIO_BYPASS_OFF 1

// this are taken from e2, the player converts them to falid values
 //AUDIO_ENCODING_AC3
#define AUDIO_STREAMTYPE_AC3 0
 //AUDIO_ENCODING_MPEG2
#define AUDIO_STREAMTYPE_MPEG 1
//AUDIO_ENCODING_DTS
#define AUDIO_STREAMTYPE_DTS 2

#define AUDIO_ENCODING_LPCM 2
#define AUDIO_ENCODING_LPCMA 11
#ifndef AUDIO_SET_ENCODING 
#define AUDIO_SET_ENCODING              _IO('o',  70)
#endif
#ifndef AUDIO_FLUSH 
#define AUDIO_FLUSH                     _IO('o',  71)
#endif
//EVIL END


/* internal methods */
int cAudio::setBypassMode(int disable)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	return 0;
}

int cAudio::LipsyncAdjust(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	return 0;
}

/* construct & destruct */
cAudio::cAudio(void * hBuffer, void * encHD, void * encSD)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	privateData = (CS_AUDIO_PDATA*)malloc(sizeof(CS_AUDIO_PDATA));

	Open();

	cEncodedDataOnSPDIF = 0;
	cEncodedDataOnHDMI = 0;
	Muted = false;

	StreamType = AUDIO_FMT_MPEG;
	SyncMode = AUDIO_NO_SYNC;
	started = false;
	uAudioPTSDelay = 0;
	uAudioDolbyPTSDelay = 0;
	uAudioMpegPTSDelay = 0;
	receivedDelay = false;

	atten = 0;
	volume = 0;

	clip_started = false;
	hdmiDD = false;
	spdifDD = false;
	hasMuteScheduled = false;
}


cAudio::~cAudio(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	Close();

	free(privateData);
}

bool cAudio::Open()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	privateData->m_fd = open("/dev/dvb/adapter0/audio0", O_RDWR);
	if(privateData->m_fd > 0) {

	if (ioctl(privateData->m_fd, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX) < 0)
		printf("AUDIO_SELECT_SOURCE failed(%m)");

		return true;

}
	return false;
}

bool cAudio::Close()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	close(privateData->m_fd);

	return true;
}

void * cAudio::GetHandle()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return NULL;
}

void * cAudio::GetDSP()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return NULL;
}

void cAudio::HandleAudioMessage(int Event, void *pData)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
}

void cAudio::HandlePcmMessage(int Event, void *pData)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
}

/* shut up */
int cAudio::mute(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	Muted = true;
	
	SetMute(Muted);

	return 0;
}

int cAudio::unmute(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	Muted = false;
	
	SetMute(Muted);

/*	char sVolume[4];
	int fd = open("/proc/stb/avs/0/volume", O_RDWR);
	read(fd, sVolume, 4);

	write(fd, sVolume, strlen(sVolume));
	close(fd);
*/
	return 0;
}

int cAudio::SetMute(int enable)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	Muted = enable?true:false;
	
	char sMuted[4];
	sprintf(sMuted, "%d", Muted);


	int fd = open("/proc/stb/audio/j1_mute", O_RDWR);
	write(fd, sMuted, strlen(sMuted));
	close(fd);

	return 0;
}


/* bypass audio to external decoder */
int cAudio::enableBypass(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	if (ioctl(privateData->m_fd, AUDIO_SET_BYPASS_MODE, AUDIO_BYPASS_ON) < 0)
		printf("AUDIO_SET_BYPASS_MODE failed(%m)");

	return 0;
}

int cAudio::disableBypass(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	if (ioctl(privateData->m_fd, AUDIO_SET_BYPASS_MODE, AUDIO_BYPASS_OFF) < 0)
		printf("AUDIO_SET_BYPASS_MODE failed(%m)");

	return 0;
}

/* volume, min = 0, max = 100 */
/* e2 sets 0 to 63 */
int cAudio::setVolume(unsigned int left, unsigned int right)
{
	printf("%s:%s left %u right %u\n", FILENAME, __FUNCTION__, left, right);
	
	volume = left;

	char sVolume[4];
	sprintf(sVolume, "%d", (int)(63-(int)(volume * 0.63)));

	int fd = open("/proc/stb/avs/0/volume", O_RDWR);
	write(fd, sVolume, strlen(sVolume));
	close(fd);
	
	return 0;
}


/* start and stop audio */
int cAudio::Start(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

#ifdef AZBOX_GEN_1
	if (ioctl(privateData->m_fd, AUDIO_CONTINUE, 1) < 0)
		printf("AUDIO_CONTINUE failed(%m)");
#else
	if (ioctl(privateData->m_fd, AUDIO_PLAY, 1) < 0)
		printf("AUDIO_PLAY failed(%m)");
#endif

	return 0;
}

int cAudio::Stop(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	if (ioctl(privateData->m_fd, AUDIO_STOP, 1) < 0)
		printf("AUDIO_STOP failed(%m)");

	return 0;
}

bool cAudio::Pause(bool Pcm)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	if (ioctl(privateData->m_fd, AUDIO_PAUSE, 1) < 0)
		printf("AUDIO_PAUSE failed(%m)");

	return true;
}

bool cAudio::Resume(bool Pcm)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	if (ioctl(privateData->m_fd, AUDIO_CONTINUE, 1) < 0)
		printf("AUDIO_CONTINUE failed(%m)");
	
	return true;
}

void cAudio::SetStreamType(AUDIO_FORMAT type)
{
	char *aAUDIOFORMAT[] = {
		"AUDIO_FMT_AUTO",
		"AUDIO_FMT_MPEG",
		"AUDIO_FMT_MP3",
		"AUDIO_FMT_DOLBY_DIGITAL",
		"AUDIO_FMT_AAC",
		"AUDIO_FMT_AAC_PLUS",
		"AUDIO_FMT_DD_PLUS",
		"AUDIO_FMT_DTS",
		"AUDIO_FMT_AVS",
		"AUDIO_FMT_MLP",
		"AUDIO_FMT_WMA",
	};

	printf("%s:%s - type=%s\n", FILENAME, __FUNCTION__, aAUDIOFORMAT[type]);

	int bypass = AUDIO_STREAMTYPE_MPEG;

	switch(type)
	{
	case AUDIO_FMT_DOLBY_DIGITAL: 	bypass = AUDIO_STREAMTYPE_AC3;  break;
	case AUDIO_FMT_DTS: 		bypass = AUDIO_STREAMTYPE_DTS;  break;
	case AUDIO_FMT_MPEG: default:	bypass = AUDIO_STREAMTYPE_MPEG; break;
	}

	

	// Normaly the encoding should be set using AUDIO_SET_ENCODING
	// But as we implemented the behavior to bypass (cause of e2) this is correct here
	if (ioctl(privateData->m_fd, AUDIO_SET_BYPASS_MODE, bypass) < 0)
		printf("AUDIO_SET_BYPASS_MODE(%m)");

	StreamType = type;
};

void cAudio::SetSyncMode(AVSYNC_TYPE Mode)
{
//Dagobert: noop here all will be done in video_cs.cpp ->SetSyncMode
	printf("%s:%s\n", FILENAME, __FUNCTION__);
}


/* stream source */
int cAudio::getSource(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return 0;
}

int cAudio::setSource(int source)
{
	printf("%s:%s - source=%d\n", FILENAME, __FUNCTION__, source);
	


	return 0;
}

int cAudio::Flush(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	if (ioctl(privateData->m_fd, AUDIO_FLUSH, NULL) < 0)
		printf("AUDIO_FLUSH failed(%m)");

	return 0;
}


/* select channels */
int cAudio::setChannel(int channel)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return 0;
}

int cAudio::getChannel(void)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return 0;
}

static unsigned int SubFrameLen = 0;
static unsigned int SubFramesPerPES = 0;

static const unsigned char clpcm_pes[18] = {   0x00, 0x00, 0x01, 0xBD, //start code
					0x07, 0xF1,             //pes length
					0x81, 0x81, 0x09,       //fixed
					0x21, 0x00, 0x01, 0x00, 0x01, //PTS marker bits
					0x1E, 0x60, 0x0A,           //first pes only, 0xFF after
					0xFF
			};
static const unsigned char clpcm_prv[14] = {   0xA0,   //sub_stream_id
					0, 0,   //resvd and UPC_EAN_ISRC stuff, unused
					0x0A,   //private header length
					0, 9,   //first_access_unit_pointer
					0x00,   //emph,rsvd,stereo,downmix
					0x0F,   //quantisation word length 1,2
					0x0F,   //audio sampling freqency 1,2
					0,              //resvd, multi channel type
					0,              //bit shift on channel GR2, assignment
					0x80,   //dynamic range control
					0, 0    //resvd for copyright management
			};

static unsigned char lpcm_pes[18];
static unsigned char lpcm_prv[14];

static unsigned char breakBuffer[8192];
static unsigned int breakBufferFillSize = 0;

//Neutrino uses softdecoder to playback music, this has to be insertas pcm in the player
int cAudio::PrepareClipPlay(int uNoOfChannels, int uSampleRate, int uBitsPerSample, int bLittleEndian)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	privateData->uNoOfChannels = uNoOfChannels;
	privateData->uSampleRate = uSampleRate;
	privateData->uBitsPerSample = uBitsPerSample;
	privateData->bLittleEndian = bLittleEndian;

	printf("rate: %d ch: %d bits: %d (%d bps)\n",
		uSampleRate/*Format->dwSamplesPerSec*/,
		uNoOfChannels/*Format->wChannels*/,
		uBitsPerSample/*Format->wBitsPerSample*/,
		(uBitsPerSample/*Format->wBitsPerSample*/ / 8)
	);

	SubFrameLen = 0;
	SubFramesPerPES = 0;
	breakBufferFillSize = 0;

	memcpy(lpcm_pes, clpcm_pes, sizeof(lpcm_pes));
	memcpy(lpcm_prv, clpcm_prv, sizeof(lpcm_prv));

	//figure out size of subframe
	//and set up sample rate
	switch(uSampleRate) {
		case 48000:             SubFrameLen = 40;
				                break;
		case 96000:             lpcm_prv[8] |= 0x10;
				                SubFrameLen = 80;
				                break;
		case 192000:    lpcm_prv[8] |= 0x20;
				                SubFrameLen = 160;
				                break;
		case 44100:             lpcm_prv[8] |= 0x80;
				                SubFrameLen = 40;
				                break;
		case 88200:             lpcm_prv[8] |= 0x90;
				                SubFrameLen = 80;
				                break;
		case 176400:    lpcm_prv[8] |= 0xA0;
				                SubFrameLen = 160;
				                break;
		default:                break;
	}

	SubFrameLen *= uNoOfChannels;
	SubFrameLen *= (uBitsPerSample / 8);

	//rewrite PES size to have as many complete subframes per PES as we can
	SubFramesPerPES = ((2048-sizeof(lpcm_pes))-sizeof(lpcm_prv))/SubFrameLen;
	SubFrameLen *= SubFramesPerPES;

	lpcm_pes[4] = ((SubFrameLen+(sizeof(lpcm_pes)-6)+sizeof(lpcm_prv))>>8) & 0xFF;
	lpcm_pes[5] =  (SubFrameLen+(sizeof(lpcm_pes)-6)+sizeof(lpcm_prv))     & 0xFF;

	//set number of channels
	lpcm_prv[10]  = uNoOfChannels - 1;

	switch(uBitsPerSample) {
		case    16: break;
		case    24: lpcm_prv[7] |= 0x20;
				        break;
		default:        printf("inappropriate bits per sample (%d) - must be 16 or 24\n",uBitsPerSample);
				        return 1;
	}

	
	if (ioctl(privateData->m_fd, AUDIO_SET_ENCODING, (void*)AUDIO_ENCODING_LPCMA) < 0)
		printf("AUDIO_SET_ENCODING(%m)");

	if (ioctl(privateData->m_fd, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_MEMORY) < 0)
		printf("AUDIO_SELECT_SOURCE failed(%m)");

	Start();

	return 0;
}



int cAudio::WriteClip(unsigned char * buffer, int size)
{
	//printf("%s:%s - size=%d (SubFrameLen=%d)\n", FILENAME, __FUNCTION__, size, SubFrameLen);
	
	unsigned int qty;
	unsigned int n;
	unsigned int injectBufferSize = sizeof(lpcm_pes)+sizeof(lpcm_prv)+SubFrameLen;
	unsigned char * injectBuffer = (unsigned char *)malloc(sizeof(unsigned char)*injectBufferSize);
	unsigned char * injectBufferDataPointer = &injectBuffer[sizeof(lpcm_pes)+sizeof(lpcm_prv)];

	for(int pos = 0; pos < size; )
	{
		//printf("%s:%s - Position=%d\n", FILENAME, __FUNCTION__, pos);
		if((size - pos) < SubFrameLen)
		{
			breakBufferFillSize = size - pos;
			memcpy(breakBuffer, &buffer[pos], sizeof(unsigned char)*breakBufferFillSize);
			//printf("%s:%s - Unplayed=%d\n", FILENAME, __FUNCTION__, breakBufferFillSize);
			break;
		}

                //get first PES's worth
		if(breakBufferFillSize > 0)
		{
			memcpy(injectBufferDataPointer, breakBuffer, sizeof(unsigned char)*breakBufferFillSize);
			memcpy(&injectBufferDataPointer[breakBufferFillSize], &buffer[pos], sizeof(unsigned char)*(SubFrameLen - breakBufferFillSize));
			pos += (SubFrameLen - breakBufferFillSize);
			breakBufferFillSize = 0;
		} else
		{
		        memcpy(injectBufferDataPointer, &buffer[pos], sizeof(unsigned char)*SubFrameLen);
			pos += SubFrameLen;
		}

		//write the PES header
		memcpy(injectBuffer, lpcm_pes, sizeof(lpcm_pes));

		//write the private data area
		memcpy(&injectBuffer[sizeof(lpcm_pes)], lpcm_prv, sizeof(lpcm_prv));

		//write the PCM data
		if(privateData->uBitsPerSample == 16) {
			for(n=0; n<SubFrameLen; n+=2) {
				unsigned char tmp;
				tmp=injectBufferDataPointer[n];
				injectBufferDataPointer[n]=injectBufferDataPointer[n+1];
				injectBufferDataPointer[n+1]=tmp;
			}
		} else {
		//A1cA1bA1a-B1cB1bB1a-A2cA2bA2a-B2cB2bB2a to A1aA1bB1aB1b.A2aA2bB2aB2b-A1cB1cA2cB2c
			for(n=0; n<SubFrameLen; n+=12) {
				unsigned char tmp[12];
				tmp[ 0]=injectBufferDataPointer[n+2];
				tmp[ 1]=injectBufferDataPointer[n+1];
				tmp[ 8]=injectBufferDataPointer[n+0];
				tmp[ 2]=injectBufferDataPointer[n+5];
				tmp[ 3]=injectBufferDataPointer[n+4];
				tmp[ 9]=injectBufferDataPointer[n+3];
				tmp[ 4]=injectBufferDataPointer[n+8];
				tmp[ 5]=injectBufferDataPointer[n+7];
				tmp[10]=injectBufferDataPointer[n+6];
				tmp[ 7]=injectBufferDataPointer[n+11];
				tmp[ 8]=injectBufferDataPointer[n+10];
				tmp[11]=injectBufferDataPointer[n+9];
				memcpy(&injectBufferDataPointer[n],tmp,12);
			}
		}

		//increment err... subframe count?
		lpcm_prv[1] = ((lpcm_prv[1]+SubFramesPerPES) & 0x1F);

	        //disable PES to save calculating correct values
	        lpcm_pes[7] = 0x01;

	        //kill off first A_PKT only fields in PES header
	        lpcm_pes[14] = 0xFF;
	        lpcm_pes[15] = 0xFF;
	        lpcm_pes[16] = 0xFF;


		write(privateData->m_fd, injectBuffer, injectBufferSize);
	}
	free(injectBuffer);

	return size;
}

int cAudio::StopClip()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);

	breakBufferFillSize = 0;
	
	Stop();

	if (ioctl(privateData->m_fd, AUDIO_SELECT_SOURCE, AUDIO_SOURCE_DEMUX) < 0)
		printf("AUDIO_SELECT_SOURCE failed(%m)");

	return 0;
}

void cAudio::getAudioInfo(int &type, int &layer, int& freq, int &bitrate, int &mode)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
#ifdef AZBOX_GEN_1	// <--- FIXME joseba
	type = 0;
	layer = 0;
	freq = 0;
	bitrate = 0;
	mode = 0;
#endif
}

void cAudio::SetSRS(int iq_enable, int nmgr_enable, int iq_mode, int iq_level)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
//DolbyDigital on/off
}

bool cAudio::IsHdmiDDSupported()
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
	
	return true;
}

void cAudio::SetHdmiDD(bool enable)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);


	int fd = open("/proc/stb/hdmi/audio_source", O_RDWR);
	if(enable) {
		// do i have to enable bypass for that ?
		// enableBypass();

		write(fd, "spdif", strlen("spdif"));
	}
	else
		write(fd, "pcm", strlen("pcm"));
	close(fd);


}

void cAudio::SetSpdifDD(int dd, int dts, int aac)
{
	printf("%s:%s\n dd %i, dts %i, aac %i", FILENAME, __FUNCTION__, dd, dts, aac);

	const char* sEnable[] =
	{
		"0",
		"1"
	};

	int fd = open("/proc/codecs/ac3", O_RDWR);
	write(fd, sEnable[dd], 1);
	close(fd);
	
	fd = open("/proc/codecs/dts", O_RDWR);
	write(fd, sEnable[dts], 1);
	close(fd);
	
	fd = open("/proc/codecs/aac", O_RDWR);
	write(fd, sEnable[aac], 1);
	close(fd);
	
#if 0
	if(enable)
		enableBypass();
	else
		disableBypass();
#endif
}

void cAudio::ScheduleMute(bool On)
{
	printf("%s:%s\n", FILENAME, __FUNCTION__);
}

