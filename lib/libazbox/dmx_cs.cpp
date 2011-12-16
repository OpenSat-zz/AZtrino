#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>

#include "dmx_cs.h"
#include <linux/dvb/dmx.h>

static const char * FILENAME = "dmx_cs.cpp";

//EVIL

cDemux *videoDemux = NULL;
cDemux *audioDemux = NULL;
cDemux *pcrDemux = NULL;

//EVIL END

static const char * aDMXCHANNELTYPE[] = {
	"",
   "DMX_VIDEO_CHANNEL",
   "DMX_AUDIO_CHANNEL",
   "DMX_PES_CHANNEL",
   "DMX_PSI_CHANNEL",
   "DMX_PIP_CHANNEL",
   "DMX_TP_CHANNEL",
   "DMX_PCR_ONLY_CHANNEL",
	};

bool cDemux::Open(DMX_CHANNEL_TYPE pes_type, void * hVideoBuffer , int uBufferSize)
{
	printf("%s:%s - pes_type=%s hVideoBuffer= uBufferSize=%d\n", FILENAME, __FUNCTION__, 
		aDMXCHANNELTYPE[pes_type], uBufferSize);
	type = pes_type;

	char filename[128];
	sprintf(filename, "/dev/dvb/adapter%d/demux%d", privateData->adapter, privateData->demux);
	privateData->m_fd_demux = open(filename, O_RDWR);

	if(privateData->m_fd_demux <= 0)
	{
		printf("%s:%s - open failed\n", FILENAME, __FUNCTION__);
		return false;
	}
	
	int n = DMX_SOURCE_FRONT0;
	if (ioctl(privateData->m_fd_demux, DMX_SET_SOURCE, &n) < 0)
		printf("DMX_SET_SOURCE failed(%m)");

	if (ioctl(privateData->m_fd_demux, DMX_SET_BUFFER_SIZE, uBufferSize/*Dagobert???*8*/) < 0)
		printf("DMX_SET_BUFFER_SIZE failed(%m)");
	
	return true;
}

