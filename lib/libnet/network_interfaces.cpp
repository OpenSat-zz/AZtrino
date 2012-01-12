/*
 * $Header: /cvs/tuxbox/apps/misc/libs/libnet/network_interfaces.cpp,v 1.6 2003/03/20 15:32:52 thegoodguy Exp $
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <fstream>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <string.h>
#include <dirent.h>

#include <iwlib.h>





bool read_file(const std::string filename, std::list<std::string> &line)
{
	std::string   s;
	std::ifstream in(filename.c_str());

	line.clear();

	if (!in.is_open())
		return false;

	while (getline(in, s))
		line.push_back(s);

	return true;
}

bool write_file(const std::string filename, const std::list<std::string> line)
{
	std::ofstream out(filename.c_str());

	if (!out.is_open())
		return false;

	for (std::list<std::string>::const_iterator it = line.begin(); it != line.end(); it++)
		out << (*it) << std::endl;

	return true;
}

std::list<std::string>::iterator add_attributes(const std::map<std::string, std::string> attribute, std::list<std::string> &line, std::list<std::string>::iterator here)
{
	for (std::map<std::string, std::string>::const_iterator it = attribute.begin(); it != attribute.end(); it++)
	{
		std::ostringstream out;
		out << '\t' << (it -> first) << ' ' << (it -> second);
		line.insert(here, out.str());
	}
	return here;
}

std::string remove_interface_from_line(const std::string interface, const std::string line)
{
	std::string        s;
	std::istringstream in(line.c_str());
	std::ostringstream out;
	bool               contains_at_least_one_interface = false;

	if (in >> s)
	{
		out << s;  /* auto */
		
		while (in >> s)
		{
			if (s != interface)
			{
				out << ' ' << s;
				contains_at_least_one_interface = true;
			}
		}
	}

	return (contains_at_least_one_interface ? out.str() : "");
}

bool write_interface(const std::string filename, const std::string name, const bool automatic_start, const std::string family, const std::string method, const std::map<std::string, std::string> attribute)
{
	std::string            s;
	std::list<std::string> line;
	bool                   found = false;

	read_file(filename, line); /* ignore return value */

	for (std::list<std::string>::iterator it = line.begin(); it != line.end(); )
	{
		{
			std::istringstream in((*it).c_str());
			
			if (!(in >> s))
			{
				it++;
				continue;
			}
			
			if (s != std::string("iface"))
			{
				if (s == std::string("auto"))
				{
					bool advance = true;
					while (in >> s)
					{
						if (s == std::string(name))
						{
							*it = remove_interface_from_line(name, *it);
							if ((*it).empty())
							{
								it = line.erase(it); /* erase advances it */
								advance = false;
							}
							break;
						}
					}
					if (advance)
						it++;
				}
				else
					it++;
				continue;
			}

			if (!(in >> s))
			{
				it++;
				continue;
			}
			
			if (s != std::string(name))
			{
				it++;
				continue;
			}
		}
			
		found = true;

		/* replace line */
		std::ostringstream out;
		out << "iface " << name << ' ' << family << ' ' << method;
		(*it) = out.str();

		if (automatic_start)
			line.insert(it, "auto " + name);
		
		/* add attributes */
		it++;
		it = add_attributes(attribute, line, it);

		/* remove current attributes */
		while (it != line.end())
		{
			std::istringstream in((*it).c_str());

			if (!(in >> s))             /* retain empty lines */	
			{
				it++;
				continue;
			}
		
			if (s[0] == '#')            /* retain comments */
			{
				it++;
				continue;
			}
			
			if (s == std::string("iface"))
				break;
			
			if (s == std::string("auto"))
				break;
			
			if (s == std::string("mapping"))
				break;
			
			it = line.erase(it);
		}
	}

	if (!found)
	{
		std::ostringstream out;

		if (automatic_start)
			line.push_back("auto " + name);

		out << "iface " << name << ' ' << family << ' ' << method;
		line.push_back(out.str());
		add_attributes(attribute, line, line.end());
	}

	return write_file(filename, line);
}

