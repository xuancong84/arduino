/*
  OverrideSerialPassthrough sketch

  Although PIN0 and PIN1 are used for USB serial communication, if only DEV->UNO and UNO->PC are needed,
  (and UNO->DEV and PC->UNO are NOT needed) it is possible to connect DEV's TX pin to UNO's RX Pin 0,
  and perform the serial passthrough.
  
  by Wang Xuancong
*/

//#include <CustomSoftwareSerial.h>

//CustomSoftwareSerial Serial1(10, 11);

void sensor_on(){
  HDR = 1;
  digitalWrite(D5, 1);
}

void setup() {
  delay(200);
  
  // avoid floating ports
  for(int x=D2; x<=D13; x++){
    pinMode(x, OUTPUT);
    digitalWrite(x, 0);
  }
  for(int x=A0; x<=A7; x++){
    pinMode(x, OUTPUT);
    digitalWrite(x, 0);
  }

  Serial.begin(115200, SERIAL_8N1);
  Serial.println("Custom software serial passthrough started on PIN 0 and PIN 1");
  sensor_on();
}

int cnt=0;
void loop() {
  if(false){
    if (Serial.available()) {
      Serial.print(Serial.read(), HEX);
      Serial.print(" ");
      if(++cnt>=16){
        Serial.println("");
        cnt = 0;
      }
    }
  } else {
    if (Serial.available()) {      // If anything comes in Serial PIN 0
      Serial.write(Serial.read());   // read it and send it out Serial PIN 1
    }
  }
}
