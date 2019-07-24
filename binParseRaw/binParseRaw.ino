#include <AltSoftSerial.h>

AltSoftSerial binDT;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  binDT.begin(9600);
  Serial.print("binParseRaw is up and running");
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available()){
    binDT.print(Serial.read());
  }
  while(binDT.available()){
    Serial.print(binDT.read());
    Serial.print("-");
  }
}