bool read_interface(const std::string filename, const std::string name, bool &automatic_start, std::string &family, std::string &method, std::map<std::string, std::string> &attribute)
{
	std::string   s;
	std::string   t;
	std::ifstream in(filename.c_str());
	bool          advance = true;

	automatic_start = false;
	attribute.clear();

	if (!in.is_open())
		return false;

	while (true)
	{
		if (advance)
		{
			if (!getline(in, s))
				break;
		}
		else
			advance = true;

		{
			std::istringstream in(s.c_str());
			
			if (!(in >> s))
				continue;
			
			if (s != std::string("iface"))
			{
				if (s == std::string("auto"))
				{
					while (in >> s)
					{
						if (s == std::string(name))
						{
							automatic_start = true;
							break;
						}
					}
				}
				continue;
			}

			if (!(in >> s))
				continue;
			
			if (s != std::string(name))
				continue;
			
			if (!(in >> s))
				continue;
			
			if (!(in >> t))
				continue;
			
			family = s;
			method = t;
		}

		while (true)
		{
			if (!getline(in, s))
				return true;

			std::istringstream in(s.c_str());

			if (!(in >> t))             /* ignore empty lines */	
				continue;
		
			if (t[0] == '#')            /* ignore comments */
				continue;
			
			if (t == std::string("iface"))
				break;
			
			if (t == std::string("auto"))
				break;
			
			if (t == std::string("mapping"))
				break;
			
			if (!(in >> s))
				continue;
			
			attribute[t] = s;
		}
		advance = false;
	}

	return true;
}

bool getInetAttributes(const std::string name, bool &automatic_start, std::string &address, std::string &netmask, std::string &broadcast, std::string &gateway)
{
	std::string family;
	std::string method;
	std::map<std::string, std::string> attribute;

	if (!read_interface("/etc/network/interfaces", name, automatic_start, family, method, attribute))
		return false;

	if (family != "inet")
		return false;

	if (method != "static")
		return false;

	address   = "";
	netmask   = "";
	broadcast = "";
	gateway   = "";

	for (std::map<std::string, std::string>::const_iterator it = attribute.begin(); it != attribute.end(); it++)
	{
		if ((*it).first == "address")
			address = (*it).second;
		if ((*it).first == "netmask")
			netmask = (*it).second;
		if ((*it).first == "broadcast")
			broadcast = (*it).second;
		if ((*it).first == "gateway")
			gateway = (*it).second;
	}
	return true;
}

bool addLoopbackDevice(const std::string name, const bool automatic_start)
{
	std::map<std::string, std::string> attribute;

	return write_interface("/etc/network/interfaces", name, automatic_start, "inet", "loopback", attribute);
}

bool setStaticAttributes(const std::string name, const bool automatic_start, const std::string address, const std::string netmask, const std::string broadcast, const std::string gateway)
{
	std::map<std::string, std::string> attribute;

	attribute["address"] = address;
	attribute["netmask"] = netmask;

	if (!broadcast.empty())
		attribute["broadcast"] = broadcast;

	if (!gateway.empty())
		attribute["gateway"] = gateway;

	return write_interface("/etc/network/interfaces", name, automatic_start, "inet", "static", attribute);
}

bool setDhcpAttributes(const std::string name, const bool automatic_start)
{
	std::map<std::string, std::string> attribute;

	return write_interface("/etc/network/interfaces", name, automatic_start, "inet", "dhcp", attribute);
}




