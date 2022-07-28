#define FlashButtonPIN 0
volatile int dMode = 1;

bool state = false;
void IRAM_ATTR handleInterrupt() {
  state = !state;
  Serial.print("button state = ");
  Serial.println(state);
  digitalWrite(D1, state);
  digitalWrite(LED_BUILTIN, !state);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nSystem initialized:");

  pinMode(FlashButtonPIN, INPUT_PULLUP);
  pinMode(D1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, state);
  digitalWrite(LED_BUILTIN, !state);
  attachInterrupt(digitalPinToInterrupt(FlashButtonPIN), handleInterrupt, FALLING);
}

void loop() {
  delay(1000);
}
