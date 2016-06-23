#include "Cmd.h" //private: heat, automa, hilimit, lolimit, empty
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "MQclient.h" //for extern NEW_MAIL
#include "STATE.h"
#include "PORTS.h"

//[CYURD003/cmd {temp1:{relay:0,hilimit:88,lolimit:65},
//temp2:{relay:0,hilimit:88,lolimit:65},
//timr1:{relay:0},timr2:{relay:0},timr3:{relay:0},empty:0,auto:1}
// bool Cmd::deserialize(char* kstr){
//   StaticJsonBuffer<200> jsonBuffer;
//   JsonObject& root = jsonBuffer.parseObject(kstr);
//   heat = root["heat"];
//   automa = root["auto"];
//   hilimit = root["hilimit"];
//   lolimit = root["lolimit"];
//   empty = root["empty"];  
//   return root.success();
// }
bool Cmd::deserialize(char* kstr){
  DynamicJsonBuffer jsonBuffer;
  // StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(kstr);
  temp1r = root["temp1"]["relay"];
  temp1hi = root["temp1"]["hilimit"];
  temp1lo = root["temp1"]["lolimit"];
  temp2r = root["temp2"]["relay"];
  temp2hi = root["temp2"]["hilimit"];
  temp2lo = root["temp2"]["lolimit"];
  timr1r = root["timr1"]["relay"];
  timr2r = root["timr2"]["relay"];
  timr3r = root["timr3"]["relay"];
  automa = root["auto"];
  empty = root["empty"];  
  return root.success();
}

void Cmd::act(STATE& st, PORTS& po){
  //char cmdArr[][15] = {"heat", "automa", "hilimit", "lolimit", "empty"};
  for(int i=0;i<5;i++){
    if(temp1r != st.temp1r){
      st.temp1r = temp1r;
      digitalWrite(po.temp1, st.temp1r);
      st.HAY_CNG=1;
    }
    if(temp1hi > 0 && temp1hi != st.temp1hi){
      st.temp1hi = temp1hi;
      st.HAY_CNG=1;
    }
    if(temp1lo > 0 && temp1lo != st.temp1lo){
      st.temp1lo = temp1lo;
      st.HAY_CNG=1;
    }
    if(temp2r != st.temp2r){
      st.temp2r = temp2r;
      digitalWrite(po.temp2, st.temp2r);
      st.HAY_CNG=1;
    }
    if(temp2hi > 0 && temp2hi != st.temp2hi){
      st.temp2hi = temp2hi;
      st.HAY_CNG=1;
    }
    if(temp2lo > 0 && temp2lo != st.temp2lo){
      st.temp2lo = temp2lo;
      st.HAY_CNG=1;
    }
    if(timr1r != st.timr1r){
      st.timr1r = timr1r;
      digitalWrite(po.timr1, st.timr1r);
      st.HAY_CNG=1;
    }
    if(timr2r != st.timr2r){
      st.timr2r = timr2r;
      digitalWrite(po.timr2, st.timr2r);
      st.HAY_CNG=1;
    }
    if(timr3r != st.timr3r){
      st.timr3r = timr3r;
      digitalWrite(po.timr3, st.timr3r);
      st.HAY_CNG=1;
    }
    if(automa != st.AUTOMA){
      st.AUTOMA = automa;
      st.HAY_CNG=1;
    }
    if (empty==1){
      eraseConfig();
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      delay(100);  
      st.NEEDS_RESET=1;    
    }  
    NEW_MAIL=0;    
  } 	
}