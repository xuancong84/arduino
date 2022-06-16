#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>


int readAvgVolt(int pin){
  long sum = 0;
  for(int x=0;x<8;x++)
    sum += analogRead(pin);
  return (int)((sum+4)/8);
}

int Vs[10];

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

  pinMode(A0, INPUT_PULLUP);
  analogReference(DEFAULT);

  digitalWrite(LED_BUILTIN, 1);
  
  Serial.begin(115200);
  Serial.print("System initialized, light sensor = ");
  Serial.println(readAvgVolt(A0));
  Serial.flush();
}

void loop(){
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

  // Timer2 interrupt in 4 sec
  cli();
  OCR2A = 125;
  TCNT2 = 0;
  TCCR2A = 0b00000010;
  TCCR2B = 0b00000111;
  TIMSK2 = 0b00000010;
  ASSR = 0b10100000;
  while(ASSR&0b00011111);
  sei();

  // sleep
  for(int x=0; x<10; x++){
    digitalWrite(LED_BUILTIN, x&1);
    Vs[x] = readAvgVolt(A0);
    SMCR = 0b00000111;
    sleep_cpu();
  }

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
  Serial.println("wake-up complete");
  Serial.flush();
  digitalWrite(LED_BUILTIN, 0);

  for(int x=0;x<10;x++)
    Serial.println(Vs[x]);
}

ISR(TIMER2_COMPA_vect){
  cli();
  // TIMSK2 = 0;
  sleep_disable();
  sei();
}

