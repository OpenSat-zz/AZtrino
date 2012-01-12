/*
	Neutrino-GUI  -   DBoxII-Project

	WIRELESSMount/Umount GUI by Zwen

	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gui/wireless.h>

#include <gui/filebrowser.h>
#include <gui/widget/menue.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>

#include <fstream>

#include <global.h>

#include <errno.h>
#include <pthread.h>
#include <sys/mount.h>
#include <unistd.h>

#include "network_interfaces.h"             /* getInetAttributes, setInetAttributes */

#include <zapit/client/zapittools.h>


const char * wireless_entry_printf_string[3] =
{
	"NONE %s:%s -> %s auto: %4s",
	"WEP //%s/%s -> %s auto: %4s",
	"WPA2 %s/%s -> %s auto: %4s"
};
std::string *networks;
int nNetworkFound=0;

int CWIRELESSSearchGui::exec( CMenuTarget* parent, const std::string & actionKey )
{
	//printf("exec: %s\n", actionKey.c_str());
	int returnval = menu_return::RETURN_REPAINT;


	if (actionKey.empty())
	{
		printf("actionKey == empty\n");
		parent->hide();
		for(int i=0 ; i < nNetworkFound; i++)
		{
			/*sprintf(m_entry[i],
				wireless_entry_printf_string[(g_settings.network_wireless_encrypt[i] == (int) CNetworkConfig::NONE) ? 0 : ((g_settings.network_wireless_encrypt[i] == (int) CNetworkConfig::WEP) ? 1 : ((g_settings.network_wireless_encrypt[i] == (int) CNetworkConfig::WPA) ? 2 : 3))],
				g_settings.network_nfs_ip[i].c_str(),
				FILESYSTEM_ENCODING_TO_UTF8(g_settings.network_wireless_essid[i]),
				FILESYSTEM_ENCODING_TO_UTF8(g_settings.network_wireless_bssid[i]),
				g_Locale->getText(g_settings.network_nfs_automount[i] ? LOCALE_MESSAGEBOX_YES : LOCALE_MESSAGEBOX_NO));
				*/
			//sprintf(m_entry[i], networks[i].c_str()	);
			printf("####EMPTY###### %s\n",m_entry[i]);
			sprintf(m_entry[i], g_settings.network_wireless_essid[i]);
			printf("####EMPTY###### %s\n",m_entry[i]);

		}
		returnval = menu();
	}
	else if(actionKey.substr(0,8)=="netentry")
	{
		printf("actionKey == netentry\n");
		parent->hide();
		returnval = menuEntry(actionKey[8]-'0');
		for(int i=0 ; i < nNetworkFound; i++)
		{
		/*	sprintf(m_entry[i],
				wireless_entry_printf_string[(g_settings.network_wireless_encrypt[i] == (int) CNetworkConfig::NONE) ? 0 : ((g_settings.network_wireless_encrypt[i] == (int) CNetworkConfig::WEP) ? 1 : ((g_settings.network_wireless_encrypt[i] == (int) CNetworkConfig::WPA) ? 2 : 3))],
				g_settings.network_nfs_ip[i].c_str(),
				FILESYSTEM_ENCODING_TO_UTF8(g_settings.network_wireless_essid[i]),
				FILESYSTEM_ENCODING_TO_UTF8(g_settings.network_wireless_bssid[i]),
				g_Locale->getText(g_settings.network_nfs_automount[i] ? LOCALE_MESSAGEBOX_YES : LOCALE_MESSAGEBOX_NO));
				*/

			printf("####netentry###### %s\n",m_entry[i]);
			sprintf(m_entry[i], g_settings.network_wireless_essid[i]);
			printf("####netentry###### %s\n",m_entry[i]);
			//sprintf(ISO_8859_1_entry[i],ZapitTools::UTF8_to_Latin1(m_entry[i]).c_str());
		}
	}
	else if(actionKey.substr(0,10)=="connectnow")
	{
		//Push menu "CONNECT NOW"
		int nr=atoi(actionKey.substr(7,1).c_str());

		//TODO: Write file to config wireless
		CNetworkConfig::startWirelessNetwork(iface);

		CNetworkConfig::connectWirelessNetwork(iface,essid,*encrypt,password);
		// TODO show msg in case of error
		returnval = menu_return::RETURN_EXIT;
	}

	return returnval;
}