void cDemux::Close(void)
{
	printf("%s:%s (type=%s) Pid %d\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type], pid);
	
	close(privateData->m_fd_demux);
}

bool cDemux::Start(void)
{
	printf("%s:%s (type=%s) Pid %d\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type], pid);
		
#ifdef do_not_start_immediate
/*
Dagobert/Donald: We must start the pesFilter with flag
immediate, otherwise the player crashes on zapping.
Since we dont find this "bug" we ignore the "manual"
start of the filter, because starting two times the
same filter leads to stop_feed call in the dvbapi
before it will be restarted. And this leads to a
stopping of Video in the player ;)
*/
        if (ioctl(privateData->m_fd_demux , DMX_START) < 0)
        {
                printf("failed (%m)");
                return false;
        }		
#endif
        printf("ok\n");
	return 0;
}

bool cDemux::Stop(void)
{
	printf("%s:%s (type=%s) Pid %d\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type], pid);
	
	ioctl(privateData->m_fd_demux, DMX_STOP);
	
	return true;
}

int cDemux::Read(unsigned char *buff, int len, int Timeout)
{
//AS THIS SEEMS TO BE WORKING, DEACTIVATING DEBUG

	/*printf("%s:%s (type=%s) - buff= len=%d, Timeout=%d\n", FILENAME, __FUNCTION__, 
		aDMXCHANNELTYPE[type], len, Timeout);*/
	
	int vSelectRtv = 1;
	
	if(Timeout > 0) {
		struct timeval timeout;
		/* Initialize the timeout data structure. */
		timeout.tv_sec = 0;
		timeout.tv_usec = Timeout * 1000;
		
		fd_set     rfds;
		FD_ZERO(&rfds);
		FD_SET(privateData->m_fd_demux, &rfds);
		
		/* select returns 0 if timeout, 1 if input available, -1 if error. */
		vSelectRtv = select (privateData->m_fd_demux + 1,
             &rfds , NULL, NULL, &timeout);
	}
	if(vSelectRtv == 1) {
		int ret = read(privateData->m_fd_demux, buff, len);
		
		/*printf("%s:%s (type=%s) - buff= len=%d, Timeout=%d ->> ret: %d\n", FILENAME, __FUNCTION__, 
		aDMXCHANNELTYPE[type], len, Timeout, ret);*/
		
		return ret;
	}
	
	/*printf("%s:%s (type=%s) - timeout!\n", FILENAME, __FUNCTION__, 
		aDMXCHANNELTYPE[type]);*/
		
	return -1;
}

void cDemux::SignalRead(int len)
{
	printf("%s:%s (type=%s) - len=%d\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type], len);
}

bool cDemux::sectionFilter(unsigned short Pid, const unsigned char * const Tid, 
	const unsigned char * const Mask, int len, int Timeout, const unsigned char * const nMask )
{
	printf("%s:%s (type=%s) - Pid=%d Tid= Mask= len=%d Timeout=%d nMask=\n", FILENAME, __FUNCTION__,
		aDMXCHANNELTYPE[type], Pid, len, Timeout);
	

	/*if(Tid != NULL) {
		printf("Tid {\n");
		for(int i = 0; i < DMX_FILTER_SIZE; i++)
			printf("\t%d (%02x)\n", Tid[i], Tid[i]);
		printf("}\n");
	}
	
	if(Mask != NULL) {
		printf("Mask {\n");
		for(int i = 0; i < DMX_FILTER_SIZE; i++)
			printf("\t%d (%02x)\n", Mask[i], Mask[i]);
		printf("}\n");
	}
	
	if(nMask != NULL) {
		printf("nMask {\n");
		for(int i = 0; i < DMX_FILTER_SIZE; i++)
			printf("\t%d (%02x)\n", nMask[i], nMask[i]);
		printf("}\n");
	}*/
	
	pid = Pid;
	
	dmx_sct_filter_params sct;
	memset(&sct, 0, sizeof(dmx_sct_filter_params));
	sct.pid     = pid;
	sct.timeout = Timeout;
	sct.flags   = DMX_IMMEDIATE_START;

	//There seems to be something wrong, the given array is not 0 initialisied
	//So just copy the first entry
	if(Tid)
		memcpy(sct.filter.filter, Tid/*mask.data*/, /*DMX_FILTER_SIZE*/len * sizeof(unsigned char));
	if(Mask)
		memcpy(sct.filter.mask, Mask/*mask.mask*/, /*DMX_FILTER_SIZE*/len * sizeof(unsigned char));
	if(nMask)
		memcpy(sct.filter.mode, nMask/*mask.mode*/, /*DMX_FILTER_SIZE*/len * sizeof(unsigned char));

	if (ioctl(privateData->m_fd_demux, DMX_SET_FILTER, &sct) < 0)
	{
		printf("failed (%m)\n");
		return false;
	}
	printf("ok\n");

	return true;
}

bool cDemux::pesFilter(const unsigned short Pid)
{
	printf("%s:%s (type=%s) - Pid=%d\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type], Pid);
	
	if(privateData->m_fd_demux <= 0)
		return false;

	struct dmx_pes_filter_params pes;
	memset(&pes, 0, sizeof(struct dmx_pes_filter_params));

	pid = Pid;

	pes.pid      = Pid;
	pes.input    = DMX_IN_FRONTEND;
	pes.output   = DMX_OUT_DECODER;
	pes.flags    = DMX_IMMEDIATE_START;	

	switch(type) {
		case DMX_VIDEO_CHANNEL:
			pes.pes_type = privateData->demux ? DMX_PES_VIDEO1 : DMX_PES_VIDEO0; /* FIXME */
			break;
		case DMX_AUDIO_CHANNEL:
			pes.pes_type = privateData->demux ? DMX_PES_AUDIO1 : DMX_PES_AUDIO0; /* FIXME */
			break;
		case DMX_PCR_ONLY_CHANNEL:
			pes.pes_type = privateData->demux ? DMX_PES_PCR1 : DMX_PES_PCR0; /* FIXME */
			break;
		case DMX_PES_CHANNEL:
			pes.output   = DMX_OUT_TAP;
			pes.pes_type = DMX_PES_OTHER;
			break;
		case DMX_TP_CHANNEL:
			pes.output   = DMX_OUT_TS_TAP;
			pes.pes_type = DMX_PES_OTHER;
			break;
		default:
			printf("[%s] %s # HELP # unknown pesFilter type %s\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
			return false;
	}

	if (ioctl(privateData->m_fd_demux, DMX_SET_PES_FILTER, &pes) < 0)
	{
		printf("%s:%s failed\n", FILENAME, __FUNCTION__);
		return false;
	}

	printf("%s:%s ok\n", FILENAME, __FUNCTION__);

	return true;
}

void cDemux::SetSyncMode(AVSYNC_TYPE mode)
{
	printf("%s:%s (type=%s) - mode=\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
	
	SyncMode = mode;
}

void * cDemux::getBuffer()
{
	printf("%s:%s (type=%s)\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
	
	return NULL;
}

void * cDemux::getChannel()
{
	printf("%s:%s (type=%s)\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
	
	return NULL;
}

const DMX_CHANNEL_TYPE cDemux::getChannelType(void)
{
	printf("%s:%s (type=%s)\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
	
	return type; 
}

void cDemux::addPid(unsigned short Pid)
{
	printf("%s:%s (type=%s) - Pid=%d\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type], Pid);
	
	/*
	DEMUX_ADD_PID impelemtation ?
	not supported - only used in driver/streamts.cpp
	*/
	
	//pid = Pid;
}

void cDemux::getSTC(int64_t * STC)
{
	printf("%s:%s (type=%s) STC=\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
	
	struct dmx_stc stc;
	memset(&stc, 0, sizeof(dmx_stc));
	stc.num = 0/*num*/;
	stc.base = 1;
	ioctl(privateData->m_fd_demux, DMX_GET_STC, &stc);
	*STC = (int64_t)stc.stc;
}

//
cDemux::cDemux(int num)
{
	printf("%s:%s - num=%d\n", FILENAME, __FUNCTION__, num);
	
	SyncMode = AVSYNC_DISABLED;
	
	privateData = (CS_DMX_PDATA*)malloc(sizeof(CS_DMX_PDATA));
	
	// This is a good idea but does unfortunatly not work with our drivers
	//privateData->demux = num;
	privateData->demux = 0;
	privateData->adapter = 0;
}

cDemux::~cDemux()
{
	printf("%s:%s (type=%s)\n", FILENAME, __FUNCTION__, aDMXCHANNELTYPE[type]);
	
	close(privateData->m_fd_demux);
	
	free(privateData);
}

