

# cascada-mqtt
2016-6-13
inherits code from demiot and cascada-webpack

## tags
### 03-generalizing_state
might still be a little cludgy. Sched::updateTimers checks for time left on timers and keeps them on if not zero so CMD::deserialize2 has to explicitly set them to zero.

### 02-messing_w_structs 
todo: retain the state of other senrels

rule: you can't set the value of a (complex)struct outside of a function. You need something like 

    void initState(){
      ste = {{0,80,70},{0,90,80},{1},{1},{1},1,0,0,1,0};
      ste.temp1.hilimit=85;
    }

this wemos is setup for 2 temp sensors and 3 timers;

    struct temp_t {
      bool state;
      int hilimit;
      int lolimit;
    };

    struct timr_t{
      bool state;
    };

    struct state_t{
      temp_t temp1;
      temp_t temp2;
      timr_t timr1;
      timr_t timr2;
      timr_t timr3;
      bool AUTOMA;
      bool sndStatus;
      bool sndSched;
      bool HAY_CNG;
      bool NEEDS_RESET;   
    };

Maybe all the structs should be in SETUP.h: state_t(temp_t,timr_t),ports, progs? eh

### 01-recover_from_failure
lesson#1: don't develop on esp8266 without compiling/uploading continuously. Contrary to internet consensus. Stack errors and crashes can happen easily from a change in application code and they are ridiculously hard to figure out.

toodo: 
* extend CYURDOO2/cmd to turn the thing on and off
* when reprogramming one senesor relay, keep existing programs on others

### 00-initial-commit
server running at sitebuilt.net 162.217.250.109 copy of server at /sbdev0/mqttserver. 
#### architecture
The demiot architecture is not general enough, particularly around scheduling. A more general setup might implement:
* as many as 6 relays and a ONE_WIRE bus
* something in setup() or processIncoming() that indicates what sensors and relays are being used for this device.
* some way to distinguish between schedule control, automatic control or just on/off
* a way to just update the daily program for one sensor/relay while leaving the others intact. 
* a way to go from manual control back to that days schedule

#### generalized setup

Each device needs a setup routine
* how many sensors&sched controlled relays and how many sched controlled relays
* their names

    PORTS po {5, 16, 15, 13, 12, 4, 14};
    //PORTS po {5, 16, 15, 13, 12, 4, 14};
    //{temp1, temp2, timr1, timr2, timr3 ,ds18b20}
    //{io5d1, io16d0, io15d8, io13d7, io12d6, io4d2, io14d5} wemos d1 mini
    STATE st {42, 38, 0, 82, 73, 1, 1, 0};
state

    {   senrels: [temp1, temp2, timr1, timr2], 
        [{name: "temp1", value: 68, state: 1, hilimit:84, lowlimit: 64, mode: 'auto/on/off/timed,  timeleft: 3456},
        {name: "temp2", value: 86, state: 0, hilimit:84, lowlimit: 64, mode: 'auto/on/off/timed', timeleft:3456},
        {name: "timr1", state: 1, mode: 'auto/on/off/timed', timeleft:3456},
        {name: "timr2", state: 2, mode: 'auto/on/off/timed', timeleft:3456}]
    }
setup

    {   senrels: ["temp1" "temp2", "timr1", "timr2"],
        relsen: [0,1],
        rel: [2,3,4],
        [{name: "temp1", oport: 4, iport: 5, progDataSz: 3, type: sentrel/rel},
        {name: "temp2", oport: 15, iport: 5, progDataSz: 3, type: sentrel/rel},
        {name: "timr1", oport: 13, progDataSz: 1, type: rel},
        {name: "timr2", oport: 15, progDataSz: 1, type: rel},
        {name: "timr3", oport: 12, progDataSz: 1, type: rel}]
    }    
    
    } temp2, heat, hilimit, lolimit, AUTOMA, HAY_CNG, NEEDS_RESET}
    TMR tmr {0,0,0,3,5,0};
    //timr1, timr2, timr3, numtmrs, crement, IS_ON 
    char * schedArr[]={"temp1","temp2","tmr1","tmr2","tmr3"};

    void setup(){
        getOnline()
        client.setServer(ip, 1883);
        client.setCallback(handleCallback); 
        mq.reconn(client); 
        req.stime();
        configPorts();
    }
    void loop(){
      server.handleClient();
      if(NEW_MAIL){
          processInc();
      }
      if(NEW_ALARM>0){ //set by bm callback or when new programs come in
        sched.actProgs2(tmr, st);
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
          sched.updateTmrs(tmr, client, st, po);
          publishTmr();
        }
      }
      if (inow - before > 1000) { //every second do the following 
        before = inow;
        if(st.AUTOMA){
          readTemps(); //or readSensors()
          controlHeat(); // or controlRelays()
        }
        if(st.HAY_CNG){
          //console.log("example console.log entry");
          publishState();
          st.HAY_CNG=0;
        }
      }              
    }

`bool Sched::deseriProgs(char* kstr){}` replaces daily programs for one or more sensors

`void Sched::actProgs2(TMR& tmr, STATE& st){]` 

`cmd`'s bypass the scheduling appparatus and directly set values of a senrel
`{"heat":1,"auto":1,"hilimit":80,"lolimit":74,"empty":0}` should mo better be

`{name: 'temp1', 'state':0, 'hilimit': 87, 'lolimit': 41}`
or 
`{name: 'timr1', state: 1}` that will serve to change until next program event
`{name: 'empty'}`
`{name: 'sndStatus'}`
`{name: 'sndSched'}`
`{name: 'auto', state: 1}`
or 
`{name:"timr1", ["state", "hilimit", "lolimit"], [0,84,46]}`
or
`{name"'temp1, [0, 84, 46]`
or 
`{id:1, darr: [0,84,45]}`
`{id:2, data: 1}`

    int id = rot["id"]
    if(rot["data"]){
      data = copy data
    }
    if(rot["flag"]){
      flag = rot["flag"]
    }
built in data structs
temp1.state
temp1.hilimit
temp1.lolimit
or





Are timers just as particular case of thermostat?
* a sensor&sched controlled relay changes state (activates a relay) based upon a combination of a sensor reading and the current parameters on what values cause the sensor to change state.
    `[0,0,78,54]`
    `hr min hilimit lowlimit`
* a sched relay changes state on schedule
    `[0,0,1]`
    `nr min on`



#### pinouts 
can't use
* io2d4 is wemos led and is used by eeprom synch upload of programs from pc.
* io0d3 won't connect to wifi is this pin is connected at startup
can use
* io5d1 is ALED, the output from temp1 to control a relay like a thermostat. 
* io4d2 is ONE_WIRE_BUS
* io16d0 seems open
* io14d5 seems open
* io12d6 seems open
* io13d7 seems open
* io15d8 seems open



