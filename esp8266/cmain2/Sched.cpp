#include "Sched.h" //private: 
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <PubSubClient.h>
#include "config.h"
#include "MQclient.h" //for extern NEW_MAIL
#include "STATE.h"
#include "PORTS.h"
#include "TMR.h"

char * schedArr[]={"temp1","temp2","tmr1","tmr2","tmr3"};
//int NEW_ALARM = -1;


bool Sched::deserialize(char* kstr){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(kstr);
  root.prettyPrintTo(Serial);
  unix = root["unix"];
  LLLL = root["LLLL"];
  zone = root["zone"];
  return root.success();
}

void Sched::actTime(STATE& st){
	Serial.println(unix);
	Serial.println(LLLL);
	Serial.println(zone);
  time_t datetime = unix + zone*60*60;
  Serial.println(datetime); 
  setTime(datetime);
  setSyncInterval(4000000); 
  Serial.println(hour());  	
	NEW_MAIL=0;
	//Alarm.alarmOnce(hour(), minute()+1,0,abdd);
}

int Sched::idxOsenrels(int j){
	for(int i=0;i<seresz;i++){
		if(senrels[i]==j){
			return i;
		} 
	} 
}

void Sched::printSched(int i){
	Serial.println(schedArr[idxOsenrels(i)]);
	Serial.print("haynRset=");
	Serial.println(haynRset[i]);
	for(int j = 0; j<events[i]; j++){//4
		for(int k=0; k<haynRset[i]+2;k++){
			Serial.print(progs[i][j][k]);
			Serial.print(",");
		}
		Serial.println("");  
	}
	Serial.println("");  
}


bool Sched::deseriProgs(char* kstr){
	//DynamicJsonBuffer jsonBuffer;
  StaticJsonBuffer<2000> jsonBuffer;
	JsonObject& rot = jsonBuffer.parseObject(kstr);
	crement = rot["crement"];
	JsonArray& sere = rot["serels"];
	JsonArray& root = rot["progs"];
	seresz = sere.size();
	for(int h=0;h<seresz;h++){
		senrels[h] = sere[h];
	}
	nsr = root.size();
	Serial.println("");  
	Serial.print("the # of sinks is: "); //nsr in num of progs
	Serial.println(nsr);	
	for(int i = 0; i<nsr; i++){
		JsonArray& asnk = root[i]; //asnk[4][3] the prog for some sensor/relay
		events[i] = root[i].size(); //4 the number events in that sen/rel prog
		Serial.println(schedArr[idxOsenrels(i)]); //finds the name of the sen/rel by scanning for 'i'  and grabbing the index
		// Serial.println(idxOsenrels(i));
		for(int j = 0; j<events[i]; j++){//4 for each event of i
			int bsz = asnk[j].size(); //how much data is here
			haynRset[i] = bsz-2; // the first two items are hour min haynRset tells you how many data elements are included in this program 
			for(int k=0; k<bsz;k++){ 
				progs[i][j][k] = asnk[j][k]; //copy the data to the 3d progs array 
				Serial.print(progs[i][j][k]);
				Serial.print(",");
			}
			Serial.println("");  
		}
		Serial.println("");  
	}
	Serial.println(progs[0][1][2]);
	Serial.println(progs[1][1][2]);
  return rot.success();
}

//[CYURD001/progs] {"crement":5,"serels":[0,99,1,2],"progs":[[[0,0,80,77],[6,12,82,75],[8,20,85,75],[22,0,78,74],[23,30,85,75]],[[0,0,58],[18,0,68],[21,30,58]],[[0,0,0],[12,34,1],[12,37,0]]]}
//var sched3 = "{\"crement"\:5,"serels\":[0,99,1,2],\"progs\":[[[0,0,80,77],[6,12,82,75],[8,20,85,75],[22,0,78,74],[23,30,85,75]],[[0,0,58],[18,0,68],[21,30,58]],[[5,30,1],[6,10,0]]]}";
//char * schedArr[]={"temp1","temp2","tmr1","tmr2","tmr3"};
//events =  [5,3,2]
//haynRset= [2,1,1]

void Sched::resetAlarm(int i, int &cur, int &nxt){
	//int cur =0;
	Serial.println("in resetAlarm");
	int idx = senrels[i];
	printSched(idx);
	char senrel[10];
	strcpy(senrel,schedArr[i]);
	//make cur the 
	for(int j=0; j<events[idx];j++){
		if (hour() == progs[idx][j][0]){
			if (minute() < progs[idx][j][1]){
				cur = j-1;
				break;
			}
		}
		if (hour() < progs[idx][j][0]){
			Serial.print("should breat at: ");
			Serial.println(j);
			Serial.println(progs[idx][j][0]);
			cur= j-1;
			break;
		}
		cur =j;
	}
	nxt = cur+1;
	if (nxt>=events[idx]){
		nxt=0;
	}
	Serial.println(cur);
	Serial.println(senrel);
	Serial.print(hour());
	Serial.print(":");
	Serial.print(minute());
	Serial.println();
}