std::string print_scanning_token (struct iw_event * event)
{
	std::string result;
  /* Now, let's decode the event */
  switch (event->cmd)
    {
    case SIOCGIWESSID:
	char
	  essid[IW_ESSID_MAX_SIZE + 1];
	if ((event->u.essid.pointer) && (event->u.essid.length))
	  memcpy (essid, event->u.essid.pointer, event->u.essid.length);
	essid[event->u.essid.length] = '\0';
	if (event->u.essid.flags) {
	  result = essid;
	  }
      break;

    case SIOCGIWMODE:
      if (event->u.mode == 1) { result = "Ad-Hoc"; }
      else if (event->u.mode == 3) { result = "Managed"; }
      else result = "Unknown";
      break;

    case IWEVQUAL:
      {
		/*char buffer[128];
		WIFI_PRINT_STATS(buffer, event);
			result = buffer;
		*/
			break;

      }
    case SIOCGIWENCODE:
	if (event->u.data.flags & IW_ENCODE_DISABLED) {
	  result = "off" ;
	} else {
	  result =  "on" ;
	}
	break;
   default:
      break;

  }                           /* switch(event->cmd) */

  return result;
}


std::string*  getWirelessNetworks( char *iface)
{
	//print_scanning_info(0,dev,NULL,0);
	//iw_get_kernel_we_version();

	std::string networks[20];

	 /* add 1, then: number of networks found */
	    int netcount = -1;
	    int socket=0;
	    struct iwreq wrq;
	    unsigned char buffer[IW_SCAN_MAX_DATA];	/* Results */
	    struct timeval tv;				/* Select timeout */
	    int	timeout = 5000000;			/* 5s */
	    char *	interface_name = "ra0";

	    //networks== new char[31][20];
	    if(!socket)
	    	socket = iw_sockets_open();
	    	//return;
	    //if (interface_name.empty())
		//return NULL;

	    /* Init timeout value -> 250ms */
	    tv.tv_sec = 0;
	    tv.tv_usec = 250000;

	    /*
	     * Here we should look at the command line args and set the IW_SCAN_ flags
	     * properly
	     */
	    wrq.u.param.flags = IW_SCAN_DEFAULT;
	    wrq.u.param.value = 0;	/* Later */

	    /* Initiate Scanning */
	    wrq.u.data.pointer = NULL;
	    wrq.u.data.flags = 0;
	    wrq.u.data.length = 0;

	    if (iw_set_ext(socket, interface_name, SIOCSIWSCAN, &wrq) < 0)
	      {
			if (errno != EPERM)
			  {
				printf("Interface does not support scanning (errno= %d %s)\n", errno);
				return networks;
			  }
			/* If we don't have the permission to initiate the scan, we may
			 * still have permission to read left-over results.
			 * But, don't wait !!! */
			tv.tv_usec = 0;
	      }
	    timeout -= tv.tv_usec;



	    /* Forever */
	    while (1)
	      {
		fd_set rfds;		/* File descriptors for select */
		int last_fd;		/* Last fd */
		int ret;

		/* Guess what ? We must re-generate rfds each time */
		FD_ZERO (&rfds);
		last_fd = -1;

		/* In here, add the rtnetlink fd in the list */

		/* Wait until something happens */
		// ret = select (last_fd + 1, &rfds, NULL, NULL, &tv);
		ret = sleep(3);

		/* Check if there was an error */
		if (ret > 0)
		  {
		    if (errno == EAGAIN || errno == EINTR)
		      continue;
		   printf("Unhandled signal - aborting scan...\n");
		    return networks;
		  }

		/* Check if there was a timeout */
		if (ret == 0)
		  {
		    /* Try to read the results */
		    wrq.u.data.pointer = (char *) buffer;
		    wrq.u.data.flags = 0;
		    wrq.u.data.length = sizeof (buffer);
		    if (iw_get_ext	(socket, interface_name, SIOCGIWSCAN,
			 &wrq) < 0)
		      {
			/* Check if results not available yet */
			// if (errno == EAGAIN)
			  // {
			    /* Restart timer for only 100ms */
			    //tv.tv_sec = 0;
			    //tv.tv_usec = 100000;
			    //timeout -= tv.tv_usec;
			    //if (timeout > 0)
			    //  continue;	/* Try again later */
			  //}

			/* Bad error */
			printf("Failed to read scan data - aborting scan...\n" );
			return networks;
		      }
		    else
		      /* We have the results, go to process them */
		      break;
		  }

		/* In here, check if event and event type
		 * if scan event, read results. All errors bad & no reset timeout */
	      }

	    if (wrq.u.data.length)
	      {
		struct iw_event
		  iwe;
		struct stream_descr
		  stream;
		int
		  ret;
		int nnet=0;

		printf("Searching networks\n");

		iw_init_event_stream (&stream, (char *) buffer, wrq.u.data.length);
		do
		  {
		    /* Extract an event and print it */

		    ret = iw_extract_event_stream(&stream, &iwe, WIRELESS_EXT);

		    /* Currently, we only return the ESSIDs. We could
		       collect further data, but that's something for
		       maybe KDE 3.5 */

		    /* The first token is about ESSID. So if we see that, create a
		       new row. If it's something else, it belongs to the network
		       before. */

		    if (ret > 0) {
		      if (iwe.cmd == SIOCGIWESSID) {
			 netcount++;
			 //printf("Count networks: %d\n", netcount );
			 //networks[nnet]=netcount;
			 //nnet++;
			 std::string tempnet = print_scanning_token (&iwe);
			 if (!tempnet.empty())
				{
				 //networks->setText( netcount, 0, tempnet );

				 networks[nnet]=tempnet;
				 nnet++;

				 //printf("NETWORK: %s\n", tempnet.c_str());
				}
				else
				{
					//networks->setText( netcount, 0, "(hidden cell)");
					printf("(hidden cell)\n");
				}
		      }
		      if (iwe.cmd == SIOCGIWMODE) {
		    	     std::string tempresult= print_scanning_token (&iwe);
	                 //networks->setText( netcount, 1, tempresult );
	                // printf("RESULT: %s\n", tempresult.c_str());

		      }
	              if (iwe.cmd == IWEVQUAL) {
	            	 std::string tempresult= print_scanning_token (&iwe);
			 // crop the result... it's a little too verbose
			 // find everything between the first : and the first ' '
			 //tempresult = tempresult.mid(tempresult.find(':') + 1, tempresult.find( ' ', tempresult.find(':') ) - tempresult.find(':') );
	                // networks->setText( netcount, 2, tempresult );
	                //printf("RESULT2: %s\n", tempresult.c_str());
		      }
		      if (iwe.cmd == SIOCGIWENCODE) {
			   std::string tempresult = print_scanning_token (&iwe);
			   //networks->setText( netcount, 3, tempresult );
			   printf("RESULT3: %s\n", tempresult.c_str());
		      }
		    }

		  }
		while (ret > 0);
	      }

	    //for (int i = 0; i<4; i++ ) networks->adjustColumn(i);

	    return networks;

}

char*	getWirelessInterface(void)
{
	//list<string> interfaceNames;
	DIR* sysDir = opendir("/sys/class/net");
	struct dirent* sysDirEntry = 0;
	char iface[6]="null";

	if(!sysDir)
	    return "null";

	while((sysDirEntry = readdir(sysDir)))
	    {
	        std::string interfaceName(sysDirEntry->d_name);
	        char ifacepath[35]="/sys/class/net/";

	         if(interfaceName[0] == '.')
	             continue;

	         //interfaceNames.push_back(interfaceName);
	         printf("Found %s \n", interfaceName.c_str());
	         sprintf(ifacepath,"/sys/class/net/%s/wireless/",interfaceName.c_str());
	         if(chdir(ifacepath) == 0)
	         {
	        	 sprintf(iface,interfaceName.c_str());
	         	 return iface;
	         }
	    }

	closedir(sysDir);
	return iface;

}
