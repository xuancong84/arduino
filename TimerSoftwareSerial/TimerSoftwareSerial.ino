#define RX_PIN 3
#define TX_PIN 2
#define MAX_BUF 256
#define readPIN (PIND&8?1:0)
#define CYCLES_PER_BIT 139
#define GRACE_CYCLES 40

int RX_head = 0, RX_tail = 0;
char x, RX_BUF[MAX_BUF];
uint16_t RX_packet = 0;
uint16_t next_tick = CYCLES_PER_BIT-GRACE_CYCLES;

void wait_for_nodata(){ // when not sending data, RX PIN is hold high
  TCNT1 = 0;
  while(TCNT1< CYCLES_PER_BIT*10)
    if(!readPIN)TCNT1=0;
}

void uart_start_vect(){
  cli();  // clock-cycle critical session, must disable all interrupts
  TCNT1 = 0;  // reset counter
  while(TCNT1<next_tick);  // wait for the start bit to finish

  // start reading packet
re:
  while(1){
    RX_packet |= (readPIN<<x);
    if(++x>8)break;
    next_tick += CYCLES_PER_BIT;
    while(TCNT1<next_tick);
  }

  // save the byte into circular buffer
  RX_BUF[RX_head] = RX_packet;
  if(++RX_head>=MAX_BUF)
    RX_head = 0;

  // 16000000/115200=138.888 that is 139 for 8/9 of the time and 1/9 of the time
  next_tick += CYCLES_PER_BIT-1;
  RX_packet = 0;
  x = 0;
  while(TCNT1<next_tick); // wait for stop bit to finish
  if(!readPIN){ // another start bit => consective packet
    next_tick += CYCLES_PER_BIT;
    while(TCNT1<next_tick); // wait for start bit to finish
    goto re;
  }

  // prepare for the next
  next_tick = CYCLES_PER_BIT-GRACE_CYCLES;
  
  sei();  // resume all interrupts
}

void setup() {
  delay(200); // prevent running twice when uploading while console is open
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RX_PIN, INPUT_PULLUP);
  pinMode(TX_PIN, INPUT_PULLUP);
  
  // setup serial
  digitalWrite(LED_BUILTIN, 1);
  Serial.begin(115200);
  Serial.println("TimerSoftwareSerial started:");

  // setup timer1 in normal mode for gettickcount()
  cli();
  TCCR1A = 0;
  TCCR1B = 1;
  OCR1A = 0xffff;

  // setup external interrupt
  attachInterrupt(digitalPinToInterrupt(RX_PIN), uart_start_vect, FALLING);

  wait_for_nodata();

  // start all interrupts
  sei();
}

bool pool(){
  return RX_head!=RX_tail;
}

char *recv(char*buf){
  int p=0;
  while(RX_head!=RX_tail){
    buf[p++] = RX_BUF[RX_tail++]&0b01111111;
    if(RX_tail>=MAX_BUF)
      RX_tail=0;
    if(p>=MAX_BUF)
      Serial.println("\nrecv: buffer overflow");
  }
  buf[p] = 0;
  return buf;
}

char arr[2*256];
void print_bits(char *buf){
  int posi=0;
  char ch;
  for(unsigned char x=0; buf[x]!=0; ++x){
    ch = buf[x];
    for(unsigned char y=0; y<8; ++y)
      arr[posi++] = ((ch<<y)&0b10000000)?'1':'0';
    arr[posi++] = ' ';
    if(posi>100)break;
  }
  arr[posi]=0;
  Serial.println(arr);
}

char buf[MAX_BUF];
unsigned char *last;
void loop() {
  // put your main code here, to run repeatedly:
  if(pool()){
    recv(buf);
    last = (unsigned char*)&buf[strlen(buf)-1];
    while(*last>0x7e || *last<0x20)
      *(last--)=0;
    Serial.println(buf);
//    else Serial.println(buf);
//    print_bits(buf);
  }
}
