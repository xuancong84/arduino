#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>

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

  pinMode(PC6, INPUT_PULLUP);
  PMX2 |= 0b10000000;
  PMX2 |= 1;
  PCICR |= (1<<PCIE1);
  PCMSK1 |= (1<<PCINT14);

  Serial.begin(115200, SERIAL_8N1);

  // setup ambient light sensor
  pinMode(A0, INPUT_PULLUP);
  analogReference(DEFAULT);

  Serial.println("Started:");
}

bool DEBUG = false;
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
    DEBUG = !DEBUG;
  }
  sei();
}

void loop(){
    int val = readAvgVolt(A0);
    Serial.println(val);
    if(val>2000){
      digitalWrite(D3, 1);
      digitalWrite(LED_BUILTIN, 1);
    }else{
      digitalWrite(D3, 0);
      digitalWrite(LED_BUILTIN, 0);
    }
    Serial.println(DEBUG?"DEBUG is on":"DEBUG is off");
    delay(1000);
}