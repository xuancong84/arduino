/*
 * This program is designed to work on LGT8F328P
 * for control ceiling light using HLK-HD1155D micro-motion sensor
 * Author: Wang Xuancong (2022.6)
 */

#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>


bool state = false;

void setup() {
  // 0. Initialize
  delay(1000); // avoid soft brick

  // avoid floating ports
  for(int x=D2; x<=D13; x++){
    pinMode(x, OUTPUT);
    digitalWrite(x, 0);
  }
  for(int x=A0; x<=A7; x++){
    pinMode(x, OUTPUT);
    digitalWrite(x, 0);
  }

  // setup D4 as DAC output, use internal reference 2.048V
  pinMode(D4, INPUT);
  DACON = 0b00001100;

  Serial.begin(115200);
  Serial.println("\nSystem initialized, input interger between 0-255 for DAC output to D4 (-ve value to slide from 150 to it):");
  Serial.flush();
}

void smooth_to(int target) {
  if(target<150){
    analogWrite(DAC0, target);
    analogWrite(D3, target);
    analogWrite(D6, target);
  }else for(int x=150; x<target; ++x){
    analogWrite(DAC0, x);
    analogWrite(D3, x);
    analogWrite(D6, x);
    delay(x>>2);
  }
}

char str[512];
bool skip = false;
void loop() {
  if(Serial.available()){
    int val = Serial.parseInt();
    if(val>=255) val = 255;
    if(!skip){
      if(val<0){
        Serial.print("DAC value smooth from 150 to ");
        smooth_to(-val);
        Serial.println(-val);
      }else{
        analogWrite(DAC0, val);
        analogWrite(D3, val);
        analogWrite(D6, val);
        Serial.print("DAC value set to ");
        Serial.println(val);
      }
    }
    skip = !skip;
  } else
    delay(100);
}
