//HERE: need to ctr F forEach to replicate for each bus
//delthis: stuff for testing
//i think writing 0 in bytes is getting skipped by my other program...

//TIL
  //first write the program with functions that aren't filled in, then write the functions. 

#include <OneWire.h>
#include <DallasTemperature.h>
//#include <SoftwareSerial.h>
#define ONE 2
//forEACH

OneWire oneWireONE(ONE);
DallasTemperature dsONE(&oneWireONE);
//forEach

//For storing number of sensors on each bus. 
int sensorsONE;
int sensorsTWO;
int sensorsTHREE;
//forEach
//sensor iterations ONE ...
int siONE;
int siTWO;
int siTHREE;
//forEach
int sumTotal; 

byte bufferToSend[252];
byte addr[8];
bool isReady = false;



void setup() {
  // put your setup code here, to run once:
  dsONE.begin();
  Serial.begin(9600);
}

void await_getALL(void){ //>getALL <sumTotal >next <sendInReads
  while(!Serial.find("getALL")){}; //wait for getALL
  //wait until getALL is found
  //get a count, divide by 21 and round up, save it to this device, sumTotal and relay, init reads. then compile 21 reads and await next
  sensorsONE = dsONE.getDeviceCount(); //somehow this is just set with begin()
  siONE = sensorsONE/21;
  if((sensorsONE%21)>0){siONE++;};//just to round up.. shnizzle. what if it already did
  //forEach 
  sumTotal = siONE + siTWO + siTHREE; //beaut. this is a compiled total number fo iterations needed by this dt
  if(sumTotal<10){
    Serial.write("00");
  } //these functions just standardize to be sure first 3 bytes represent up to 999
  else if(sumTotal<100){Serial.write("0");};
  Serial.print(sumTotal); //maybe TODO: gaurd over rollover
  //thinking: particle sends next to early it gets caught in the buffer. no problem. so I'll compile a buffer to send then check with a while(!serial.av):
  dsONE.requestTemperatures();
  delay(750);
  //forEach
}

void next(){
    //next functionction that listens for next after compiling first read
  for(int unimportant=0; unimportant<siONE; unimportant++){ //HERE stress test
    for(int i=0; i<21; i++){ //do this up to 21 times to get 1 iterations
      //stop it if there is less than 21...whatever it works like a gershdernedt charm otherwise. except for being a broken loop.
        //if(lastIteration) {int cap = i<sensorsONE%21;}; newp. use a break function. if unimportant == siONE-1 && i == sensorsONE%21 ::: break;
      //need to get address, index, and temp. 8bytes, int int. do this 21 times if you can 
      //
      if((unimportant == (siONE - 1)) && (i == (sensorsONE%21))){ //"if it's the last iteration and we're done finding sensors"
        isReady = searchForNext();
        if(isReady){
          relay();
        };
        break;
      };
      oneWireONE.search(addr);
      int tempC = dsONE.getTempCByIndex(21*unimportant + i) * 10;
      for(int d=0; d<9; d++) {
        if(d<8){
          bufferToSend[(12*i)+d] = addr[d];
          //todo: memset as well
          //sol'n: for(i<siONE), nested iteration: for(i<21
        }
        else if(d==8){
          bufferToSend[(12*i)+d] = highByte(i);
          bufferToSend[(12*i)+d+1] = lowByte(i);
          bufferToSend[(12*i)+d+2] = highByte(tempC);
          bufferToSend[(12*i)+d+3] = lowByte(tempC);
        }
      }; //done one sensor
      if(i==20){
        isReady = searchForNext();
        if(isReady){
          relay();
        };
      };
    };
  };
  memset(bufferToSend, 0, 252);
  oneWireONE.reset_search(); // this is right
}

bool searchForNext(void) { //test
  if(Serial.find("next")){
    while(Serial.available()){
      Serial.read();
    };
    return true;
  };
  watchdog(60000);
  if(Serial.find("next")){
    while(Serial.available()){
      Serial.read();
    };
    return true;
  }
  else {
    return false;
  };
}

void relay(void){
  Serial.write("000");
  Serial.println(); //delthis
  for(int d=0; d<252; d++){
    Serial.print('-'); //delthis - print to write
    Serial.print(bufferToSend[d]);//todo - print to write
      //my problem is too many bytes of 0 (not"0") causes some sort of thingo to happen
  };
  Serial.println();//delthis
  isReady = false;
  memset(bufferToSend, 0, 252);
}

void watchdog(unsigned long msWD){
  unsigned long msStart = millis();
  while(!Serial.available()){
    if((msStart + msWD)<millis()){
      break;
    };
  };
}

void loop() {
  // put your main code here, to run repeatedly:
  await_getALL();
  next();
}
