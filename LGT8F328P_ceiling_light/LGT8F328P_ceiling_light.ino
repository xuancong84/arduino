/*
 * This program is designed to work on LGT8F328P
 * for control ceiling light using HLK-HD1155D micro-motion sensor
 * Author: Wang Xuancong (2022.6)
 */

#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>

#define DEBUG 1
#define LIGHT_TH_LOW  1600
#define LIGHT_TH_HIGH 1800
#define DELAY_ON_MOV  30000
#define DELAY_ON_OCC  20000
#define OCC_TRIG_TH  2000
#define OCC_CONT_TH  0
#define MOV_TRIG_TH  0
#define MOV_CONT_TH  0
#define CHECK_INTERVAL  125   // N*0.032 sec

uint16_t adc_value;
bool is_light_on = false;
extern volatile unsigned long timer0_millis;

void tx_to_pd6(char *str){
  char old = PMX0;
  char new1 = old|2;
  PMX0 |= 0x80;
  PMX0 = new1;
  Serial.print(str);
  Serial.flush();
  PMX0 |= 0x80;
  PMX0 = old;
}

void sensor_on(){
  HDR = 1;
  digitalWrite(D5, 1);
}

void sensor_off(){
  digitalWrite(D5, 0);
  HDR = 0;
}

inline void light_on(){
  digitalWrite(D3, 1);
#ifdef DEBUG
  digitalWrite(LED_BUILTIN, 1);
  Serial.println("Light on");
  Serial.flush();
#endif
}

inline void light_off(){
  digitalWrite(D3, 0);
#ifdef DEBUG
  digitalWrite(LED_BUILTIN, 0);
  Serial.println("Light off");
  Serial.flush();
#endif
}

int readAvgVolt(int pin){
  long sum = 0;
  for(int x=0;x<8;x++)
    sum += analogRead(pin);
  return (int)((sum+4)/8);
}

void setup() {
  // 0. Initialize
  delay(200); // avoid soft brick
  
  // avoid floating ports
  for(int x=D2; x<=D13; x++){
    pinMode(x, OUTPUT);
    digitalWrite(x, 0);
  }
  for(int x=A0; x<=A7; x++){
    pinMode(x, OUTPUT);
    digitalWrite(x, 0);
  }

  // setup ambient light sensor
  pinMode(A0, INPUT_PULLUP);
  analogReference(DEFAULT);
  
  Serial.begin(115200);
  Serial.print("System initialized, light sensor = ");
  Serial.println(readAvgVolt(A0));
  Serial.flush();
}

ISR(TIMER2_COMPA_vect){
  cli();
  sleep_disable();
  sei();
}

int parse_output_value(String s){
  char posi = s.lastIndexOf(' ');
  if(posi<0)return 0;
  return s.substring(posi+1).toInt();
}

void loop() {
  // A. check light sensor
  if(readAvgVolt(A0)<LIGHT_TH_HIGH){
#ifdef DEBUG
        Serial.println("Entering sleep ...");
        Serial.flush();
#endif
    // enter slow clock
    char old_PMCR = PMCR;

    PMCR = 0b10000000;
    PMCR = 0b01010010;

    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    PRR = 0b10101110;
    PRR1 = 0b00101110;

    // setup Timer2 interrupt every 4 sec
    cli();
    OCR2A = CHECK_INTERVAL;
    TCNT2 = 0;
    TCCR2A = 0b00000010;
    TCCR2B = 0b00000111;
    TIMSK2 = 0b00000010;
    ASSR = 0b10100000;
    while(ASSR&0b00011111);
    sei();

    // sleep check
    int x=0;
    do{
      SMCR = 0b00000111;
      sleep_cpu();
#ifdef DEBUG
      digitalWrite(LED_BUILTIN, (++x)&1);
#endif
    }while(readAvgVolt(A0)<LIGHT_TH_HIGH);
#ifdef DEBUG
    digitalWrite(LED_BUILTIN, 0);
#endif

    // return to fast clock
    TIMSK2 = 0;
    PMCR = 0b10000000;
    PMCR = old_PMCR;

    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    PRR=PRR1=0;
#ifdef DEBUG
    Serial.println("Exited sleep ...");
    Serial.flush();
#endif    
  }


  // B. enter dark mode
  sensor_on();
  is_light_on = false;
  // reset timer0 counter
  noInterrupts ();
  timer0_millis = 0;
  interrupts ();
  delay(30000); // wait for sensor to stabilize
  while(Serial.available())
    Serial.readStringUntil('\n');

  unsigned long elapse = millis(), ul=0;
  while(1){
    if(is_light_on){  // when light is on
      if(Serial.available()){
        String s = Serial.readStringUntil('\n');
#ifdef DEBUG
        Serial.println(s);
#endif
        if(s.startsWith("mov") && parse_output_value(s)>=MOV_CONT_TH){
          ul = millis()+DELAY_ON_MOV;
        }else if(s.startsWith("occ") && parse_output_value(s)>=OCC_CONT_TH){
          ul = millis()+DELAY_ON_OCC;
        }
        if(ul>elapse)
          elapse = ul;
      }else if(millis()>elapse){
        light_off();
        is_light_on=false;
        delay(500); // wait for light sensor to stablize
      }
    }else{  // when light is off
      if(Serial.available()){
        String s = Serial.readStringUntil('\n');
#ifdef DEBUG
        Serial.println(s);
#endif
        if((s.startsWith("mov") && parse_output_value(s)>=MOV_TRIG_TH)
         || (s.startsWith("occ") && parse_output_value(s)>=OCC_TRIG_TH)){
          light_on();
          is_light_on=true;
          elapse = millis()+DELAY_ON_MOV;          
        }
      }else if(readAvgVolt(A0)<LIGHT_TH_LOW){
        sensor_off();
        delay(1000);
        break;
      }
    }
  }
}
