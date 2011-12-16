/* Dagobert: Most parts taken from old neutrino stream2file.cpp */

/*
 * $Id: stream2file.cpp,v 1.32 2009/04/02 07:53:53 seife Exp $
 * 
 * streaming to file/disc
 * 
 * Copyright (C) 2004 Axel Buehning <diemade@tuxbox.org>,
 *                    thegoodguy <thegoodguy@berlios.de>
 *
 * based on code which is
 * Copyright (C) 2001 TripleDES
 * Copyright (C) 2000, 2001 Marcus Metzler <marcus@convergence.de>
 * Copyright (C) 2002 Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 *
 */
#include <stdio.h>

#include "record_cs.h"

#include <driver/stream2file.h>

static const char * FILENAME = "record_cs.cpp";

static int sync_byte_offset(const unsigned char * buf, const unsigned int len) {

	unsigned int i;

	for (i = 0; i < len; i++)
		if (buf[i] == 0x47)
			return i;

	return -1;
}


static int setPesFilter(const unsigned short pid, const dmx_output_t dmx_output)
{
	int fd;
	struct dmx_pes_filter_params flt; 

printf("%s:%s >\n", __FILE__, __FUNCTION__);

	if ((fd = open(DMXDEV, O_RDWR|O_NONBLOCK)) < 0)
		return -1;
	if (ioctl(fd, DMX_SET_BUFFER_SIZE, DMX_BUFFER_SIZE) < 0)
		return -1;

	flt.pid = pid;
	flt.input = DMX_IN_FRONTEND;
	flt.output = dmx_output;
	flt.pes_type = DMX_PES_OTHER;
	flt.flags = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_PES_FILTER, &flt) < 0)
		return -1;

printf("%s:%s <\n", __FILE__, __FUNCTION__);
	return fd;
}


static void unsetPesFilter(const int fd)
{
	ioctl(fd, DMX_STOP);
	close(fd);
}


void cRecord::FileThread()
{
	ringbuffer_data_t vec[2];
	size_t readsize;

printf("%s:%s >\n", __FILE__, __FUNCTION__);

	ringbuffer_t * ringbuf = ringbuffer;
	while (1)
	{
		ringbuffer_get_read_vector(ringbuf, &(vec[0]));
		readsize = vec[0].len + vec[1].len;
		if (readsize)
		{
			ssize_t written;

			while (1)
			{
				if ((written = write(file_fd, vec[0].buf, vec[0].len)) < 0)
				{
					if (errno != EAGAIN)
					{
						exit_flag = STREAM2FILE_STATUS_WRITE_FAILURE;
						perror("[stream2file]: error in write");
						goto terminate_thread;
					}
				}
				else
				{
					ringbuffer_read_advance(ringbuf, written);
					
					if (vec[0].len == (size_t)written)
					{
						if (vec[1].len == 0)
						{
							goto all_bytes_written;
						}
						
						vec[0] = vec[1];
						vec[1].len = 0;
					}
					else
					{
						vec[0].len -= written;
						vec[0].buf += written;
					}
				}
			}

all_bytes_written:
;
//fixme: do we need this			if (use_fdatasync)
//				fdatasync(fd2);
			
		}
		else
		{
			if (exit_flag != STREAM2FILE_STATUS_RUNNING)
				goto terminate_thread;
			usleep(1000);
		}
	}
 terminate_thread:

printf("%s:%s <\n", __FILE__, __FUNCTION__);
	pthread_exit(NULL);
}

/* helper function to call the cpp thread loop */
void* execute_file_thread(void *c)
{
   cRecord *obj=(cRecord*)c;

printf("%s:%s >\n", __FILE__, __FUNCTION__);

   obj->FileThread();

printf("%s:%s <\n", __FILE__, __FUNCTION__);
   return NULL;
}

