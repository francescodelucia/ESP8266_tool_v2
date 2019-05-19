#ifndef __ManFS___
	#define __ManFS__
	#ifndef _WIFI_DEBUG_
		#define _WIFI_DEBUG_
	#endif
	#include <string.h>
	#include <defs.h>
	#include <FS.h> 
	#include <ArduinoJson.h>
	#define JSON_MAX_SIZE 400

	class ManFS{
	  
	  private:
	  
	  public:
		void formatFS(){
		  //Format File System
		  if(SPIFFS.format())
		  {
	#ifdef _SPIFFS_DEBUG_		  
			Serial.println("File System Formated");
	#endif
		  }
		  else
		  {
	#ifdef _SPIFFS_DEBUG_
			Serial.println("File System Formatting Error");
	#endif        
		  }
		};
		void InitFS(){
		  if(SPIFFS.begin())
		  {
	#ifdef _SPIFFS_DEBUG_
			Serial.println("SPIFFS Initialize....ok");
	#endif
		  }
		  else
		  {
	#ifdef _SPIFFS_DEBUG_
			Serial.println("SPIFFS Initialization...failed");
	#endif
		  }      
		};
		Dir openDir(String path){
			return SPIFFS.openDir(path);
		};
	  public:
		ManFS(){
			this->InitFS(); 
		};
		int ExistFile(String fname){
			File f = SPIFFS.open(fname.c_str(), "r");
			if(!f){return 0;}else{return 1;}
		};
		void WriteFile(String fname,String data){
		  File f = SPIFFS.open(fname.c_str(), "w");
		  if (!f) {
	#ifdef _SPIFFS_DEBUG_
			Serial.println("failed to open file:");
			Serial.println(fname.c_str());
	#endif
		  }
		  else
		  {
			  //Write data to file
	#ifdef _SPIFFS_DEBUG_
			  Serial.print("Writing Data to file: ");
			  Serial.println(data.c_str());
	#endif
			  f.print(data.c_str());
			  f.close();  //Close file
		  }
		};
		void WriteDataToFile(String fname,String data){
		  File f = SPIFFS.open(fname.c_str(), "w");
		  if (!f) {
	#ifdef _SPIFFS_DEBUG_
			Serial.println("failed to open file:");
			Serial.println(fname.c_str());
	#endif
		  }
		  else
		  {
			  //Write data to file
	#ifdef _SPIFFS_DEBUG_
			  Serial.print("Writing Data to file: ");
			  Serial.println(data.c_str());
	#endif
			  f.println(data.c_str());
			  f.close();  //Close file
		  }
		};
		
		String GetDataByTag(String data,String myTag){
		  String ret="";
		  char * pch;
		  pch = strtok((char*)data.c_str(),"\n");
		  while (pch != NULL)
		  {    
			String ff = String(pch);
			if(ff.startsWith(myTag)){
			  char *pchData;
			  pchData = strtok(pch,":");
			  while (pchData != NULL)
			  {                 
				String pchData1 = String(pchData);
				if(!pchData1.startsWith(myTag)){
				  pchData1.trim();
				  ret = String(pchData1);
				}
				pchData = strtok (NULL, ":");     
			  }          
			}
			pch = strtok (NULL, "\n");
		  }
		  return ret;
		};
		File open(String fName,String mode){
			return SPIFFS.open(fName.c_str(),mode.c_str());
		};
		void RemFile(String fName){ 
		  if (SPIFFS.remove(fName)) {    
	#ifdef _SPIFFS_DEBUG_    
			Serial.println("failed to remove file:");
			Serial.println(fName.c_str());
	#endif
		  }
		  else
		  {
			  String _stData("");
	#ifdef _SPIFFS_DEBUG_
			  Serial.print("removed File:");
			  Serial.println(fName.c_str());
	#endif
		  }  
		};
		
		String ReadFile(String filename){
		  int i;
		  String data="";
		  File f = SPIFFS.open(filename, "r");    
		  if (!f) {    
	#ifdef _SPIFFS_DEBUG_    
			Serial.println("failed to open file:");
			Serial.println(filename.c_str());
	#endif
		  }
		  else
		  {
			  String _stData("");
	#ifdef _SPIFFS_DEBUG_
			  Serial.print("Reading Data from File:");
			  Serial.println(filename.c_str());
	#endif
			  //Data from file
			  for(i=0;i< f.size();i++) //Read upto complete file size
			  {
				 char x =(char)f.read();
				_stData+=x;
				
	#ifdef _SPIFFS_DEBUG_
				 Serial.print(x);
	#endif
			  }
			  data = String(_stData);
			  f.close();  //Close file
	#ifdef _SPIFFS_DEBUG_
			  Serial.println("File Closed");
	#endif
		  }  
		  return data;
		};
	};

	class JsonMan{

		private:
			String _fileName;
			ManFS* manFS;
		private:
			void Save(JsonObject& jR){
				String _data;
				jR.printTo(_data);
				this->manFS->RemFile(this->_fileName);
				this->manFS->WriteFile(this->_fileName,_data);
			};
			JsonObject& Read(){
				String out=this->manFS->ReadFile(this->_fileName);
				const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
				DynamicJsonBuffer cmdJsonBuffer(bufferSize);
				return cmdJsonBuffer.parseObject(out);
			}
		public:
			JsonMan(String _fileName){
				this->manFS = new ManFS();
				this->_fileName = _fileName;
			};
			int beginFile(){
				return this->manFS->ExistFile(this->_fileName);
			}
			void beginJsonData(String _jsonData){
				//StaticJsonBuffer<JSON_MAX_SIZE> cmdJsonBuffer;
				const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
				DynamicJsonBuffer cmdJsonBuffer(bufferSize);
				JsonObject& _jRoot = cmdJsonBuffer.parseObject(_jsonData);
				String _data;
				_jRoot.printTo(_data);
				this->manFS->WriteFile(this->_fileName,_data);
			};
			String GetStringByField(String field){ 
				JsonObject& _jRoot = this->Read();
				String out = _jRoot[field];
				return out;
			};
			
			int GetIntByField(String field){  
				JsonObject& _jRoot = this->Read();
				return _jRoot[field];
			};
			float GetFloatByField(String field){  
				JsonObject& _jRoot = this->Read();
				return _jRoot[field];
			};
			void AddStringByField(String field,String data){
				String out=this->manFS->ReadFile(this->_fileName);
				//StaticJsonBuffer<JSON_MAX_SIZE> cmdJsonBuffer;
				const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
				DynamicJsonBuffer cmdJsonBuffer(bufferSize);
				JsonObject& _jRoot = cmdJsonBuffer.parseObject(out);
				//JsonObject& _jRoot = this->Read();
				_jRoot[field] =data;
				this->Save(_jRoot);
				/*String _data;
				_jRoot.printTo(_data);
				this->manFS->WriteFile(this->_fileName,_data);*/
			};
			void AddIntByField(String field,int data){
				String out=this->manFS->ReadFile(this->_fileName);
				//StaticJsonBuffer<JSON_MAX_SIZE> cmdJsonBuffer;
				const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
				DynamicJsonBuffer cmdJsonBuffer(bufferSize);
				JsonObject& _jRoot = cmdJsonBuffer.parseObject(out);
				_jRoot[field] =data;
				this->Save(_jRoot);
				/*String _data;
				_jRoot.printTo(_data);
				this->manFS->WriteFile(this->_fileName,_data);*/
			};
			void AddFloatByField(String field,float data){
				String out=this->manFS->ReadFile(this->_fileName);
				//StaticJsonBuffer<JSON_MAX_SIZE> cmdJsonBuffer;
				const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
				DynamicJsonBuffer cmdJsonBuffer(bufferSize);
				JsonObject& _jRoot = cmdJsonBuffer.parseObject(out);
				_jRoot[field] =data;
				this->Save(_jRoot);
				/*String _data;
				_jRoot.printTo(_data);
				this->manFS->WriteFile(this->_fileName,_data);*/
			};
			void AddArrayStringByField(String field,String* array,int size){

			};
			void AddArrayFloatByField(String field,float* array,int size){

			};
			void AddArrayIntByField(String field,int* array,int size){

			};
			String ReadJsonFile(){
				return this->manFS->ReadFile(this->_fileName);
			};
	};
#endif