//first time through NEW_ALARM=31=11111
void Sched::actProgs2(TMR& tmr, STATE& st){
	tmr.crement=crement;
	Serial.print("in actProgs2, NEW_ALARM=");
	Serial.println(NEW_ALARM);
	if((NEW_ALARM & 1) == 1){
		Serial.print("temp1 w mask62 :");
		NEW_ALARM = NEW_ALARM & 62;
		Serial.println(NEW_ALARM);
		Serial.print(hour());
		Serial.print(":");
		Serial.println(minute());
		int i0 = 0;
		int ii= senrels[i0];
		int cur = 0;
		int nxt = 0;
		resetAlarm(i0,cur, nxt);
		Serial.print("current: ");
		Serial.println(cur);
		Serial.print("next: ");
		Serial.println(nxt);
		Serial.print("current alarm is for: ");
		Serial.print(progs[ii][cur][0]);
		Serial.print(":");
		Serial.print(progs[ii][cur][1]);	
		Serial.print("->");
		Serial.print(progs[ii][cur][2]);	
		Serial.print(",");
		Serial.println(progs[ii][cur][3]);	
		st.temp1hi = progs[ii][cur][2];
		st.temp1lo = progs[ii][cur][3];
		st.HAY_CNG=1;
		Alarm.alarmOnce(progs[ii][nxt][0],progs[ii][nxt][1], 0, bm1);
	}
	if((NEW_ALARM & 2) == 2){
		Serial.print("temp2 w mask61 :");
		NEW_ALARM = NEW_ALARM & 61;
		Serial.println(NEW_ALARM);
		Serial.print(hour());
		Serial.print(":");
		Serial.println(minute()+1); 	
		//Alarm.alarmOnce(hour(), minute()+1,9,bm2);
	}
	if((NEW_ALARM & 4) == 4){
		Serial.print("timr1 4 w mask59 :");
		NEW_ALARM = NEW_ALARM & 59;
		Serial.println("timr1 mask 59");
		int i0 = 2;
		int ii= senrels[i0];
		int cur = 0;
		int nxt = 0;
		resetAlarm(i0,cur, nxt);
		if(progs[ii][cur][2]==1){
			Serial.print("countdown tmr1 starts at: ");
			int fhr = progs[ii][cur+1][0];
			int fmi = progs[ii][cur+1][1];
			int shr = progs[ii][cur][0];
			int smin = progs[ii][cur][1];
			int mi;
			int hr;
			if(fmi< smin){
				mi = 60-smin+fmi;
				shr++;
			}else {
				mi = fmi -smin;
			}
			hr = fhr - shr;
			int dur = 60*hr + mi;
			tmr.timr1= dur*60;
			Serial.println(dur);
		}else {
			tmr.timr1=0;
			Serial.println("countdown tmr1 is OFF");
		}
		Serial.print("next countdown timr1 is set for: ");
		Serial.print(progs[ii][nxt][0]);
		Serial.print(":");
		Serial.print(progs[ii][nxt][1]);	
		Serial.print("->");
		Serial.println(progs[ii][nxt][2]);				
		Alarm.alarmOnce(progs[ii][nxt][0],progs[ii][nxt][1], 1, bm4);			
	}
	if((NEW_ALARM & 8) == 8 && senrels[3]<99){
		NEW_ALARM = NEW_ALARM & 55;
		Serial.println("timr2 mask 55");
		int i0 = 3;
		int ii= senrels[i0];
		int cur = 0;
		int nxt = 0;
		resetAlarm(i0,cur, nxt);
		if(progs[ii][cur][2]==1){
			Serial.print("countdown tmr2 starts at: ");
			int fhr = progs[ii][cur+1][0];
			int fmi = progs[ii][cur+1][1];
			int shr = progs[ii][cur][0];
			int smin = progs[ii][cur][1];
			int mi;
			int hr;
			if(fmi< smin){
				mi = 60-smin+fmi;
				shr++;
			}else {
				mi = fmi -smin;
			}
			hr = fhr - shr;
			int dur = 60*hr + mi;
			tmr.timr2= dur*60;
			Serial.println(dur);
		}else {
			tmr.timr2=0;
			Serial.println("countdown tmr2 is OFF");
		}
		Serial.print("next countdown timr2 is set for: ");
		Serial.print(progs[ii][nxt][0]);
		Serial.print(":");
		Serial.print(progs[ii][nxt][1]);	
		Serial.print("->");
		Serial.println(progs[ii][nxt][2]);				
		Alarm.alarmOnce(progs[ii][nxt][0],progs[ii][nxt][1], 2, bm8);			
	}
	if((NEW_ALARM & 16) == 16){
		Serial.print("timr3 16 w mask47 :");
		NEW_ALARM = NEW_ALARM & 47;
		int i0 = 4;
		int ii= senrels[i0];
		int cur = 0;
		int nxt = 0;
		resetAlarm(i0,cur, nxt);
		if(progs[ii][cur][2]==1){
			Serial.print("countdown tmr3 starts at: ");
			int fhr = progs[ii][cur+1][0];
			int fmi = progs[ii][cur+1][1];
			int shr = progs[ii][cur][0];
			int smin = progs[ii][cur][1];
			int mi;
			int hr;
			if(fmi< smin){
				mi = 60-smin+fmi;
				shr++;
			}else {
				mi = fmi -smin;
			}
			hr = fhr - shr;
			int dur = 60*hr + mi;
			tmr.timr3= dur*60;
			Serial.println(dur);
		}else {
			tmr.timr3=0;
			Serial.println("countdown tmr3 is OFF");
		}
		Serial.print("next countdown timr3 is set for: ");
		Serial.print(progs[ii][nxt][0]);
		Serial.print(":");
		Serial.print(progs[ii][nxt][1]);	
		Serial.print("->");
		Serial.println(progs[ii][nxt][2]);				
		Alarm.alarmOnce(progs[ii][nxt][0],progs[ii][nxt][1], 3, bm16);		
	}
	if((NEW_ALARM & 32) == 32){
		Serial.print("reset for 1 min and shutting off 32 w mask31 :");
		NEW_ALARM = NEW_ALARM & 31;
		Serial.println(NEW_ALARM);
		Serial.print(hour());
		Serial.print(":");
		Serial.println(minute()+1); 	
		Alarm.alarmOnce(hour(), minute()+1,15,bm32);
	}
}

