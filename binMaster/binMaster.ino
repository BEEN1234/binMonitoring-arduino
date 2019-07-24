//binMaster is a state machine
//binHub is a stateless machine
//...
//binMaster must call the slave, command, and iteration.

#include <AltSoftSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

AltSoftSerial serialDownstream;
OneWire ow(2);
DallasTemperature ds(&ow);

byte DTbuf[255];
byte Sbuf[255];
unsigned long mss;

bool debugPrint = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  serialDownstream.begin(9600);
  ds.begin();
  if(ds.getDeviceCount()){
    byte addr[8];
    ds.requestTemperatures();
    delay(750);
    ow.search(addr);
    Serial.print("0x");
    for(int i = 0; i<8; i++){
      if(addr[i] < 16){Serial.write('0');};
      Serial.print(addr[i], HEX);
    };
    int Temp = ds.getTempCByIndex(0) * 10;
    Serial.print(" and TempC is ");
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
    Serial.println();
    ow.reset_search();
  };
  
  Serial.write("binMaster. debugPrint?Y/N");
  if(Serial.find('Y')){debugPrint = true;};

  Serial.println("input 'start' to start");
}

void start(bool Print, unsigned long timeToWait){
  serialDownstream.write((byte)0xFF);
  serialDownstream.print("st");
  serialDownstream.write((byte)0);

  unsigned long timeNow = millis();


  while(millis() < timeNow + timeToWait){
    if(Print){Serial.println("start function waiting");};//somehow this makes or breaks.
    while(serialDownstream.available()){
      if('s' == serialDownstream.read() && 't' == serialDownstream.read()){ //todo buggy
        byte id = serialDownstream.read();
        //Print
        Serial.print("binHub-");
        Serial.print(id);
        Serial.println(" found.");
      }
      timeNow = millis();
    }
  }
}




void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.find("start")){//todo: bad code, flushes buffer
    Serial.println("in start");
    start(debugPrint, 500);
  }
}
