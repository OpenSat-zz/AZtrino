/*******************************************************************************/
/*                                                                             */
/* libazbox/cszapper/demux.h                                              */
/*   ZAP interface for neutrino frontend                                       */
/*                                                                             */
/* (C) 2008 CoolStream International                                           */
/*                                                                             */
/*******************************************************************************/
#ifndef __DEMUX_CS_H
#define __DEMUX_CS_H

#include <string>

#include <cstdlib>
#include <vector>
extern "C" {
#include <inttypes.h>
#include <sys/ioctl.h>

}
#define DEMUX_POLL_TIMEOUT 5000  // timeout in ms
#define MAX_FILTER_LENGTH 16    // maximum number of filters
#ifndef DMX_FILTER_SIZE
#define DMX_FILTER_SIZE MAX_FILTER_LENGTH
#endif
#define MAX_DMX_UNITS 4 //DMX_NUM_TSS_INPUTS_REVB

#ifndef CS_DMX_PDATA
typedef struct {
	int m_fd_demux;
	int demux;
	int adapter;
} CS_DMX_PDATA;
#endif

#include "cs_types.h"

typedef enum
{
	DMX_INVALID = 0,
	DMX_VIDEO_CHANNEL = 1,
	DMX_AUDIO_CHANNEL,
	DMX_PES_CHANNEL,
	DMX_PSI_CHANNEL,
	DMX_PIP_CHANNEL,
	DMX_TP_CHANNEL,
	DMX_PCR_ONLY_CHANNEL
} DMX_CHANNEL_TYPE;

typedef struct
{
	int fd;
	unsigned short pid;
} pes_pids;

class cDemux
{
	private:
		int timeout;
		unsigned short pid;
		unsigned char tid[MAX_FILTER_LENGTH], mask[MAX_FILTER_LENGTH];
		bool nb; // non block
		pthread_cond_t read_cond;
		pthread_mutex_t mutex;
		AVSYNC_TYPE SyncMode;
		int uLength;
		bool enabled;
		int unit;



		int num;
				int fd;
				int buffersize;
				bool measure;
				uint64_t last_measure, last_data;
				DMX_CHANNEL_TYPE dmx_type;
				std::vector<pes_pids> pesfds;


		DMX_CHANNEL_TYPE type;
		CS_DMX_PDATA * privateData;
	public:

		bool Open(DMX_CHANNEL_TYPE pes_type, void * hVideoBuffer = NULL, int uBufferSize = 8192);
		void Close(void);
		bool Start(void);
		bool Stop(void);
		int Read(unsigned char *buff, int len, int Timeout = 0);
		void SignalRead(int len);
		unsigned short GetPID(void) { return pid; }
		const unsigned char *GetFilterTID(void) { return tid; }
		const unsigned char *GetFilterMask(void) { return mask; }
		bool sectionFilter(unsigned short Pid, const unsigned char * const Tid, const unsigned char * const Mask, int len, int Timeout = DEMUX_POLL_TIMEOUT, const unsigned char * const nMask = NULL);
		bool pesFilter(const unsigned short Pid);
		void SetSyncMode(AVSYNC_TYPE mode);
		void * getBuffer();
		void * getChannel();
		const DMX_CHANNEL_TYPE getChannelType(void);
		void addPid(unsigned short Pid);
		void getSTC(int64_t * STC);
		//
		cDemux(int num = 0);
		~cDemux();
};

#endif //__DEMUX_H
