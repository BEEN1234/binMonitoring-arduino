#include <AltSoftSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

AltSoftSerial binDT;
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
    for(int i = 0; i<8; i++){
      if(addr[i] < 16){Serial.write('0');};
      Serial.print(addr[i], HEX);
    };
    int Temp = ds.getTempCByIndex(0) * 10;
    ow.reset_search();
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

void processBinComs(){
  for(int i = 0; i<255; i++){
    if(DTbuf[i] == 0x28 && DTbuf[i+1] == 0xFF){
      Serial.println();
      Serial.print("0x");
      for(int j = 0; j<8; j++){
        if(DTbuf[i+j]<16){Serial.print("0");};
        Serial.print(DTbuf[i + j], HEX);
      }
      Serial.print(" ");
      i = i + 8; //test
    };
    Serial.write(DTbuf[i]); //for some reason this writes gibberish for the last letters of words.
  };
  Serial.println();
  memset(DTbuf, 0, 255);
}

void loop() {
  // put your main code here, to run repeatedly:
  //a very simple task here -> write what you read. print 28FF and all after bytes in hex
  //how do i want to line break -> at the start of every 0x28FF...sure
  //
  
  while(!Serial.available()){};
  delay(20);

  Serial.print("input: ");
  while(Serial.available()){
    byte b = Serial.read();
    binDT.write(b);
    Serial.write(b);
  };

  Serial.println("waiting for binDT");
  
  while(!binDT.available()){};
  delay(20);

  Serial.println("done waiting for binDT. Reading then processing...");

  int counter = 0;
  while(binDT.available()){
    DTbuf[counter] = binDT.read();
    counter++;
    unsigned long msStart = millis();
    while(!binDT.available()){
      if((msStart + 1000)<millis()){
        break;
      };
    };
  };

  processBinComs();
}