void cRecord::DMXThread()
{
	pthread_t         file_thread;
	
	ringbuffer_data_t vec[2];
	ssize_t           written;
	ssize_t           todo = 0;
	ssize_t           todo2;
	unsigned char        buf[TS_SIZE];
	int                offset = 0;
	ssize_t            r = 0;
	struct pollfd       pfd;
	int                pres;

printf("%s:%s >\n", __FILE__, __FUNCTION__);

        pfd.fd = dvrfd;
        pfd.events = POLLIN|POLLERR;
	pfd.revents = 0;

	ringbuffer_t * ringbuf = ringbuffer_create(ringbuffersize);

	if (!ringbuf)
	{
		exit_flag = STREAM2FILE_STATUS_WRITE_OPEN_FAILURE;
		printf("[stream2file]: error allocating ringbuffer! (out of memory?)\n"); 
	}
	else
		fprintf(stderr, "[stream2file] allocated ringbuffer size: %ld\n", ringbuffer_write_space(ringbuf));

	ringbuffer = ringbuf;

	if (pthread_create(&file_thread, 0, execute_file_thread, this) != 0)
	{
		exit_flag = STREAM2FILE_STATUS_WRITE_OPEN_FAILURE;
		printf("[stream2file]: error creating file_thread! (out of memory?)\n"); 
	}

	while (exit_flag == STREAM2FILE_STATUS_RUNNING)
	{
		if ((pres=poll (&pfd, 1, 15000))>0)
		{
			if (!(pfd.revents&POLLIN))
			{
				printf ("[stream2file]: PANIC: error reading from demux, bailing out\n");
				exit_flag = STREAM2FILE_STATUS_READ_FAILURE;
			}
			r = read(dvrfd, &(buf[0]), TS_SIZE);
			if (r > 0)
			{
				offset = sync_byte_offset(&(buf[0]), r);
				if (offset != -1)
					break;
			}
		}
		else if (!pres)
		{
			printf ("[stream2file]: timeout from demux\n");
		}
	}

	if (exit_flag == STREAM2FILE_STATUS_RUNNING)
	{
		written = ringbuffer_write(ringbuf, (char *)&(buf[offset]), r - offset);
		// TODO: Retry
		if (written != r - offset) {
			printf("PANIC: wrote less than requested to ringbuffer, written %d, requested %d\n", written, r - offset);
			exit_flag = STREAM2FILE_STATUS_BUFFER_OVERFLOW;
		}
		todo = IN_SIZE - (r - offset);
	}

	/* IN_SIZE > TS_SIZE => todo > 0 */

	while (exit_flag == STREAM2FILE_STATUS_RUNNING)
	{
		ringbuffer_get_write_vector(ringbuf, &(vec[0]));
		todo2 = todo - vec[0].len;
		if (todo2 < 0)
		{
			todo2 = 0;
		}
		else
		{
			if (((size_t)todo2) > vec[1].len)
			{
				printf("PANIC: not enough space in ringbuffer, available %d, needed %d\n", vec[0].len + vec[1].len, todo + todo2);
				exit_flag = STREAM2FILE_STATUS_BUFFER_OVERFLOW;
			}
			todo = vec[0].len;
		}

		while (exit_flag == STREAM2FILE_STATUS_RUNNING)
		{
			if ((pres=poll (&pfd, 1, 5000))>0)
			{
				if (!(pfd.revents&POLLIN))
				{
					printf ("PANIC: error reading from demux, bailing out\n");
					exit_flag = STREAM2FILE_STATUS_READ_FAILURE;
				}
				r = read(dvrfd, vec[0].buf, todo);
				if (r > 0)
				{
					ringbuffer_write_advance(ringbuf, r);
	
					if (todo == r)
					{
						if (todo2 == 0)
							goto next;
	
						todo = todo2;
						todo2 = 0;
						vec[0].buf = vec[1].buf;
					}
					else
					{
						vec[0].buf += r;
						todo -= r;
					}
				}
			}
			else if (!pres){
				printf ("[stream2file]: timeout reading from demux\n");
				exit_flag = STREAM2FILE_STATUS_READ_FAILURE;
			}
		}
		next:
			todo = IN_SIZE;
	}

	close(dvrfd);

	pthread_join(file_thread, NULL);

	if (ringbuf)
		ringbuffer_free(ringbuf);

	while (demuxfd_count > 0)
		unsetPesFilter(demuxfd[--demuxfd_count]);

#ifdef needed
fixme: do we need it?
	CEventServer eventServer;
	eventServer.registerEvent2(NeutrinoMessages::EVT_RECORDING_ENDED, CEventServer::INITID_NEUTRINO, "/tmp/neutrino.sock");
	stream2file_status2_t s;
	s.status = exit_flag;
	strncpy(s.filename,basename(myfilename),512);
	s.filename[511] = '\0';
	strncpy(s.dir,dirname(myfilename),100);
	s.dir[99] = '\0';
	eventServer.sendEvent(NeutrinoMessages::EVT_RECORDING_ENDED, CEventServer::INITID_NEUTRINO, &s, sizeof(s));
	printf("[stream2file]: pthreads exit code: %i, dir: '%s', filename: '%s' myfilename: '%s'\n", exit_flag, s.dir, s.filename, myfilename);
#endif


printf("%s:%s <\n", __FILE__, __FUNCTION__);
	pthread_exit(NULL);
}


