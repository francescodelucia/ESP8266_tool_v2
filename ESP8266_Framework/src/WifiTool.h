#ifndef _WIFI_TOOL_
#define _WIFI_TOOL_
#include <defs.h>
//#include <ESP8266FtpServer.h>

extern "C" {
#include "user_interface.h"
}

#define CRITICAL_BEGIN 	this->UserScheduler->DisableAllEvent();this->SysScheduler->DisableAllEvent();
#define CRITICAL_END	this->UserScheduler->RestoreAllEvent();this->SysScheduler->RestoreAllEvent();
extern void localSeconds();
extern void TimeCorrection();
typedef void (*httpFunction)();
typedef void(*botCallBack)(void);


int LogTimeEvent = 0;



void Diagnostic_Comunication()
{	
#ifdef _WIFI_DEBUG_
	Serial.println("Diagnostic_Comunication: ");
#endif
	if(WiFi.status()!= WL_CONNECTED){system_restart();}
}

class WiFiTool{
	private:
		//int _personalRootWebPage = 0;
		WiFiComunication* wificom = NULL; 
		ESP8266_ScanNetwork *scan = NULL;
		Scheduler *SysScheduler=NULL;
		Scheduler *UserScheduler=NULL;
		Drivers *drivers =NULL;
		JsonMan *_Config;
		ManFS * _fs;
#ifdef EN_BOT
		TelegramBOT *teleBOT = NULL;
#endif		   

/*
 * Implement te remote Updater, but on ESP model with low Memory onboard this option doesn't work,
 * and in every case you need consider the maximun Maximun memory size of our programm not exceed of 
 *  50% of total memory, just the Framework it is 25% of memory on device with 1M memory size.
 * */
#ifndef ESP_8266_01
		ESP8266HTTPUpdateServer *httpUpdater = NULL;
#endif

		Service_NTP *sntp = NULL;
		int WifiClient;
	protected:
		int NtpEvent = -1;
	public:
		ESP8266WebServer *server= NULL;
		/*
		 * This Routine(WebPage) save the system parameter on Json
		 */
		void storeData(){
			String configuration=  this->server->arg("plain");
			this->_fs->WriteFile("/config.json",configuration);
			/*Force to Reboot*/
			WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while(1)wdt_reset();
		}
		void SaveFile(String fName,String fData){
			this->_fs->WriteFile(fName,fData);
		}
		String ReadFile(String fName){
			return this->_fs->ReadFile(fName);
		}
		 /*
		 * This routine handle web page on SPIFFS
		 * */
		void PageFS(){
			PageFS(this->server->uri());
		}
		void PageFS(String pName){
			if( pName == "/scanData"){
				CRITICAL_BEGIN
				String sNets=  this->scan->EnumNetwork();
				this->_fs->WriteFile("/scanData.json",sNets);
				this->server->send(200, "text/html", sNets.c_str());  
				CRITICAL_END
			}
			else if(this->_fs->ExistFile(pName)==1){
				if(this->server->method() == HTTP_GET){
					String message = this->_fs->ReadFile(pName);
					this->server->send(200, "text/html", message.c_str());  
				}else{
					String message = this->_fs->ReadFile(pName);
					this->server->send(200, "text/html", message.c_str());  
				}
			}else{
				String message = NO_WEB_PAGE;
				message += "URI: ";
				message += this->server->uri();
				message += "\nMethod: ";
				message += (this->server->method() == HTTP_GET)?"GET":"POST";
				message += "\nArguments: ";
				message += this->server->args();
				message += "\n";
				for (uint8_t i=0; i<this->server->args(); i++){
					message += " " + this->server->argName(i) + ": " + this->server->arg(i) + "\n";
				}
				this->server->send(404, "text/plain", message);  
			}
		}
		
		String SystemInformation()
		{
			String tmp;
			tmp =  "Flash memory:     " + String(ESP.getFlashChipSize())+" Bytes.\n";
			tmp += "Free heap memory: " + String(ESP.getFreeHeap()) + " Bytes.\n";
			tmp += "Chip speed:       " + String(ESP.getFlashChipSpeed())+" Hz.\n";
			return tmp;
		}
		int strToInt (const char str[])
		{
			int i, result = 0;
			int negative;
			if (str[0] == '-'){negative = 1;}else{negative = 0;}
			if (atoi(&str[1]) > 0){result = atoi(&str[1]);}else{result = atoi(&str[2]);}
			if (negative == 1){result = result*(-1);}else{result = result;}
			return result;
		}
		
		//format bytes
		String formatBytes(size_t bytes) {
		  if (bytes < 1024) {
			return String(bytes) + "B";
		  } else if (bytes < (1024 * 1024)) {
			return String(bytes / 1024.0) + "KB";
		  } else if (bytes < (1024 * 1024 * 1024)) {
			return String(bytes / 1024.0 / 1024.0) + "MB";
		  } else {
			return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
		  }
		}
		
