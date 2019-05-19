#include "HX711.h"
#include <WebSocketsServer.h>
#include <WifiTool.h>
#include <FsLog.h>
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266HTTPClient.h>
//#define _DBG_TRIG_
//#define _DBG_HTTP_

WiFiTool *tool;

#define calibration_factor 23000
#define zero_factor 200000

FsLog* myLog;
WebSocketsServer webSocket = WebSocketsServer(81);
WebSocketsServer webSocket2 = WebSocketsServer(82);

#define DOUT 4 // D2 maps to GPIO4
#define CLK 5  // D1 maps to GPIO5
#define SARRAY 5

// Initialize our HX711 interface
HX711 scale(DOUT, CLK);


String sData;
float fdata;
float avgData[5]={0};
int avgDataCount=0;

struct trig
{
  String Link;
  int vMax;
  int vMin;
  int proccessed;
} Triggers[20];
int trigCount = -1;

void storeTrigger(){
   const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
  trigCount = 0;
  String triggers="{\"triggers\":" +  tool->ReadFile("/trigger.json") + "}";
  Serial.println(triggers);
  DynamicJsonBuffer cmdJsonBuffer(bufferSize);
  JsonObject& jObj = cmdJsonBuffer.parseObject(triggers);
  int triggerSize =  jObj["triggers"].size();
  float vCur = avgArray(avgData);
  for(int k=0; k<triggerSize;k++){
      int vMax = (atoi(jObj["triggers"][k]["value1"]) > atoi(jObj["triggers"][k]["value2"])?atoi(jObj["triggers"][k]["value1"]):atoi(jObj["triggers"][k]["value2"]));
      int vMin = (atoi(jObj["triggers"][k]["value1"]) > atoi(jObj["triggers"][k]["value2"])?atoi(jObj["triggers"][k]["value2"]):atoi(jObj["triggers"][k]["value1"]));
      String sAction =  jObj["triggers"][k]["action"];
      Triggers[k].vMax = vMax;
      Triggers[k].vMin = vMin;
      Triggers[k].Link = sAction;
      Triggers[k].proccessed=0;
      trigCount=k;
  } 
}


void procTriggers(){
  float vCur = avgArray(avgData);
#ifdef  _DBG_TRIG_
  Serial.println("######################################");       
  Serial.print("Media vals:");
  Serial.println(vCur);
  Serial.print("trigCount:");
  Serial.println(trigCount);
  Serial.println("######################################");              
#endif  
  
  for(int k=0; k<trigCount+1;k++){

#ifdef  _DBG_TRIG_
    Serial.println("######################################");       
    Serial.print("vMax:");
    Serial.println(Triggers[k].vMax);
    Serial.print("vMin:");
    Serial.println(Triggers[k].vMin);
    Serial.print("sAction:");
    Serial.println(Triggers[k].Link);
    Serial.println("######################################");              
#endif  
      String lHttp = Triggers[k].Link.c_str();
      int vMax = Triggers[k].vMax;
      int vMin = Triggers[k].vMin; 
      if(  vCur < vMax && vCur > vMin ){
        if(Triggers[k].proccessed==0){
          HTTPClient http;  //Declare an object of class HTTPClient
          http.begin(lHttp);  //Specify request destination
          int httpCode = http.GET();  //Send the request      
          if (httpCode == 200) { //Check the returning code
            //String payload = http.getString();   //Get the request response payload
           Triggers[k].proccessed=1;
          }else if (httpCode > 400) {
           Triggers[k].proccessed=0; 
          }
          http.end();   //Close connection         
        }
     }else{Triggers[k].proccessed = 0;}
    }  
}
/*
void procTriggers(){
  const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
  trigCount = 0;
  String triggers="{\"triggers\":" +  tool->ReadFile("/trigger.json") + "}";
  Serial.println(triggers);
  DynamicJsonBuffer cmdJsonBuffer(bufferSize);
  JsonObject& jObj = cmdJsonBuffer.parseObject(triggers);
  int triggerSize =  jObj["triggers"].size();
#ifdef  _DBG_TRIG_
  Serial.println("######################################");       
  Serial.print("Media vals:");
  Serial.println(avgArray(avgData));
  Serial.print("trigCount:");
  Serial.println(triggerSize);
  Serial.println("######################################");              
#endif  
  float vCur = avgArray(avgData);
  for(int k=0; k<triggerSize;k++){
      int vMax = (atoi(jObj["triggers"][k]["value1"]) > atoi(jObj["triggers"][k]["value2"])?atoi(jObj["triggers"][k]["value1"]):atoi(jObj["triggers"][k]["value2"]));
      int vMin = (atoi(jObj["triggers"][k]["value1"]) > atoi(jObj["triggers"][k]["value2"])?atoi(jObj["triggers"][k]["value2"]):atoi(jObj["triggers"][k]["value1"]));
      String sAction =  jObj["triggers"][k]["action"];
#ifdef  _DBG_TRIG_
    Serial.println("######################################");       
    Serial.print("vMax:");
    Serial.println(vMax);
    Serial.print("vMin:");
    Serial.println(vMin);
    Serial.print("sAction:");
    Serial.println(sAction);
    Serial.println("######################################");              
#endif  
      if(  vCur < vMax && vCur > vMin ){
        Serial.println("++++++++++++++++++++++++++++++++");
        Serial.println("++++++++++++++++++++++++++++++++");  
          HTTPClient http;  //Declare an object of class HTTPClient
          http.begin(sAction.c_str());  //Specify request destination
          int httpCode = http.GET();  //Send the request      
          if (httpCode > 0) { //Check the returning code
            String payload = http.getString();   //Get the request response payload
#ifdef _DBG_HTTP_        
        Serial.println(payload);                     //Print the response payload
#endif        
      }
      http.end();   //Close connection
    }
  }  
}*/
void printArray(float arr[])
{
    int i;
    for(i=0; i<SARRAY; i++)
    {
        Serial.println(arr[i]);
    }
}
float avgArray(float arr[]){
    int i;
    float tot = 0.0;
    for(i=0; i<SARRAY; i++)
    {
        /* Move each array element to its left */
        tot += arr[i];
    }
    return tot/SARRAY;
}

