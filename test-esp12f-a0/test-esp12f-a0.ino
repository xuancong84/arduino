#define FlashButtonPIN 0

char PINs[11] = {A0, D0, D1, D2, D4, D5, D6, D7, D8, D9, D10};
bool state_up = true;
void init_pins(){
  for(int x=0; x<11; ++x)
    pinMode(PINs[x], state_up?INPUT_PULLUP:INPUT_PULLDOWN_16);
}

void IRAM_ATTR handleInterrupt() {
  state_up = !state_up;
  Serial.print("button state = ");
  Serial.println(state_up?"INPUT_PULLUP":"INPUT_PULLDOWN");
}

void setup() {
  delay(1000);
  pinMode(FlashButtonPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FlashButtonPIN), handleInterrupt, FALLING);
  init_pins();
  
  Serial.begin(115200);
  Serial.println("\nSystem initialized: press FLASH button to toggle INPUT_PULLUP and INPUT_PULLDOWN");
}

void loop() {
  int val = analogRead(A0);
  Serial.println(val);
  delay(500);
}