		String getContentType(String filename) {
			if (this->server->hasArg("download")) return "application/octet-stream";
			else if (filename.endsWith(".htm")) return "text/html";
			else if (filename.endsWith(".html")) return "text/html";
			else if (filename.endsWith(".css")) return "text/css";
			else if (filename.endsWith(".js")) return "application/javascript";
			else if (filename.endsWith(".json")) return "application/json";
			else if (filename.endsWith(".png")) return "image/png";
			else if (filename.endsWith(".gif")) return "image/gif";
			else if (filename.endsWith(".jpg")) return "image/jpeg";
			else if (filename.endsWith(".ico")) return "image/x-icon";
			else if (filename.endsWith(".xml")) return "text/xml";
			else if (filename.endsWith(".pdf")) return "application/x-pdf";
			else if (filename.endsWith(".zip")) return "application/x-zip";
			else if (filename.endsWith(".gz")) return "application/x-gzip";
			return "text/plain";
		}

		bool handleFileRead(String path) {
		  if (path.endsWith("/")) {
			path += "index.htm";
		  }
		  String contentType = getContentType(path);
		  String pathWithGz = path + ".gz";
		  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
			if (SPIFFS.exists(pathWithGz)) {
			  path += ".gz";
			}
			File file = SPIFFS.open(path, "r");
			this->server->streamFile(file, contentType);
			file.close();
			return true;
		  }
		  return false;
		}

		void handleFileUpload() {
		  CRITICAL_BEGIN
		  String page=  this->server->arg("plain");
		  const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;  
		  DynamicJsonBuffer jsonBuffer(bufferSize);
		  JsonObject& _jRoot = jsonBuffer.parseObject(page);
		  String fname = _jRoot["fn"];
		  String fdata = _jRoot["html"];
		  this->_fs->WriteFile(fname,fdata);
		  CRITICAL_END
		}

		void handleFileDelete() {
		  if (this->server->args() == 0) {return this->server->send(500, "text/plain", "BAD ARGS");}
		  String path = this->server->arg(0);
		  //DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
		  if (path == "/") {return this->server->send(500, "text/plain", "BAD PATH");}
		  if (!this->_fs->ExistFile(path)) {return this->server->send(404, "text/plain", "FileNotFound");}
		  this->_fs->RemFile(path);
		  this->server->send(200, "text/plain", "");
		  path = String();
		}

		void handleFileCreate() {
		  if (this->server->args() == 0) {return this->server->send(500, "text/plain", "BAD ARGS");}
		  String path = this->server->arg(0);
		  if (path == "/") {return this->server->send(500, "text/plain", "BAD PATH");}
		  if (this->_fs->ExistFile(path)) {return this->server->send(500, "text/plain", "FILE EXISTS"); }
		  File file = this->_fs->open(path, "w");
		  if (file) {file.close();}
		  else {return this->server->send(500, "text/plain", "CREATE FAILED");}
		  this->server->send(200, "text/plain", "");
		  path = String();
		}

		void handleFileList() {
		  if (!this->server->hasArg("dir")) {
			this->server->send(500, "text/plain", "BAD ARGS");
			return;
		  }
		  String path = this->server->arg("dir");
		  Serial.println("handleFileList: " + path);
		  //DBG_OUTPUT_PORT.println("handleFileList: " + path);
		  Dir dir = this->_fs->openDir(path);
		  path = String();
		  String output = "[";
		  while (dir.next()) {
			File entry = dir.openFile("r");
			if (output != "[") {
			  output += ',';
			}
			bool isDir = false;
			output += "{\"type\":\"";
			output += (isDir) ? "dir" : "file";
			output += "\",\"name\":\"";
			output += String(entry.name()).substring(1);
			output += "\"}";
			entry.close();
		  }
		  output += "]";
		  Serial.println("handleFileList: " + output);
		  this->server->send(200, "text/json", output);
		}
		void ApplyConfigNTP(){
			int ConfStat = this->_Config->beginFile();
			int cNTPOn = 0;
			int cNtpTZ = 0;
			String cNTP = "" ;
			if(this->WifiClient == WIFI_CLIENT){
				if(ConfStat==1){
					cNTP = this->_Config->GetStringByField("NTP");
					cNTPOn = (this->_Config->GetStringByField("NTP_ON")=="On"?1:0);
					cNtpTZ = strToInt(this->_Config->GetStringByField("NTP_TIME_ZONE").c_str());
				}
				if(cNTPOn==1){
					Serial.printf("ntp %i\n",cNTPOn);
					this->sntp = new Service_NTP(cNTP);
					this->sntp->SetLocalTimeZone(cNtpTZ);
					this->NtpEvent = this->SysScheduler->AddEvent((void*)localSeconds,60000);
					localSeconds();
					localSeconds();
					localSeconds();
				}else{
					if(this->NtpEvent !=-1)
					{
						this->SysScheduler->RemoveEvent(this->NtpEvent);
					}
				}
				
			}
		}
		void ApplyConfig(){
			int ConfStat = this->_Config->beginFile();
			int cServerPort = -1;
			String cServerMAC = "";
			String cServerSSID = ""; 
			String cServerPWD = "";
			String cClientSSID = "";
			String cClientPWD = "" ;
			
			if(ConfStat==1){
				cServerPort = this->_Config->GetIntByField("SERVER_PORT");
				cServerMAC = this->_Config->GetStringByField("MAC");
				cServerSSID = this->_Config->GetStringByField("SERVER_SSID");
				cServerPWD = this->_Config->GetStringByField("SERVER_PWD");
				cClientSSID = this->_Config->GetStringByField("CLIENT_SSID");
				cClientPWD = this->_Config->GetStringByField("CLIENT_PWD");
			}
			if(ConfStat==0){
				this->server = new ESP8266WebServer(WEB_PORT);
			}else{
				if((int)cServerPort<1)
				{
					this->server = new ESP8266WebServer(WEB_PORT);
				}else{
					this->server = new ESP8266WebServer(cServerPort);
				}
			}
#ifdef _WIFI_DEBUG_
			Serial.printf("SSID %s PWD %s\n",cClientSSID.c_str(),cClientPWD.c_str());  
#endif
			if(ConfStat==0){
				this->wificom = new WiFiComunication((char*)cClientSSID.c_str(),(char*)cClientPWD.c_str());
				String jData = "{MAC:'" + String(this->wificom->MAC_char) 
							+ "',SERVER_SSID:'ESP8266" + String(this->wificom->MAC_char) 
							+ "',SERVER_PWD:'" + String(this->wificom->MAC_char) 
							+ "',SERVER_PORT:'80',CLIENT_SSID:'',CLIENT_PWD:'',NTP:'ntp1.inrim.it',NTP_ON:0,NTP_TZ:0}";
				this->_Config->beginJsonData(jData);
				
			}
			else
			{
				this->wificom = new WiFiComunication((char*)cClientSSID.c_str(),(char*)cClientPWD.c_str(),(char*)cServerSSID.c_str(),(char*)cServerPWD.c_str());
			}
			if(this->wificom->CreateWFClient() == ACCESS_POINT)
			{         
				this->WifiClient = ACCESS_POINT;       
				this->WebServerRequests();
				this->server->begin();
			}
			else 
			{
				this->WifiClient = WIFI_CLIENT;
				this->WebServerRequests();
				this->SysScheduler->AddEvent((void*)Diagnostic_Comunication,60000);
				this->ApplyConfigNTP();
				this->server->begin();
			}
		}
	public:	
		/*
		 * Costructor overload  where choose if you want customize Root page
		 * */
		WiFiTool(){
			ESP.eraseConfig();
			delay(500);
			
			WifiClient = ACCESS_POINT;
			Serial.begin(_BAUD_);   
			this->_Config = new JsonMan("/config.json");
			this->_fs = new ManFS();
			
			
#ifdef _WIFI_DEBUG_
			Serial.println("#######################################################");
			Serial.println("Esp8266 Framework by Francesco De lucia ver 0.180102");
			Serial.println("This Messages are Debug comunicatio and system Messages");  
			Serial.println("If you want remove it. Comment following lines:");  
			Serial.println("\n#define _WIFICOM_DEBUG_");
			Serial.println("#define _WIFIMEM_DEBUG_");
			Serial.println("#define _WIFI_DEBUG_");
			Serial.println("#define _NTP_DEBUG_\n");
			Serial.println("#define _SPIFFS_DEBUG_\n");
			Serial.println("on file defs.h");
			Serial.println("#######################################################");  
#endif	 
#ifndef ESP_8266_01
			this->httpUpdater = new ESP8266HTTPUpdateServer();
#endif
		
			this->SysScheduler = new Scheduler();
			this->UserScheduler = new Scheduler();
			
			this->scan = new ESP8266_ScanNetwork();
			this->drivers = new Drivers();
			this->drivers->InitDriver();
			
			this->ApplyConfig();
			
		}
		void WebServerRequests(){
			this->server->on("/",[this](){PageFS("/index.htm");});
			this->server->on("/store", [this](){storeData();});
			this->server->on("/reboot", [this](){PageFS("/reboot.htm"); WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while(1)wdt_reset();});
			this->server->on("/list", HTTP_GET, [this](){handleFileList();});
			this->server->on("/edit", HTTP_GET, [this]() {
				if (!handleFileRead("/edit.html")) {this->server->send(404, "text/plain", "FileNotFound");}
			});
			this->server->on("/edit", HTTP_PUT, [this](){handleFileCreate();} );
			this->server->on("/edit", HTTP_DELETE, [this](){handleFileDelete();} );
			this->server->on("/edit",HTTP_POST, [this](){handleFileUpload();});
			this->server->onNotFound([this](){PageFS();});
#ifndef ESP_8266_01
				this->httpUpdater->setup(this->server);
#endif			
		}
		void AddWebPage(void* callbackWebRoutine,char* webpagename) {
			this->server->on(webpagename,(void(*)())callbackWebRoutine);
		} 
		void AddPostData(void* callbackWebRoutine,char* webpagename) {
			this->server->on(webpagename, HTTP_POST,(void(*)())callbackWebRoutine);
		} 
		void SetLocalTimeZone(int ltz)
		{
			if (this->sntp != NULL)
			{
				this->sntp->SetLocalTimeZone(ltz);
			}
		}	
		String GetIp()
		{						
			return this->wificom->GetIp();
		}
		char* GetNTP(){		
			return (char*)this->sntp->GetNTPServer().c_str();
		}
		char* GetClientSSID()
		{
			
			return (char*)this->_Config->GetStringByField("CLIENT_SSID").c_str();
		}
		char* GetServerSSID()
		{
			return (char*)this->_Config->GetStringByField("SERVER_SSID").c_str();
		}
		int GetUTCMinutes(){
			if(this->sntp != NULL){
				return this->sntp->GetUTCMinutes();
			}
			return 0;
		}
		int GetUTCHours(){
			if(this->sntp != NULL){
				return this->sntp->GetUTCHours();
			}
			return 0;
		}
		int GetUTCSeconds(){
			if(this->sntp != NULL){
				return this->sntp->GetUTCSeconds();
			}
			return 0;
		}
		int GetLTZMinutes(){
			if(this->sntp != NULL){
				return this->sntp->GetLTZMinutes();
			}
			return 0;
		}
		int GetLTZHours(){
			if(this->sntp != NULL){
				return this->sntp->GetLTZHours();
			}
			return 0;
		}
		int GetLTZSeconds(){
			if(this->sntp != NULL){
				return this->sntp->GetLTZSeconds();
			}
			return 0;
		}
		int IsWifiClient()
		{
			return  WifiClient;
		}
		int AddEvent(void *pCallBack,int millisecond_event)
		{
			return this->UserScheduler->AddEvent(pCallBack,millisecond_event);
		}
		void DisableEvent(int ID_EVENT)
		{
			this->UserScheduler->DisableEvent(ID_EVENT);
		}
		void DisableAllEvent()
		{
			this->UserScheduler->DisableAllEvent();
		}
		void RestoreAllEvent()
		{
			this->UserScheduler->RestoreAllEvent();
		}
		void DisableAllSystemEvent()
		{
			this->SysScheduler->DisableAllEvent();
		}
		void RestoreAllSystemEvent()
		{
			this->SysScheduler->RestoreAllEvent();
		}
		void ModifyEvent(int ID_EVENT,void *pCallBack,int millisecond_event)
		{
			this->UserScheduler->ModifyEvent(ID_EVENT,pCallBack,millisecond_event);
		}
		void HandlServerEvent()
		{			
			this->server->handleClient();
			if (this->WifiClient != ACCESS_POINT){
				this->drivers->poolingDriver();
				this->SysScheduler->RunScheduler();
				this->UserScheduler->RunScheduler();
				TimeCorrection();
			}
		}
		void s_print(char *msg)
		{
			Serial.println(msg);
		}
		int* GetDriversData(int dType,void* data)
		{			
			this->drivers->GetDataDriver(dType,data);
		}
		void SendDisplayCommand(int dType,void* data,void* optional,void* optional1,void* optional2){
			this->drivers->DisplayCommand(dType,data,optional,optional1,optional2);
		}
		void SendDisplayCommand(int dType,void* data,void* optional){
			this->drivers->DisplayCommand(dType,data,optional,NULL,NULL);
		}
		void SendDisplayCommand(int dType,void* data){
			this->drivers->DisplayCommand(dType,data,NULL,NULL,NULL);
		}
		void SendDisplayCommand(int dType){
			this->drivers->DisplayCommand(dType,NULL,NULL,NULL,NULL);
		}
#ifdef TELEGRAM_BOT
		TelegramBOT *SetTelegramBOT(String token, String name, String username, void* callbackRoutine){	
			this->teleBOT = new TelegramBOT(token, name, username);
			this->teleBOT->begin();
			this->SysScheduler->AddEvent(callbackRoutine,1000);
			return this->teleBOT;
		}
		
		TelegramBOT *GetTelegramBot(){
			return this->teleBOT;
		}
#endif		
};
#endif
