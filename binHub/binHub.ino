//important:: buggy/todo: whenever i receive I need to deal with it immediately and memset the buffer

//todo
  //reset teh Ow in start
  //put in mosfets
  //long range comms
  //better 

//it worked with 3 wires... maybe a laboured thingo maboo
  //MAYBEEEE... not enough V... the whole short thing di'n't work.

#include <AltSoftSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE 2
#define TWO 3
#define THREE 4
#define shortONE 11 //so when this worked it returned disconnected, and when it didn't work it gave a sorta right answer. . . hrm hrm hrm. unlikely that it's a bad plug. 
//forEACH

OneWire oneWireONE(ONE);
OneWire oneWireTWO(TWO);
OneWire oneWireTHREE(THREE);
DallasTemperature dsONE(&oneWireONE);
DallasTemperature dsTWO(&oneWireTWO);
DallasTemperature dsTHREE(&oneWireTHREE);
AltSoftSerial serialDownstream;

bool debugPrint = false;

#define serialReadWindow 50//how long it waits for the next character

//number of sensors on each bus
byte sensorsONE;
byte sensorsTWO;
byte sensorsTHREE;
byte sensors;

byte binHubPosition = 0;

#define sensorsPerIteration 13 //reset this if I change the overall architecture of my sexy sexy code

byte receiveBuffer[260];
byte bufferToSend[260]; //not sure if I'll use this
byte addr[8];

void setup() {
  // put your setup code here, to run once:
  dsONE.begin();
  dsTWO.begin();
  dsTHREE.begin();
  Serial.begin(9600);
  serialDownstream.begin(9600);
  Serial.write("binHub. debugPrint?Y/N");
  if(Serial.find('Y')){
    debugPrint = true;
    Serial.print("ok");
  };
}

int receiveDownStream(){
  memset(receiveBuffer,0,260);
  int counter = 0;
  while(serialDownstream.available()){
    receiveBuffer[counter] = serialDownstream.read();
    counter++;
    unsigned long timeNow = millis();
    while(!serialDownstream.available()){
      if(millis() > timeNow + serialReadWindow){break;};
    };
  };
  return counter;
}

void sendCommandUpstream(byte command, byte binPos, byte argument){
  Serial.write('<');
  Serial.write(command);  
  Serial.write(binPos);  
  Serial.write(argument);  
}

void sendCommandDownstream(byte command, byte binPos, byte argument){
  serialDownstream.write('>');
  serialDownstream.write(command);  
  serialDownstream.write(binPos);  
  serialDownstream.write(argument);
}

byte searchForCommand(){
  byte command = 0;
  byte binHubPositionOfIntendedDevice = 0;
  memset(receiveBuffer, 0, 260);
  if(Serial.read() == '>'){
    Serial.readBytes(receiveBuffer, 3);
    command = receiveBuffer[0];
    binHubPositionOfIntendedDevice = receiveBuffer[1];
  };
//buggy todo this assumes ONLY commands come from Serial.
  if(binHubPositionOfIntendedDevice == binHubPosition){
    if(debugPrint){Serial.print("debug: intended id");};
    return command;
  }
  else if(binHubPositionOfIntendedDevice == 0xFF){ //0xFF means all devices. The responsibility is on the called function to pass downstream. as of march7th/2019 only 's' and 'q'. 
    if(debugPrint){Serial.print("debug: 0xFF");};
    return command;
  }
  else{
    if(debugPrint){Serial.print("degub: not ID, pass");};
    sendCommandDownstream(command, binHubPositionOfIntendedDevice, receiveBuffer[2]);
    return 0;
  };
}

//execute command - > search strings and do what's required

void start(byte positionArgument){
  //get previous binHubPosition, add 1 adn send up and downstream.
  binHubPosition = positionArgument + 1;
  sendCommandUpstream('s', binHubPosition, receiveBuffer[2]);
  Serial.write(binHubPosition);//hrm
  sensorsONE = dsONE.getDeviceCount(); //somehow this is just set with begin()
  sensorsTWO = dsTWO.getDeviceCount(); //somehow this is just set with begin()
  sensorsTHREE = dsTHREE.getDeviceCount(); //somehow this is just set with begin()...
  sensors = sensorsONE + sensorsTWO + sensorsTHREE;
  if(debugPrint){
    Serial.println();
    Serial.println("debugLog");
    Serial.print("sensorsONE: ");
    Serial.println(sensorsONE);
    Serial.print("sensorsTWO: ");
    Serial.println(sensorsTWO);
    Serial.print("sensorsTHREE: ");
    Serial.println(sensorsTHREE);
    Serial.print("sensors: ");
    Serial.println(sensors);
    Serial.println("/debugLog");
  }

  sendCommandDownstream('s', 0xFF, binHubPosition);
}

