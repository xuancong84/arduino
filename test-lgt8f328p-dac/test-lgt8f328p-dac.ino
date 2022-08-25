/*
 * This program is designed to work on LGT8F328P
 * for control ceiling light using HLK-HD1155D micro-motion sensor
 * Author: Wang Xuancong (2022.6)
 */

#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>

#define n_pins 20
int PINs[n_pins]={D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, A0, A1, A2, A3, A4, A5, A6, A7};
void setup() {
  // 0. Initialize
  delay(1000); // avoid soft brick

  // avoid floating ports
  for(int x=0; x<n_pins; x++){
    pinMode(PINs[x], OUTPUT);
    digitalWrite(PINs[x], 0);
  }

  // setup D4 as DAC output, use VCC as reference
  pinMode(D4, INPUT);
  DACON = 0b00001100;

  Serial.begin(115200);
  Serial.println("\nSystem initialized, input interger between 0-255 for analogWrite() to all ports!");
  Serial.println("DAC output to D4, PWM output to PD3 (490Hz) and PD5 (980Hz), digital output to PD2 (input ###-### to slide from ### to ###):");
  Serial.flush();
}

void smooth_to(int from, int to) {
  for(int val=from; val<=to; ++val){
    for(int x=0; x<n_pins; x++)
      analogWrite(PINs[x], val);
    delay(100);
  }
}

bool skip = false;
char str[256];
void loop() {
  if(Serial.available()){
    String s = Serial.readStringUntil('\n');
    s.trim();
    if(s.length()==0) return;
    int posi = s.indexOf('-');
    if(posi==-1){
      int val = s.toInt();
      for(int x=0; x<n_pins; x++)
        analogWrite(PINs[x], val);
      Serial.print("DAC value set to ");
      Serial.println(val);
    }else{
      int val1 = s.substring(0, posi).toInt();
      int val2 = s.substring(posi+1).toInt();
      sprintf(str, "Smoothing DAC value from %d to %d ...", val1, val2);
      Serial.print(str);
      smooth_to(val1, val2);
      Serial.println("Done");
    }
  } else
    delay(100);
}
