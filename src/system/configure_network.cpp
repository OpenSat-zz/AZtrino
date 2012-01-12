/*
 * $Header: /cvsroot/tuxbox/apps/tuxbox/neutrino/src/system/configure_network.cpp,v 1.6 2003/03/26 17:53:12 thegoodguy Exp $
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
#include <config.h>
#include <sys/wait.h>
#include "configure_network.h"
#include "libnet.h"             /* netGetNameserver, netSetNameserver   */
#include "network_interfaces.h" /* getInetAttributes, setInetAttributes */


#include <stdlib.h>             /* system                               */

#ifdef AZBOX_GEN_1
#include <stdio.h>
#endif

CNetworkConfig::CNetworkConfig(void)
{
	char our_nameserver[16];
	netGetNameserver(our_nameserver);
	nameserver = our_nameserver;
	inet_static = getInetAttributes("eth0", automatic_start, address, netmask, broadcast, gateway);
	copy_to_orig();
}

void CNetworkConfig::copy_to_orig(void)
{
	orig_automatic_start = automatic_start;
	orig_address         = address;
	orig_netmask         = netmask;
	orig_broadcast       = broadcast;
	orig_gateway         = gateway;
	orig_inet_static     = inet_static;
}

bool CNetworkConfig::modified_from_orig(void)
{
	return (
		(orig_automatic_start != automatic_start) ||
		(orig_address         != address        ) ||
		(orig_netmask         != netmask        ) ||
		(orig_broadcast       != broadcast      ) ||
		(orig_gateway         != gateway        ) ||
		(orig_inet_static     != inet_static    )
		);
}

void CNetworkConfig::commitConfig(void)
{
	if (modified_from_orig())
	{
		copy_to_orig();

		if (inet_static)
		{
			addLoopbackDevice("lo", true);
			setStaticAttributes("eth0", automatic_start, address, netmask, broadcast, gateway);
		}
		else
		{
			addLoopbackDevice("lo", true);
			setDhcpAttributes("eth0", automatic_start);
		}
	}
	if (nameserver != orig_nameserver)
	{
		orig_nameserver = nameserver;
		netSetNameserver(nameserver.c_str());
	}
}

int mysystem(char * cmd, char * arg1, char * arg2)
{
        int pid, i;
        switch (pid = fork())
        {
                case -1: /* can't fork */
                        perror("fork");
                        return -1;

                case 0: /* child process */
                        for(i = 3; i < 256; i++)
                                close(i);
                        if(execlp(cmd, cmd, arg1, arg2, NULL))
                        {
                                perror("exec");
                        }
                        exit(0);
                default: /* parent returns to calling process */
                        break;
        }
	waitpid(pid, 0, 0);
	return 0;
}

void CNetworkConfig::startNetwork(void)
{
	system("/sbin/ifup -v eth0");
	//mysystem((char *) "ifup",  (char *) "-v",  (char *) "eth0");
}

void CNetworkConfig::stopNetwork(void)
{
	//mysystem("ifdown eth0", NULL, NULL);
	system("/sbin/ifdown eth0");
}

void CNetworkConfig::startWirelessNetwork(char * iface)
{
	char cmd[100];
	sprintf(cmd,"/sbin/ifconfig %s up",iface);
	system(cmd);

	//mysystem((char *) "ifup",  (char *) "-v",  (char *) "eth0");
}

void CNetworkConfig::connectWirelessNetwork(char * iface,char * essid, int encrypt,char * password )
{
	char cmd[100];
	//iwc
	//iwconfig wlan0 essid "nombre_de_la_red"
	//iwconfig wlan0 key "la_clave_que_corresponde_a_la_red"
	//dhclient wlan0
	sprintf(cmd,"/sbin/iwconfig %s mode managed",iface);
	system(cmd);
	printf ("############## %s\n",cmd);
	if ((encrypt == CNetworkConfig::NONE) || (encrypt == CNetworkConfig::WEP))
	{
		sprintf(cmd,"/sbin/iwconfig %s essid %s",iface,essid);
		system(cmd);
		printf ("############## %s\n",cmd);

		if (encrypt == CNetworkConfig::WEP)
		{
			sprintf(cmd,"/sbin/iwconfig %s key %s",iface,password);
			system(cmd);
			printf ("############## %s\n",cmd);
		}
	}
	else if ((encrypt == CNetworkConfig::WPA) || (encrypt == CNetworkConfig::WPA2))
	{
		sprintf(cmd,"echo %s | wpa_passphrase %s  >  /etc/wpa_supplicant/wpa_supplicant.conf",password,essid);
		system(cmd);
		sprintf(cmd,"wpa_supplicant  -B -w -c /etc/wpa_supplicant/wpa_supplicant.conf -Dralink -i%s",iface);
		system(cmd);
	}


	sprintf(cmd,"/sbin/udhcpc -i  %s",iface);
	system(cmd);
	printf ("############## %s\n",cmd);

	//mysystem((char *) "ifup",  (char *) "-v",  (char *) "eth0");

}

 void CNetworkConfig::stopWirelessNetwork(void)
{
	//mysystem("ifdown eth0", NULL, NULL);
	system("/sbin/ifdown ra0");
}
