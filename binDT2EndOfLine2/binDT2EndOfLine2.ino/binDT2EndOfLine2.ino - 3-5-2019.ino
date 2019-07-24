//uploaded 7:10 pm 03/05/2019

//HERE: need to ctr F forEach to replicate for each bus
//delthis: stuff for testing that i need to delete
//i think writing 0 in bytes is getting skipped by my other program...
//new plan DT covers three bins and each DT is in series with the others. Have a chain of commands.
  //receive >next and send >next
  //send <sensors
  //receive <sensors, add to running total. send <sensors
    //important to fill a buffer. pretty easy i'd say
    //have one byte for whichnext of 3 bins, and two for which DT. up to 99. I'm using byte representatives, hew hew hew
    //still need to fill a 255 buffer. each sensor itself could just report 0-99.9 degrees celsius in 3 bytes.
    //so have 8 bytes ID, 3 bytes ID, additional 2 stamped on by the DT and one for the bus. Don't need an add algo then. kew kew kew. 1st 3 bytes is how full my thingo is. once it's full or done send it in. do everything serial
    //(3bytes for DT (1 byte for bus, (8 for ID, 3 for temp)*number of sensors )*3 buses) * n of DTs. cram everything you can in. first let's do this for my one bus
    //MAYBE have a command to call oneWire begin in the future
  //maybe a getALL for sum total from each DT. gotALL000001

//TIL
  //first write the program with functions that aren't filled in, then write the functions. 

//break time - searchfornext is a bool operator. not treating it like that. watchdog has a stupid ass serial.unavailable()locked while loop...

#include <OneWire.h>
#include <DallasTemperature.h>
//#include <SoftwareSerial.h>
#define ONE 2
#define TWO 3
#define THREE 4
//forEACH

OneWire oneWireONE(ONE);
OneWire oneWireTWO(TWO);
OneWire oneWireTHREE(THREE);
DallasTemperature dsONE(&oneWireONE);
DallasTemperature dsTWO(&oneWireTWO);
DallasTemperature dsTHREE(&oneWireTHREE);

//forEach

//For storing number of sensors on each bus. 
int sensorsONE;
int sensorsTWO;
int sensorsTHREE;
int sensors;

int bytesONE;
int bytesTWO;
int bytesTHREE;

int sumTotal; 

int DTposition;
int bytesWritten;

byte bufferToSend[255];
byte addr[8];
bool isReady = false;



void setup() {
  // put your setup code here, to run once:
  dsONE.begin();
  dsTWO.begin();
  dsTHREE.begin();
  Serial.begin(9600);
}

void await_getALL(void){ //>getALL <sumTotal >next <sendInReads
  while(!Serial.find("getALL")){}; //wait for getALL
  //wait until getALL is found
  //get a count, divide by 21 and round up, save it to this device, sumTotal and relay, init reads. then compile 21 reads and await next
  sensorsONE = dsONE.getDeviceCount(); //somehow this is just set with begin()
  sensorsTWO = dsTWO.getDeviceCount(); //somehow this is just set with begin()
  sensorsTHREE = dsTHREE.getDeviceCount(); //somehow this is just set with begin()
  sensors = sensorsONE + sensorsTWO + sensorsTHREE;

  Serial.print("gotALL");
  if(sensors<10){
    Serial.print("00000");
  }
  else if(sensors<100){
    Serial.print("0000");
  }
  else if(sensors<1000){
    Serial.print("000");
  }
  else if(sensors<10000){
    Serial.print("00");
  }
  else if(sensors<100000){
    Serial.print("0");
  };
  if(sensors <1000000){Serial.print(sensors);}
  else{Serial.print("9999999");};
  
  dsONE.requestTemperatures();
  dsTWO.requestTemperatures();
  dsTHREE.requestTemperatures();
  delay(750);
  //forEach
  
}

void next(){
  //can I use goto
  //i guess I have to simplify. fuck it, who cares which DT or bus it's from. I'll go back to doing that host side. just make it 11 per sensor
  //at this moment i know bytes needed on this run. I think I'll make 2 different DTs. one for the end of the chain - this
  //good GOD. why can't buffer size be declared on teh go
  
  searchForNext();
  Serial.print("next");
  if(sensors>23){Serial.write(24);}
  else{
    byte byteToSend=0;
    byteToSend += sensors; //test this...
    Serial.write(byteToSend); //todo get 18 here
  }
  for(int i=1; i<=sensors; i++){
    if(i <= sensorsONE){ //filling the buffer like this is kind of stupid. I think I'll just send it.
      oneWireONE.search(addr);
      for(int b = 0; b<8; b++){
        Serial.write(addr[b]);
      };
      int tempC = dsONE.getTempCByIndex(i-1) * 10;
      sendTempC(tempC);
    }
    else if(i <= (sensorsONE + sensorsTWO) && sensorsTWO>0){
      oneWireTWO.search(addr);
      for(int b = 0; b<8; b++){
        Serial.write(addr[b]);
      };
      int tempC = dsTWO.getTempCByIndex(i-sensorsONE-1) * 10;
      sendTempC(tempC);
    }
    else if(sensorsTHREE){
      oneWireTWO.search(addr);
      for(int b = 0; b<8; b++){
        Serial.write(addr[b]);
      };
      int tempC = dsTWO.getTempCByIndex(i-(sensorsONE+sensorsTWO+1)) * 10;
      sendTempC(tempC);
    };
    if((i%23) == 0){
      searchForNext();
      Serial.print("next");
      if((sensors - i)>23){Serial.write(24);}
      else{Serial.write(sensors - i);};
    };
  };
  oneWireONE.reset_search();
  oneWireTWO.reset_search();
  oneWireTHREE.reset_search();
}

void sendTempC(int Temp){
  if(Temp < 10){
    Serial.print("00");
    Serial.print(Temp);      
  }
  else if(Temp < 100){
    Serial.print("0");
    Serial.print(Temp);
  }
  else if(Temp < 1000) {
    Serial.print(Temp);
  } //reporting high temps
  else if(Temp >= 1000){
    Serial.print("999");
  };
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
