void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Serial started!");
  cli();
  TCCR1A = 0;
  TCCR1B = 0xc;
  TCNT1  = 0;
  OCR1A = 31250;
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

boolean toggle1 = 1;

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
  if (toggle1){
    digitalWrite(LED_BUILTIN, HIGH);
    toggle1 = 0;
  }else{
    digitalWrite(LED_BUILTIN, LOW);
    toggle1 = 1;
  }
  Serial.print("1");
}

void loop() {
  // put your main code here, to run repeatedly:

}
