/*
  OverrideSerialPassthrough sketch

  Although PIN0 and PIN1 are used for USB serial communication, if only DEV->UNO and UNO->PC are needed,
  (and UNO->DEV and PC->UNO are NOT needed) it is possible to connect DEV's TX pin to UNO's RX Pin 0,
  and perform the serial passthrough.
  
  by Wang Xuancong
*/

//#include <CustomSoftwareSerial.h>

//CustomSoftwareSerial Serial1(10, 11);

void setup() {
  Serial.begin(115200, SERIAL_8N1);
  //Serial1.begin(115200, CSERIAL_8N1);
  Serial.println("Custom software serial passthrough started on PIN 0 and PIN 1");
}

void loop() {
  if (Serial.available()) {      // If anything comes in Serial PIN 0
    Serial.write(Serial.read());   // read it and send it out Serial PIN 1
  }
  
//  if (Serial.available()) {      // If anything comes in Serial PIN 0 (PC USB)
//    Serial1.write(Serial.read());   // read it and send it out Serial1 PIN 11
//  }

//  if (Serial1.available()) {     // If anything comes in Serial1 PIN 10
//    Serial.write(Serial1.read());   // read it and send it out Serial PIN 1 (PC USB)
//  }
}