void Sched::updateTmrs(TMR& tmr, PubSubClient& client, STATE& st, PORTS& po){
	if((IS_ON & 4) == 4){
		int hl = HIGH;
		tmr.timr1 = tmr.timr1 - tmr.crement;
		if(tmr.timr1 <= 0){
			hl = LOW;
			tmr.timr1 = 0;
			IS_ON = IS_ON & 59;
		}
		if (digitalRead(po.timr1) != hl){
			digitalWrite(po.timr1, hl);
		}
		// Serial.print("timr1 port-> hl ");
		// Serial.print(po.timr1);
		// Serial.print("-> ");
		// Serial.println(hl);
	}	
	if((IS_ON & 8) == 8){
		int hl = HIGH;
		tmr.timr2 = tmr.timr2 - tmr.crement;
		if(tmr.timr2 <= 0){
			hl = LOW;
			tmr.timr2 = 0;
			IS_ON = IS_ON & 55;
		}
		if (digitalRead(po.timr2) != hl){
			digitalWrite(po.timr2, hl);
		}
	}	
	if((IS_ON & 16) == 16){
		int hl = HIGH;
		tmr.timr3 = tmr.timr3 - tmr.crement;
		if(tmr.timr3 <= 0){
			hl = LOW;
			tmr.timr3 = 0;
			IS_ON = IS_ON & 47;
		}
		if (digitalRead(po.timr3) != hl){
			digitalWrite(po.timr3, hl);
		}
		// Serial.print("timr3 port-> hl ");
		// Serial.print(po.timr3);
		// Serial.print("-> ");
		// Serial.println(hl);
	}
	tmr.IS_ON	= IS_ON;
}

void bm32(){
	Serial.print("mask with 32 begets: ");
	NEW_ALARM = NEW_ALARM | 32;
	IS_ON = IS_ON | 32;
	Serial.println(NEW_ALARM);
}
void bm16(){
	Serial.print("timr3 16 begets: ");
	NEW_ALARM = NEW_ALARM | 16;
	IS_ON = IS_ON | 16;
	Serial.println(NEW_ALARM);
}
void bm8(){
	Serial.print("timr2 8 begets: ");
	NEW_ALARM = NEW_ALARM | 8;
	IS_ON = IS_ON | 8;
	Serial.println(NEW_ALARM);
}
void bm4(){
	Serial.print("timr1 4 begets: ");
	NEW_ALARM = NEW_ALARM | 4;
	IS_ON = IS_ON | 4;
	Serial.println(NEW_ALARM);
}
void bm2(){
	Serial.print("temp2 2 begets: ");
	NEW_ALARM = NEW_ALARM | 2;
	IS_ON = IS_ON | 2;
	Serial.println(NEW_ALARM);
}
void bm1(){
	Serial.print("temp1 1 begets: ");
	NEW_ALARM = NEW_ALARM | 1;
	IS_ON = IS_ON | 1;
	Serial.println(NEW_ALARM);
}