void valMonitor(float arr[],float aData)
{
    int i;
    float first;
    /* Store first element of array */
    for(i=0; i<SARRAY-1; i++)
    {
        /* Move each array element to its left */
        arr[i] = arr[i + 1];
    }
    /* Copies the first element of array to last */
    arr[SARRAY-1] = aData;
}

void SendData() {
  scale.power_up();
  fdata = scale.get_units();
  scale.power_down();
  String we = String(fdata, 2);
  String h_ = String(tool->GetLTZHours());
  h_ += "." +  String(tool->GetLTZMinutes());
  h_ += "." + String(tool->GetLTZSeconds());
  sData = we + "," + h_ ;
  char const *x = sData.c_str();
  webSocket.broadcastTXT(x, strlen(x));
}


void storeData() {
  myLog->AddLogData(sData);
  valMonitor(avgData,fdata);
  String g = myLog->ReadFile();
  char const *x = g.c_str();
  webSocket2.broadcastTXT(x, strlen(x));
}


void HandleWebSocket() {
  webSocket.loop();
  webSocket2.loop();
  
}
void SetTara(){
    scale.power_up();
    scale.tare();      
    scale.power_down();
    tool->server->send(200, "text/html", "set tara = ok");
}

void StoreTrig(){
    String triggers=  tool->server->arg("plain");  
    tool->SaveFile("/trigger.json",triggers);
    storeTrigger();
    //Serial.println(triggers);
}

void setup() {
  tool = new WiFiTool();
  myLog = new FsLog("test.log",50);
  myLog->DeleteLog();
  //myserver = tool->server;
  //tool->AddWebRoot((void*)WeigthData);
  tool->AddEvent((void*)storeData, 10000);
  tool->AddEvent((void*)SendData, 1000);
  tool->AddEvent((void*)HandleWebSocket, 0);
  tool->AddEvent((void*)procTriggers,2000);
  tool->AddWebPage((void*)SetTara,"/tara");
  tool->AddWebPage((void*)StoreTrig,"/storeTrig");


  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket2.begin();
  webSocket2.onEvent(webSocketEvent2);

  scale.set_scale(calibration_factor);
  scale.tare();
  storeData();
  storeTrigger();
  if (!MDNS.begin("activebalance")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  


}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      break;
    case WStype_ERROR:
      break;
    default:
      break;
  }
}

void webSocketEvent2(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket2.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      break;
    case WStype_ERROR:
      break;
    default:
      break;
  }
}

void loop() {
  tool->HandlServerEvent();
}
