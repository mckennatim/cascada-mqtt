#include "Cmd.h" //private: heat, automa, hilimit, lolimit, empty
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "MQclient.h" //for extern NEW_MAIL
#include "STATE.h"

bool Cmd::deserialize(char* kstr){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(kstr);
  heat = root["heat"];
  automa = root["auto"];
  hilimit = root["hilimit"];
  lolimit = root["lolimit"];
  empty = root["empty"];  
  return root.success();
}

void Cmd::act(STATE& st){
  char cmdArr[][15] = {"heat", "automa", "hilimit", "lolimit", "empty"};
  for(int i=0;i<5;i++){
    if(heat != st.heat){
      st.heat = heat;
      digitalWrite(st.ALED, st.heat);
      st.HAY_CNG=1;
    }
    if(automa != st.AUTOMA){
      st.AUTOMA = automa;
      st.HAY_CNG=1;
    }
    if(hilimit > 0 && hilimit != st.hilimit){
      st.hilimit = hilimit;
      st.HAY_CNG=1;
    }
    if(lolimit > 0 && lolimit != st.lolimit){
      st.lolimit = lolimit;
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