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

  Serial.begin(115200, SERIAL_8N1);

  // setup ambient light sensor
  pinMode(A0, INPUT_PULLUP);
  analogReference(DEFAULT);

  Serial.println("Started:");
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
    delay(1000);
}