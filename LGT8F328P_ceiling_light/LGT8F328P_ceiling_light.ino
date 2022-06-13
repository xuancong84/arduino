#include <WDT.h>
#include <avr/sleep.h>

#define LIGHT_TH_LOW  1600
#define LIGHT_TH_HIGH 1700

uint16_t adc_value;

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

int readAvgVolt(int pin){
  long sum = 0;
  for(int x=0;x<16;x++)
    sum += analogRead(pin);
  return (int)((sum+8)/16);
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

}

void loop() {
  // upon startup, check light condition
  wdt_enable(WTO_16S);
  SMCR = 0b00000011;
  sleep_cpu();
  if(readAvgVolt(A0)>LIGHT_TH_HIGH);
  
  char ch;
  // put your main code here, to run repeatedly:
  // toggle PE0
  if(Serial.available())
    do{
      ch = Serial.read();
      Serial.write(ch);
      Serial.flush();
    }while(ch!='\n');
    
  adc_value = readAvgVolt(A0);
  Serial.print("A0 = ");
  Serial.println(adc_value, DEC);
}
