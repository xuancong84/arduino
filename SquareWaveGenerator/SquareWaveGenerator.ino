byte state __attribute__ ((section (".noinit")));
byte isDblClick __attribute__ ((section (".noinit")));

int N_modes = 4;
void setup() {
  if(isDblClick==1){
    isDblClick=state=0;
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  isDblClick=1;
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  state++;
  if(state<0 || state>=N_modes) state=0;
  delay(1000);
  isDblClick=0;
  digitalWrite(LED_BUILTIN, LOW);

//  state=0; //DEBUG
}

int set_mode(unsigned int val1, unsigned int val2, unsigned int prescaler=0){
  int pres = 1;
  if(state%2==0){
    switch(prescaler){
      case 1024: pres++;
      case 256: pres++;
      case 128: pres++;
      default:
      case 64: pres++;
      case 32: pres++;
      case 8: pres++;
      case 1:;
    }
  }else{
    switch(prescaler){
      case 1024: pres++;
      case 256: pres++;
      case 64: pres++;
      case 8: pres++;
      case 1:
      default:;
    }
  }
  if(state==0){
    pinMode(3, OUTPUT);
    pinMode(11, OUTPUT);
    TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS22) | pres;
    OCR2A = val1;
    OCR2B = val2;
  }else if(state==1){
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    TCCR1A = _BV(COM1A0) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10) | pres;
    OCR1A = val1;
    OCR1B = val2;
  }else if(state==2){
    pinMode(3, OUTPUT);
    pinMode(11, OUTPUT);
    TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS22) | pres;
    OCR2A = val1;
    OCR2B = val2;
  }else if(state==3){
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    TCCR1A = _BV(COM1A0) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(CS10) | pres;
    OCR1A = val1;
    OCR1B = val2;
  }
  return pres;
}

void control(){
  String cmd = Serial.readStringUntil('\n');
  int posi = cmd.indexOf(" ");
  if(posi<=0) return;
  unsigned int val1 = cmd.substring(0, posi).toInt();
  cmd = cmd.substring(posi+1);
  posi = cmd.indexOf(" ");
  unsigned int val3 = 0;
  if(posi>0){
    val3 = cmd.substring(posi+1).toInt();
    cmd = cmd.substring(0, posi);
  }
  unsigned int val2 = cmd.toInt();

  int pres=set_mode(val1, val2, val3);

  if(state%2==0){
    Serial.println(String("Setting OCR2A=")+val1+String(" OCR2B=")+val2+String(" prescaler=")+pres);
  }else{
    Serial.println(String("Setting OCR1A=")+val1+String(" OCR1B=")+val2+String(" prescaler=")+pres);
  }
}

void blink(int n){
  delay(1000);
  for(int x=0; x<n; x++){
    if(Serial.available())
      control();
      
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}

void loop() {
  // wait until serial is ready
  Serial.begin(9600);
  while (!Serial);
  
  // put your main code here, to run repeatedly:
  Serial.println("PWM controller, enter \"v1 v2\" or \"v1 v2 v3\": v1 (frequency denominator), v2 (Vcc*v2/v1 = voltage), and optionally v3 (prescaler)\n"
  "Mode 0/2 uses Timer 2 (8-bit, prescales=1/8/32/64/128/256/1024); Mode 1/3 uses Timer 1 (16-bit, prescales=1/8/64/256/1024)");
  if(state==0){
    set_mode(128, 64);
    Serial.println("You are now in Mode 0 (Fast PWM), use PIN 3");
  }else if(state==1){
    set_mode(32768, 16384);
    Serial.println("You are now in Mode 1 (Fast PWM), use PIN 10");
  }else if(state==2){
    set_mode(128, 64);
    Serial.println("You are now in Mode 2 (PWM Phrase Correct), use PIN 3");
  }else if(state==3){
    set_mode(32768, 16384);
    Serial.println("You are now in Mode 3 (PWM Phrase Correct), use PIN 10");
  }
  
  while(1){blink(state+1);}
}
