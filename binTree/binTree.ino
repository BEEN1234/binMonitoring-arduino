//everytime I add sensors it'll be done with a command and a protocol. number of sensors should be known. by the thingo as well. 
//hardCode in the UART
#include <SoftwareSerial.h>

SoftwareSerial UART1(4,5);
int UART1numb;

int UART2numb;

int UART3numb;
//TODO: this for each as well as howMany and Setup. relay will use uartnum


//these are for telling the electron how many sensors there are to read
byte byt1;
byte byt2;
byte byt3;

byte relayBuffer[255];

void toBytes(unsigned long num){
  byt1 = ((num - (num%(256*256))) / (256*256));
  byt2 = ((num - byt1*256*256) - ((num - byt1*256*256)%256)/256 );
  byt3 = num%(256*256);
};

unsigned long toLong(byte b, byte bb, byte bbb){
  return b*256*256 + bb * 256 + bbb; 
};

void howMany(){
  //each controller gets a message and responds with 3 bytes, all get added together
  byte sumTotal;

  byte UART1raw[3];
  unsigned long mssStart = millis();
  UART1.print('getINDEX');
  while(!UART1.available()){
    if (mssStart + 60000 < millis()){break;};
    };
  UART1.readBytes(UART1raw, 3);
  UART1numb = toLong(UART1raw[0], UART1raw[1], UART1raw[2]);

  //same for others and adding to a running total

  sumTotal = UART1numb;

  toBytes(sumTotal);
  Serial.write(byt1);
  Serial.write(byt2);
  Serial.write(byt3);
  
};

void relay(){
  unsigned long mssStart;
  int iterations;
  if(UART1numb > 0){
    UART1.print('getALL');
    int cont = 1;
    while(cont == 1){
      mssStart = millis();
      while(!UART1.available()){
        if(mssStart + 5*60*1000 < millis()){break};
      };
      UART1.readBytes(relayBuffer, 256);
      Serial.write(relayBuffer);
      if(sizeof(relayBuffer) == 256){cont = 1}
      else {cont = 0};
    }
  };
  if(UART2numb > 0){

  };
  if(UART3numb > 0){

  };
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  UART1.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(!Serial.available()){};
  Serial.find('getALL');
  howMany();
  relay();
}
