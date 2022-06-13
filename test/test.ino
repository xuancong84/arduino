#define NOP __asm__ __volatile__ ("nop\n\t")
#include <WDT.h>
#include <avr/sleep.h>


int readAvgVolt(int pin){
  long sum = 0;
  for(int x=0;x<16;x++)
    sum += analogRead(pin);
  return (int)((sum+8)/16);
}

void setup() {
  delay(200);
  pinMode(A0, INPUT_PULLUP);
  analogReference(DEFAULT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200, SERIAL_8N1);
  Serial.println("Initialization complete:");
  Serial.flush();

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

  PRR = 0b11101110;
  PRR1 = 0x2e;

  uint16_t Vs[10];
  for(int x=0;x<10;x++){
    Vs[x] = readAvgVolt(A0);
    digitalWrite(LED_BUILTIN, 1);
    for(int y=0;y<5000;y++)NOP;
    digitalWrite(LED_BUILTIN, 0);
    for(int y=0;y<5000;y++)NOP;
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

  for(int x=0;x<10;x++)
    Serial.println(Vs[x]);

}


void loop() {
  digitalWrite(LED_BUILTIN, 1);
  delay(250);
  digitalWrite(LED_BUILTIN, 0);
  delay(250);
  Serial.println(readAvgVolt(A0));
  // put your main code here, to run repeatedly:
//  wdt_enable(WDTO_4S);
//  Serial.println(WDTCSR, BIN);
//  Serial.flush();
//  WDTCSR = 0b01000110;
//  wdt_reset();
//  PRR = 0xef;
//  PRR1 = 0x0e;
//  SMCR = 0b00001101;
//  sleep_cpu();
//  PRR = PRR1 = 0;
//  sleep_disable();
//  SMCR = 0;
//  wdt_reset();
//  wdt_disable();
//
//  digitalWrite(LED_BUILTIN,1);
//
//  while(1);
}
