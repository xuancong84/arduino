/*
 * This program is designed to work on LGT8F328P
 * for control ceiling light using HLK-HD1155D micro-motion sensor
 * Author: Wang Xuancong (2022.6)
 */

#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>

#define LIGHT_TH_LOW  3000
#define LIGHT_TH_HIGH 3200
#define DELAY_ON_MOV  30000
#define DELAY_ON_OCC  20000
#define OCC_TRIG_TH  65530
#define OCC_CONT_TH  500
#define MOV_TRIG_TH  500
#define MOV_CONT_TH  250
#define CHECK_INTERVAL  125   // N*0.032 sec

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

  // reuse RESET button as toggle DEBUG mode
  pinMode(PC6, INPUT_PULLUP);
  PMX2 |= 0b10000000;
  PMX2 |= 1;
  PCICR |= (1<<PCIE1);
  PCMSK1 |= (1<<PCINT14);

  Serial.begin(115200);
  Serial.println("\nSystem initialized:");
  Serial.println("Press RESET button to toggle all pins' voltages.");
  Serial.flush();
}

char pc_int_cnt = 0;
unsigned long last_press = 0;
ISR(PCINT1_vect){
  cli();
  if((++pc_int_cnt)&1)
    last_press = millis();
  else{
    if(millis()-last_press>1000){  // long-click restore RESET button
      PCICR &= ~(1<<PCIE1);
      PCMSK1 &= ~(1<<PCINT14);
      PMX2 |= 0b10000000;
      PMX2 &= ~1;
      pinMode(PC6, OUTPUT);
    }
    state = !state;
    for(int x=D2; x<=D13; x++)
      digitalWrite(x, state);
    for(int x=A0; x<=A7; x++)
      digitalWrite(x, state);
  }
  sei();
}

void loop() {
  delay(1000);
}
