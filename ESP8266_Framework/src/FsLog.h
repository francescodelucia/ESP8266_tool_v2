#ifndef __FsLog___
	#define __FsLog__
	#include <string.h>
	#include <defs.h>
	#include <FS.h> 

	class FsLog{
	  private:
		String _nameLog;
		int _Lines;
		int _maxLines;
		int CurrentFile;
		String currentFileName;
		String otherFilename;
	  private:
	  public:
		FsLog(String nameLog,int MaxLines){ 
			this->_nameLog = nameLog;
			this->_maxLines = MaxLines;
			this->CurrentFile = 0;
			this->currentFileName = this->_nameLog + "0";
			this->otherFilename = this->_nameLog + "1";
		};
		void AddLogData(String ldata){
		  if(this->_Lines == 0)
		  {
			SPIFFS.remove(this->currentFileName.c_str());
			File f =SPIFFS.open(this->currentFileName.c_str(),"w");
			f.close();
		  }
		  File f = SPIFFS.open(this->currentFileName.c_str() , "a");
	      f.println(ldata.c_str());
		  f.close(); 
		  this->_Lines ++;
		  if(this->_Lines == this->_maxLines)
	      {
			if(this->CurrentFile == 0){
				this->currentFileName = this->_nameLog + "1";
				this->otherFilename = this->_nameLog + "0";
				this->CurrentFile = 1;
			}else{
				this->currentFileName = this->_nameLog + "0";
				this->otherFilename = this->_nameLog + "1";
				this->CurrentFile = 0;
			}  
			this->_Lines = 0;
		  }
		};
		int GetLogLines(){
			return this->_Lines;
		};
		String GetLineLog(int lLog){
			File F;
			int pos_line = this->_Lines + lLog;
			if(pos_line> this->_maxLines){
				F = SPIFFS.open(this->currentFileName.c_str(),"r");
				pos_line -=this->_maxLines; 
			}else{
				F = SPIFFS.open(this->otherFilename.c_str(), "r");
			}
			if (!F) {
				Serial.println("file open failed");
				return "";
			}  
			String ret = "";
			int count=0;

			while(F.available()){
				String s=F.readStringUntil('\n');
				if(count==pos_line){F.close();return s;}
				count ++;
			}
			F.close();
			return ret;
		};
		void DeleteLog(){
			String f = this->otherFilename;
			SPIFFS.remove(f.c_str());
			f = this->currentFileName;
			SPIFFS.remove(f.c_str());
		};
		String ReadFile(){
			String ret="";
			Serial.println(this->_Lines,DEC);
			int pos_line = this->_maxLines - this->_Lines;
			Serial.println(pos_line,DEC);
			Serial.println(this->otherFilename);
			File F = SPIFFS.open(this->otherFilename.c_str(),"r");
			if (!F) {
				Serial.println("file open failed");
			}
			int count=0;
			while(F.available()){
				String s=F.readStringUntil('\n');
				if(count > this->_Lines && this->_Lines != 0){
					s+='\n';
					ret+=s;
				}
				count++;
			}
			F.close();
			F = SPIFFS.open(this->currentFileName.c_str(),"r");
			while(F.available()){
				String s=F.readStringUntil('\n');
				s+='\n';
				ret+=s;
			}
			F.close();
			return ret;
		};
	};

#endif

