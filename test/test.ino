int v0,v1,v2,v3;

void setup() {
  // setup Timer 1
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Serial started!");

  v0=0;
  if(v0&1)Serial.println("Nonsense");
  Serial.println(v0);

  cli();
  TCCR1A = 0;
  TCCR1B = 0b00001001;
  TCNT1  = 0;
  OCR1A = 0xffff;
  TIMSK1 |= (1 << OCIE1A);

  v0=TCNT1;
  v1=TCNT1;
  char pin = PIND&8?1:0;
  v2=TCNT1;
  
  sei();
  Serial.println(v0);
  Serial.println(v1);
  Serial.println(v2);
  Serial.print("pin=");
  Serial.println((int)pin);
  while(1);
}

boolean toggle1 = 1;
int cnt=0;

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
  if (toggle1){
    digitalWrite(LED_BUILTIN, HIGH);
    toggle1 = 0;
    Serial.print("0");
  }else{
    digitalWrite(LED_BUILTIN, LOW);
    toggle1 = 1;
    Serial.print("1");
    cnt+=1;
    if(cnt==5){
      TIMSK1 = 0;
      Serial.println("disable");
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
