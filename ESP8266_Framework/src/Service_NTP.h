#ifndef _SERVICE_NTP_
#define _SERVICE_NTP_

#include <defs.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <cstring>

extern "C" {
#include "user_interface.h"
}

void NTP_data(void);

/* Don't hardwire the IP address or we won't get the benefits of the pool.
	Lookup the IP address for the host name instead */
	
IPAddress timeServerIP; // time.nist.gov NTP server address
char ntpServerName[256];  //= NTP_SERVER;

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
		
os_timer_t myTimer;
unsigned long secsSince1900;

int hours=0;
int minutes=0;
int seconds=0;

unsigned long timeNow = 0;
unsigned long timeLast = 0;
int utcOffsetInSeconds;

void TimeCorrection(){
	timeNow = millis()/1000; // the number of milliseconds that have passed since boot
	seconds = timeNow - timeLast;
	//the number of seconds that have passed since the last time 60 seconds was reached.
	if (seconds >= 60) {
		timeLast = timeNow;
		minutes = minutes + 1; 
	}
	//if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
	if (minutes >= 60){
		minutes = 0;
		hours = hours + 1; 
	}
}

void localSeconds()
{
	WiFiUDP ntpUDP;
	NTPClient timeClient(ntpUDP,ntpServerName,utcOffsetInSeconds);
	timeClient.begin();
	timeClient.update();
	
	hours = timeClient.getHours();
	minutes = timeClient.getMinutes();
	seconds = timeClient.getSeconds();
}

class Service_NTP{
	private:
		int _ltz = 0;
	protected:
	public:
		Service_NTP(String ntpserver):Service_NTP()
		{			
			memset(ntpServerName,0,256);
			std::strcpy(ntpServerName,ntpserver.c_str()); 
		}
		Service_NTP(){
			memset(ntpServerName,0,256);
			memcpy((void*)ntpServerName,(const void*)NTP_SERVER_,strlen((const char*)NTP_SERVER_));
#ifdef _NTP_DEBUG_
			Serial.println("begin NTP");  
#endif
			//udp.begin(UDP_PORT);
			//secsSince1900 = 0;
		}
		void SetLocalTimeZone (int ltz)
		{
			utcOffsetInSeconds =  ltz*3600; //this->_ltz = ltz*3600;
		}

		int GetLTZMinutes(){return minutes;}
		int GetLTZSeconds(){return seconds;}
		int GetLTZHours(){return hours;}
		
		int GetUTCMinutes(){return minutes;}
		int GetUTCSeconds(){return seconds;}
		int GetUTCHours(){return hours;}
		
		int SettUTCMinutes(int mins){minutes = mins;}
		int SetUTCSeconds(int secs){seconds = secs;}
		int SetUTCHours(int hrs){hours= hrs;}
		
		String GetNTPServer(){return String(ntpServerName);	}
			
};
#endif