int CWIRELESSSearchGui::menu()
{
	CMenuWidget mountMenuW(LOCALE_WIRELESS_HEAD, "network.raw", 720);
	mountMenuW.addItem(GenericMenuSeparator);
	mountMenuW.addItem(GenericMenuBack);
	mountMenuW.addItem(GenericMenuSeparatorLine);
	char s2[12];

	iface = getWirelessInterface();

	//if not detect interface, try with ra0
	if (iface=="null")
		iface = "ra0";
	CNetworkConfig::startWirelessNetwork(iface);


	networks=getWirelessNetworks(iface);

	printf("Scan wireless networks\n");


	for (int i=0;i<20;i++)
	{
		printf("ESSID: %s\n", networks[i].c_str());
		if (!networks[i].empty())
		{
			strcpy(g_settings.network_wireless_essid[i],networks[i].c_str());
			nNetworkFound++;
		}
	}


	if (nNetworkFound>NETWORK_WIRELESS_NR_OF_ENTRIES)
		nNetworkFound= NETWORK_WIRELESS_NR_OF_ENTRIES;

	if (nNetworkFound==0)
	{

		sprintf(s2,"netentry%d");
		sprintf(ISO_8859_1_entry[0],ZapitTools::UTF8_to_Latin1("Not networks found, add manually").c_str());
		CMenuForwarderNonLocalized *forwarder = new CMenuForwarderNonLocalized("", true, ISO_8859_1_entry[0], this, s2);
		forwarder->iconName = NEUTRINO_ICON_NOT_MOUNTED;
		mountMenuW.addItem(forwarder);
	}
	else
	for(int i=0 ; i < nNetworkFound; i++)
		{
		sprintf(s2,"netentry%d",i);
		//sprintf(ISO_8859_1_entry[i],ZapitTools::UTF8_to_Latin1(m_entry[i]).c_str());
		sprintf(ISO_8859_1_entry[i],ZapitTools::UTF8_to_Latin1(networks[i].c_str()).c_str());  //Insert name of wifi here

		CMenuForwarderNonLocalized *forwarder = new CMenuForwarderNonLocalized("", true,g_settings.network_wireless_essid[i], this, s2);
		forwarder->iconName = NEUTRINO_ICON_NOT_MOUNTED;

		mountMenuW.addItem(forwarder);
		}

	int ret=mountMenuW.exec(this,"");
	return ret;
}

#warning MESSAGEBOX_NO_YES_XXX is defined in neutrino.cpp, too!
#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO  },
	{ 1, LOCALE_MESSAGEBOX_YES }
};

#define MESSAGEBOX_CRYTP_OPTIONS_COUNT 4
const CMenuOptionChooser::keyval MESSAGEBOX_CRYTP_OPTIONS[MESSAGEBOX_CRYTP_OPTIONS_COUNT] =
{
	{ 0, LOCALE_WIRELESS_TYPE_NONE  },
	{ 1, LOCALE_WIRELESS_TYPE_WEP },
	{ 2, LOCALE_WIRELESS_TYPE_WPA },
	{ 3, LOCALE_WIRELESS_TYPE_WPA2 }
};


int CWIRELESSSearchGui::menuEntry(int nr)
{
	char cmd[12];

	essid = g_settings.network_wireless_essid[nr];
	encrypt = &g_settings.network_wireless_encrypt[nr];
	password = g_settings.network_wireless_password[nr];

	sprintf(cmd,"connectnow%d",nr);


	CMenuWidget wirelessMenuEntryW(LOCALE_WIRELESS_HEAD, "network.raw",720);
	wirelessMenuEntryW.addItem(GenericMenuSeparator);
	wirelessMenuEntryW.addItem(GenericMenuBack);
	wirelessMenuEntryW.addItem(GenericMenuSeparatorLine);

	CStringInputSMS netEssid(LOCALE_WIRELESS_ESSID, essid, 30, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"abcdefghijklmnopqrstuvwxyz0123456789-_.,:|!?/ ");
	CMenuOptionChooser *netCrypt= new CMenuOptionChooser(LOCALE_WIRELESS_CRYPT, encrypt, MESSAGEBOX_CRYTP_OPTIONS, MESSAGEBOX_CRYTP_OPTIONS_COUNT, true);
	CStringInputSMS netPass(LOCALE_WIRELESS_PASSWORD, password, 30, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"abcdefghijklmnopqrstuvwxyz0123456789-_.,:|!?/ ");



	wirelessMenuEntryW.addItem(new CMenuForwarder(LOCALE_WIRELESS_ESSID      , true, g_settings.network_wireless_essid[nr], &netEssid       ));
	wirelessMenuEntryW.addItem(netCrypt);
	wirelessMenuEntryW.addItem(new CMenuForwarder(LOCALE_WIRELESS_PASSWORD      , true, g_settings.network_wireless_password[nr], &netPass       ));
	wirelessMenuEntryW.addItem(new CMenuForwarder(LOCALE_WIRELESS_NOW, true, NULL                         , this     , cmd ));

	int ret = wirelessMenuEntryW.exec(this,"");
	return ret;
}

