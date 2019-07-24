//this is the greatest software ever made. newp. goofe der up eral good
///todo test


#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

SoftwareSerial binDT(3, 4);
OneWire ow(2);
DallasTemperature ds(&ow);

byte DTbuf[255];
byte Sbuf[255];
unsigned long mss;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  binDT.begin(9600);
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
}

void loop() {
  // put your main code here, to run repeatedly:
  start:
  Serial.println("binParse is waiting");
  while(!Serial.available()){
    
  };
  if(Serial.peek() == 'g'){ 
    if(Serial.find("getALL")){//TODO actually i think this flushes the buffer
      binDT.write("getALL");
      while(!binDT.available()){};
      while(binDT.available()){
        Serial.write(binDT.read());
        delay(100);
        };
      goto start;
    };
  };
  while(Serial.available()){
    binDT.write(Serial.read());
  };
  int a=0;
  mss = millis();
  while(!binDT.available()){
    if(millis()-mss >= 100000){break;};
  };
  Serial.print("this many sensors inbound: ");
  binDT.find("next");//getting rid of next, and the line below does the byte. if you write next, this should e first.
  Serial.println(binDT.read());
  while(binDT.available()){
    DTbuf[a] = binDT.read();
    mss = millis();
    while(!binDT.available()){
      if(millis() - mss >= 100000){break;};
    }
    a++;
    if(a==254){break;};
  };
  if(a>0){
    for(int b = 0; b<255; b++){
      if(b%11 == 0){
        Serial.println();
        Serial.print("The address is 0x");
      };
      if(b%11 < 8){
        if(DTbuf[b] < 16){
          Serial.print('0');
        };
        Serial.print(DTbuf[b], HEX);
      }
      else if(b%11 == 8){
        Serial.print(" TempC is ");
        Serial.write(DTbuf[b]);
      }
      else if(b%11 > 8){Serial.write(DTbuf[b]);};//halp clean er up
    };
  };
  a = 0;
}
