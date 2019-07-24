//HERE: need to ctr F forEach to replicate for each bus
//delthis: stuff for testing
//i think writing 0 in bytes is getting skipped by my other program...
//new plan DT covers three bins and each DT is in series with the others. Have a chain of commands.
  //receive >next and send >next
  //send <sensors
  //receive <sensors, add to running total. send <sensors
    //important to fill a buffer. pretty easy i'd say
    //have one byte for which of 3 bins, and two for which DT. up to 99. I'm using byte representatives, hew hew hew
    //still need to fill a 255 buffer. each sensor itself could just report 0-99.9 degrees celsius in 3 bytes.
    //so have 8 bytes ID, 3 bytes ID, additional 2 stamped on by the DT and one for the bus. Don't need an add algo then. kew kew kew. 1st 3 bytes is how full my thingo is. once it's full or done send it in. do everything serial
    //(3bytes for DT (1 byte for bus, (8 for ID, 3 for temp)*number of sensors )*3 buses) * n of DTs. cram everything you can in. first let's do this for my one bus
    //MAYBE have a command to call oneWire begin in the future
  //maybe a getALL for sum total from each DT. gotALL000001

//problem, i'm stuck in my while loop. need a way to wreck it. sumTotal + sensors just isnt' working. improper data type comparisons. 

//TIL
  //first write the program with functions that aren't filled in, then write the functions. 

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
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
SoftwareSerial away(6, 7);//which should be software?, can just wait for getALL. can also wait for gotALL right after. then I can wait for next.


//forEach

//For storing number of sensors on each bus. 
int sensorsONE;
int sensorsTWO;
int sensorsTHREE;
unsigned long sensors;//just to allow comparison

int bytesONE;
int bytesTWO;
int bytesTHREE;

unsigned long sumTotal; 
unsigned long upstream;


int DTposition;
unsigned long sensorsRecieved; //todo this needs a bit of work...hrmf

byte bufferToSend[260];
byte addr[8];
bool isReady = false;



void setup() {
  // put your setup code here, to run once:
  dsONE.begin();
  dsTWO.begin();
  dsTHREE.begin();
  Serial.begin(9600);
  away.begin(9600);
  Serial.write("Up");
}

void await_getALL(void){ //>getALL <sumTotal >next <sendInReads
  while(!Serial.find("getALL")){}; //wait for getALL
  //wait until getALL is found
  away.write("getALL");
  while(!away.find("gotALL")){};
  byte num[6];
  while(!away.available()){}; //todo improve
  for(int a = 0; a<6; a++){
    num[a] = away.read();
    watchdog(2000, false);
  };
  upstream = intoInt(num[0])*100000 + intoInt(num[1])*10000 + intoInt(num[2])*1000 + intoInt(num[3])*100 + intoInt(num[4])*10 + intoInt(num[5]);
  
  //get a count, divide by 21 and round up, save it to this device, sumTotal and relay, init reads. then compile 21 reads and await next
  sensorsONE = dsONE.getDeviceCount(); //somehow this is just set with begin()
  sensorsTWO = dsTWO.getDeviceCount(); //somehow this is just set with begin()
  sensorsTHREE = dsTHREE.getDeviceCount(); //somehow this is just set with begin()
  sensors = sensorsONE + sensorsTWO + sensorsTHREE;
  sumTotal = sensors + upstream;

  dsONE.requestTemperatures();
  dsTWO.requestTemperatures();
  dsTHREE.requestTemperatures();
  delay(750);
  
  Serial.write("gotALL");
  if(sumTotal<10){
    Serial.write("00000");
  }
  else if(sumTotal<100){
    Serial.write("0000");
  }
  else if(sumTotal<1000){
    Serial.write("000");
  }
  else if(sumTotal<10000){
    Serial.write("00");
  }
  else if(sumTotal<100000){
    Serial.write("0");
  };
  if(sumTotal <1000000){Serial.print(sumTotal);}
  else{Serial.write("9999999");};
  //forEach
  
}

void next(){
  //can I use goto
  //i guess I have to simplify. fuck it, who cares which DT or bus it's from. I'll go back to doing that host side. just make it 11 per sensor
  //at this moment i know bytes needed on this run. I think I'll make 2 different DTs. one for the end of the chain - this
  //good GOD. why can't buffer size be declared on teh go



  sensorsRecieved = 24;
  while(sensorsRecieved == 24){ //maybe use do... while so that if I get 23 it'll still search for next?
    searchForNext(true);//todo, need to write it from the DTendOfTheLine
    away.write("next");
    while(!away.available()){};
    away.readBytes(bufferToSend, 260);
    int howManyBytesIllBeWriting = bufferToSend[4] * 11 + 5;
    if(bufferToSend[4] == 24){Serial.write(bufferToSend, howManyBytesIllBeWriting);} //todo
    else if(bufferToSend[4] == 23){
      if(bufferToSend[4] + sensors > 23){
        bufferToSend[4] = 24;
      };
      Serial.write(bufferToSend, howManyBytesIllBeWriting);
      break;
    }
    else{
      if(bufferToSend[4] + sensors > 23){
        bufferToSend[4] = 24;
      }
      else{bufferToSend[4] = (bufferToSend[4] + sensors);};
      Serial.write(bufferToSend, howManyBytesIllBeWriting);
      break;
    };
  };
  for(int i=1; i<=sensors; i++){//this is the juicy beefcake function. need to come up with a better way for this. how can I just go through a bus until it's done? then move to the next... just need a linear search patter where start of array is 23*iteration;;;;feq
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
    if(((i+bufferToSend[4])%23) == 0){//todo i+sensorsRecieved%23?no. it should be a different variable in case it's 24 (but it never wuld be)
      searchForNext(true);
      //todo
      Serial.write("next");
      if(sensors - i >23){Serial.write(24);}
      else{Serial.write(sensors-i);};
    };
  };
  memset(bufferToSend, 0, 260);
  oneWireONE.reset_search();
  oneWireTWO.reset_search();
  oneWireTHREE.reset_search();
}//

void sendTempC(int Temp){
  if(Temp < 10){
    Serial.write("00");
    Serial.print(Temp);      
  }
  else if(Temp < 100){
    Serial.write("0");
    Serial.print(Temp);
  }
  else if(Temp < 1000) {
    Serial.print(Temp);
  } //reporting high temps
  else if(Temp >= 1000){
    Serial.write("999");
  };
}

bool searchForNext(bool serial) { //test
  if(serial){
    if(Serial.find("next")){
      while(Serial.available()){
        Serial.read();
      };
      return true;
    };
    watchdog(60000, true);
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
  else{
    while(!away.find("next")){};
    return true;
  };
}

long intoInt(byte byt){
    if(byt == '0'){return 0;};
    if(byt == '1'){return 1;};
    if(byt == '2'){return 2;};
    if(byt == '3'){return 3;};
    if(byt == '4'){return 4;};
    if(byt == '5'){return 5;};
    if(byt == '6'){return 6;};
    if(byt == '7'){return 7;};
    if(byt == '8'){return 8;};
    if(byt == '9'){return 9;};
}

void watchdog(unsigned long msWD, bool serial){
  unsigned long msStart = millis();
  if(serial){
    while(!Serial.available()){
      if((msStart + msWD)<millis()){
        break;
      };
    };
  }
  else{
    while(!away.available()){
      if((msStart + msWD)<millis()){
        break;
      };
    };
  };
}

void loop() {
  // put your main code here, to run repeatedly:
  await_getALL();
  next();
}
