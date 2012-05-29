/*******************************************************************************/
/*                                                                             */
/* libazbox/cszapper/demux.h                                              */
/*   ZAP interface for neutrino frontend                                       */
/*                                                                             */
/* (C) 2008 CoolStream International                                           */
/*                                                                             */
/*******************************************************************************/
#ifndef __RECORD_CS_H
#define __RECORD_CS_H

#include <string>

#include <libeventserver/eventserver.h>
#include <neutrinoMessages.h>

#include <driver/stream2file.h>


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/dvb/dmx.h>

#include <pthread.h>
#include <signal.h>
#include <libgen.h>

extern "C" {
#include <driver/ringbuffer.h>
#include <driver/genpsi.h>
}

/* conversion buffer sizes */
#define TS_SIZE         188
#define IN_SIZE		(TS_SIZE * 362)

/* demux buffer size */
#define DMX_BUFFER_SIZE (256 * 1024)

/* maximum number of pes pids */
#define MAXPIDS		64

/* devices */
//#define DMXDEV	"/dev/dvb/adapter0/demux0"
//#define DVRDEV	"/dev/dvb/adapter0/dvr0"

class cRecord
{
	private:
		int num_apids;

		int ringbuffers;
                unsigned int ringbuffersize;
                time_t record_start_time;
                time_t record_end_time;

                pthread_t demux_thread[MAXPIDS];


		unsigned char demuxfd_count;
                int demuxfd[MAXPIDS];
                int dvrfd;

		int file_fd;

		ringbuffer_t * ringbuffer;

                stream2file_status_t exit_flag;

	public:
		cRecord(int num = 0);
		~cRecord();

		bool Open(int numpids);
		void Close(void);
		bool Start(int fd, unsigned short vpid, unsigned short * apids, int numpids);
		bool Stop(void);
		void RecordNotify(int Event, void *pData);

	        void DMXThread();
	        void FileThread();

};

#endif