void relay(){
  memset(receiveBuffer, 0, 260);
  int counter = receiveDownStream();
  for(int i; i<counter; i++){
    Serial.write(receiveBuffer[i]);
  }
  memset(receiveBuffer, 0, 260);
}

void queueReads(){
  dsONE.requestTemperatures();
  dsTWO.requestTemperatures();
  dsTHREE.requestTemperatures();
  //todo write short high for 750 ms + 10 ms while conv and data transfer takes place

  sendCommandDownstream('q', 0xFF, binHubPosition);
  
  if(debugPrint){Serial.print("debug: 'q'");};
  //the master should wait 1 sec before requesting: buggy todo
  return; 
}

void getReads(byte iteration){
  int startPoint = (iteration*sensorsPerIteration)+1;
  int endPoint = startPoint + sensorsPerIteration - 1;
  
  int sensorsRemaining = sensors - iteration*sensorsPerIteration;
  if(sensorsRemaining < sensorsPerIteration){endPoint = startPoint + sensorsRemaining - 1;};

  if(debugPrint){
    Serial.println("debug: 'g'");
    Serial.print("start: ");
    Serial.println(startPoint);
    Serial.print("end: ");
    Serial.println(endPoint);  
  };
  
  sendCommandUpstream('g', binHubPosition, receiveBuffer[2]);
  Serial.write(sensors);
  
  for(int i=startPoint; i<=endPoint; i++){
    //refitted from DT2: changes: 
      //somehow use iteration to establish start point, and end point is just iteration + 23
    if(i <= sensorsONE){ //filling the buffer like this is kind of stupid. I think I'll just send it.
      oneWireONE.search(addr);//todo 10ms shortHigh
      pinMode(shortONE, OUTPUT);
      digitalWrite(shortONE, HIGH);//here
      delay(10);
      digitalWrite(shortONE, LOW);
      pinMode(shortONE, INPUT);
      for(int b = 0; b<8; b++){
        if(addr[b]<16){Serial.print(0);};
        Serial.print(addr[b], HEX);
      };
      int tempC = dsONE.getTempCByIndex(i-1) * 10;
      sendTempC(tempC);
    }
    else if(i <= (sensorsONE + sensorsTWO) && sensorsTWO>0){
      oneWireTWO.search(addr);
      for(int b = 0; b<8; b++){
        if(addr[b]<16){Serial.print(0);};
        Serial.print(addr[b], HEX);
      };
      int tempC = dsTWO.getTempCByIndex(i-sensorsONE-1) * 10;
      sendTempC(tempC);
    }
    else if(sensorsTHREE){
      oneWireTHREE.search(addr);
      for(int b = 0; b<8; b++){
        if(addr[b]<16){Serial.print(0);};
        Serial.print(addr[b], HEX);
      };
      int tempC = dsTHREE.getTempCByIndex(i-(sensorsONE+sensorsTWO+1)) * 10;
      sendTempC(tempC);
    };

  };
}

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

void loop() {
  //getCommand and redirect
  byte command = 0;
  if(Serial.available()){//fixme -- if the first character isn't a > then this loops around
    command = searchForCommand(); //the function should read CIP into receive buffer. (and only  CIP). if ID is right, or FF then return command. otherwise serialDownstream.write. 
  };
  if(command>0){
    switch(command){
      case 's':
        start(receiveBuffer[2]);
        break;
      case 'q':
        queueReads();
        break;
      case 'g':
        getReads(receiveBuffer[2]);
        break;
      default:
        Serial.print("err");
        break;
    }
  };

  //reset
  command = 0;
  memset(receiveBuffer, 0, 260);

  //send data from other sensor up the line
  if(serialDownstream.available()){
    relay();
  };
}
