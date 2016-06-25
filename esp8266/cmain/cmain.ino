// // #include <Dhcp.h>
// // #include <Dns.h>
// #include <Ethernet.h>
// #include <EthernetClient.h>
// #include <EthernetServer.h>
// #include <EthernetUdp.h>

#include "config.h"
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "MQclient.h" //globals(extern) NEW_MAIL, itopic, ipayload
#include "STATE.h"
#include "PORTS.h"
#include "TMR.h"
#include "Cmd.h"
#include "Reqs.h"
#include "Sched.h"

PORTS po {5, 16, 15, 13, 12, 4, 14};
#define ONE_WIRE_BUS po.ds18b20 

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);
Console console(devid, client);
Reqs req(devid, client);
MQclient mq(devid);

int NEW_ALARM = -1;
int IS_ON = 0;
STATE st {2, 5, 42, 38, 0, 82, 73, 1, 1, 0};
//{IO2D4, ALED, temp1, temp2, heat, hilimit, lolimit, AUTOMA, HAY_CNG, NEEDS_RESET}
TMR tmr {0,0,0,3,5,0};
//timr1, timr2, timr3, numtmrs, crement, IS_ON
Sched sched;

state_t ste;
//ste = {{0,80,70},{0,90,80},{1},{1},{1},1,0,0,1,0};

void initState(){
  ste = {{0,80,70},{0,90,80},{0},{0},{0},1,0,1,0};
  ste.temp1.hilimit=85;
  ste.temp1 = {1,94,77};
}


const int numcmds = 3;
char incmd[][10]={"devtime", "progs", "cmd"};

void acb(){
  int i=0;
  switch(i){
    case 0:
      Serial.println("RING RING RING");
      Alarm.alarmOnce(hour(), minute()+1,0,acb);
  }
}

void processInc(){
  Serial.println(itopic);
  for (int i=0;i<numcmds;i++){
    if(strcmp(incmd[i], itopic)==0){
      switch (i){
        case 0:
          Serial.println("in devtime");
          sched.deserialize(ipayload);
          sched.actTime(st);
          break;            
        case 1:
          Serial.println("in progs");
          Serial.println(itopic);
          Serial.println(ipayload);
          sched.deseriProgs(ipayload);
          //sched.bootstrapSched();
          // for(int i = 0; i<sched.seresz; i++){
          //   if (sched.senrels[i]<99){
          //     int cur = 0;
          //     sched.resetAlarm(i, cur);
          //     sched.actProgs(i, cur, st, tmr);              
          //   }
          // }

          // Alarm.alarmOnce(hour(), minute()+1,0,acb);
          NEW_MAIL=0;
          NEW_ALARM=31;
          IS_ON=31;
          Alarm.clear();
          sched.actProgs2(tmr, ste);
          break;            
        case 2:
          Serial.println("in cmd");
          Cmd cmd;
          // cmd.deserialize(ipayload);
          // cmd.act(st);
          cmd.deserialize2(ipayload,ste, po, tmr);
          break; 
        default:           
          Serial.println("in default");
          break; 
      }
    }
  }
}

void publishState(){
  char astr[120];
  sprintf(astr, "{\"temp1\":%d, \"temp2\":%d, \"heat\":%d, \"hilimit\":%d, \"lolimit\":%d, \"auto\":%d}", st.temp1, st.temp2, st.heat, st.hilimit, st.lolimit, st.AUTOMA);
  char status[20];
  strcpy(status,devid);
  strcat(status,"/status");
  if (client.connected()){
    client.publish(status, astr, true);
  } 
  Serial.print(status);
  Serial.println(astr);
}

void publishTmr(){
  char astr[120];
  sprintf(astr, "{\"timr1\":%d, \"timr2\":%d, \"timr3\":%d, \"numtmrs\":%d, \"crement\":%d, \"IS_ON\":%d  }", tmr.timr1, tmr.timr2, tmr.timr3, tmr.numtmrs, tmr.crement, tmr.IS_ON);
  char status[20];
  strcpy(status,devid);
  strcat(status,"/tmr");
  if (client.connected()){
    client.publish(status, astr, true);
  } 
  // Serial.print(status);
  // Serial.println(astr);
}

void readTemps(){
  DS18B20.requestTemperatures(); 
  int temp1 = (int)DS18B20.getTempFByIndex(0);
  int temp2 = (int)DS18B20.getTempFByIndex(1);
  if(temp1 != st.temp1){
    st.temp1=temp1;
    st.HAY_CNG=1;
  }
  if(temp2 != st.temp2){
    st.temp2=temp2;
    st.HAY_CNG=1;
  }
}

void controlHeat(){
  bool heat = st.heat;
  if (st.temp1 >= st.hilimit){
    heat=0;
  } 
  if (st.temp1 <= st.lolimit){
    heat=1;
  } 
  if (heat != st.heat){
    st.heat = heat;
    digitalWrite(st.ALED, st.heat);
    st.HAY_CNG= 1;
  }
}

void cbtemp0(){
  Serial.println("triggered cbtemp0 callback");
}

void setup(){
  Serial.begin(115200);
  EEPROM.begin(512);
  Serial.println();
  Serial.println("--------------------------");
  Serial.println("ESP8266 mqttdemo");
  Serial.println("--------------------------");
  initState();
  Serial.print(ste.temp1.state);
  Serial.print(ste.temp1.hilimit);
  Serial.print(ste.temp1.lolimit);
  Serial.println(ste.temp2.hilimit);
  getOnline();
  client.setServer(ip, 1883);
  client.setCallback(handleCallback); 
  mq.reconn(client); 
  pinMode(po.temp1, OUTPUT);
  pinMode(po.temp2, OUTPUT);
  pinMode(po.timr1, OUTPUT);
  pinMode(po.timr2, OUTPUT);
  pinMode(po.timr3, OUTPUT);
  digitalWrite(po.temp1, st.heat);
  digitalWrite(po.timr3, LOW);
  req.stime();
  
  // Serial.println("should have time");
  // Serial.print(hour());
  // Serial.print(":"); 
  // Serial.println(minute()); 
  // Alarm.alarmOnce(hour(),minute()+1, 0, cbtemp0);
}



time_t before = 0;
time_t schedcrement = 0;
time_t inow;

void loop(){
  if(NEW_ALARM>0){
    sched.actProgs2(tmr, ste);
  }
  // if(NEW_ALARM>-1){
  //   int cur = 0;
  //   sched.resetAlarm(NEW_ALARM, cur);
  //   sched.actProgs(NEW_ALARM, cur, st, tmr);
  // }
  Alarm.delay(100);
  server.handleClient();
  if(NEW_MAIL){
    processInc();
  }
  if(!client.connected() && !NEEDS_RESET){
     mq.reconn(client);
  }else{
    client.loop();
  }
  inow = millis();
  if(inow-schedcrement > tmr.crement*1000){
    schedcrement = inow;
    if(IS_ON > 0){
      sched.updateTmrs(tmr, client, po);
      publishTmr();
    }
  }
  if (inow - before > 1000) {
    before = inow;
    if(st.AUTOMA){
      readTemps();
      controlHeat();
    }
    if(st.HAY_CNG){
      //console.log("example console.log entry");
      publishState();
      st.HAY_CNG=0;
    }
  } 
}