cRecord::cRecord(int num) 
{ 
   printf("%s:%s num = %d\n", FILENAME, __FUNCTION__, num); 
//fixme: what is the meaning of num? should it be the demuxer number?   
}

cRecord::~cRecord() 
{ 
   printf("%s:%s\n", FILENAME, __FUNCTION__); 
}

bool cRecord::Open(int numpids) 
{ 
   printf("%s:%s numpids %d\n", FILENAME, __FUNCTION__, numpids); 

   ringbuffers = 4;
   ringbuffersize = ((1 << 19) << ringbuffers);

   demuxfd_count = numpids;
   
   exit_flag = STREAM2FILE_STATUS_IDLE;

   return true; 
}

void cRecord::Close(void) 
{ 
     printf("%s:%s\n", FILENAME, __FUNCTION__); 
}

/* helper function to call the cpp thread loop */
void* execute_demux_thread(void *c)
{
   cRecord *obj=(cRecord*)c;

printf("%s:%s>\n", FILENAME, __FUNCTION__); 

   obj->DMXThread();

printf("%s:%s<\n", FILENAME, __FUNCTION__); 

   return NULL;
}

bool cRecord::Start(int fd, unsigned short vpid, unsigned short * apids, int numpids) 
{ 
    printf("%s:%s: fd %d, vpid 0x%02x\n", FILENAME, __FUNCTION__, fd, vpid); 
    
    printf("apids: "); 
    for (int i = 0; i < numpids; i++)
       printf("0x%02x ", apids[i]);
    printf("\n");

    file_fd = fd;

    demuxfd_count = 1 + numpids;

//fixme: currently we only deal which what is called write_ts in earlier versions
//not splitting is possible, because we dont have the filename here...

    for (unsigned int i = 0; i < demuxfd_count; i++)
    {
	    unsigned short pid;

	    if (i == 0)
	       pid = vpid;
	    else
	       pid = apids[i-1];

	    if ((demuxfd[i] = setPesFilter(pid, DMX_OUT_TS_TAP)) < 0)
	    {
		    for (unsigned int j = 0; j < i; j++)
			    unsetPesFilter(demuxfd[j]);

                    printf("error setting pes filter\n");
		    return false;
	    }
    }

    if ((dvrfd = open(DVRDEV, O_RDONLY|O_NONBLOCK)) < 0)
    {
	    while (demuxfd_count > 0)
		    unsetPesFilter(demuxfd[--demuxfd_count]);

            printf("error opening dvr device\n");
	    return false;
    }
    
    printf("dvrfd %d\n", dvrfd);
    
    exit_flag = STREAM2FILE_STATUS_RUNNING;

    if (pthread_create(&demux_thread[0], 0, execute_demux_thread, this) != 0)
    {
	    exit_flag = STREAM2FILE_STATUS_WRITE_OPEN_FAILURE; 
	    printf("[stream2file]: error creating thread! (out of memory?)\n");
	    return false; 
    }
    
    time(&record_start_time);

    printf("record start time: %lu \n", record_start_time);
    return true; 
}

bool cRecord::Stop(void) 
{ 
    printf("%s:%s\n", FILENAME, __FUNCTION__); 

    if (exit_flag == STREAM2FILE_STATUS_RUNNING)
    {

	    time(&record_end_time);
	    printf("record time: %lu \n",record_end_time-record_start_time);

	    exit_flag = STREAM2FILE_STATUS_IDLE;
	    return true;
    }

    return false; 
}

void cRecord::RecordNotify(int Event, void *pData) 
{ 
    printf("%s:%s event %d\n", FILENAME, __FUNCTION__, Event); 

//fixme: dont know what this is for? maybe we must connect to the event queues? not sure...
}

