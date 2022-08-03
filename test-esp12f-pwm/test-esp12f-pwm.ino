

char PINs[11] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10};
void setup() {
  delay(1000);
  for(int x=0; x<11; ++x){
    pinMode(PINs[x], OUTPUT);
    digitalWrite(PINs[x], 0);
  }
  
  Serial.begin(115200);
  Serial.println("\nSystem initialized: please input values for analogWrite(): PORT_number(0-10) PWM_value(0-255) [PWM_freq(1-100000)]");
}

char buf[512];
void loop() {
  if(Serial.available()){
    int x,y,z=0;
    Serial.readBytesUntil('\n', buf, 500);
    sscanf(buf, "%d%d%d", &x, &y, &z);
    analogWrite(PINs[x], y);
    sprintf(buf, "PIN D%d set to %d", x, y);
    Serial.println(buf);
    if(z>0){
      analogWriteFreq(z);
      Serial.print("PWM freq set to ");
      Serial.println(z);
    }
  }
}
