#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define FlashButtonPIN 0
bool state = false;

/* Set these to your desired credentials. */
const char *ssid = "WXC";                 //Enter your WIFI ssid
const char *password = "***";  //Enter your WIFI password
ESP8266WebServer server(80);
void handleRoot() {
  server.send(200, "text/html", String(state?"LED is on<br>":"LED is off<br>")+
    String("<form action=\"/LED_BUILTIN_on\" method=\"get\" id=\"form1\"></form><button type=\"submit\" form=\"form1\" value=\"On\">On</button>"
    "<form action=\"/LED_BUILTIN_off\" method=\"get\" id=\"form2\"></form><button type=\"submit\" form=\"form2\" value=\"Off\">Off</button>"
    "<form action=\"/LED_BUILTIN_toggle\" method=\"get\" id=\"form3\"></form><button type=\"submit\" form=\"form3\" value=\"Off\">Toggle</button>"));
}

void handleSave() {
  if (server.arg("pass") != "") {
    Serial.println(server.arg("pass"));
  }
}

void act(){
  Serial.print("button state = ");
  Serial.println(state);
  digitalWrite(D1, state);
  digitalWrite(LED_BUILTIN, !state);
}

void IRAM_ATTR handleInterrupt() {
  state = !state;
  act();
}

void setup() {
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FlashButtonPIN, INPUT_PULLUP);
  pinMode(D1, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FlashButtonPIN), handleInterrupt, FALLING);
  
  delay(1000);
  Serial.begin(115200);
  Serial.println("\nSystem initialized:");
  act();
  Serial.print("Configuring access point...");
  WiFi.begin(ssid, password);
  int x = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if(++x%32==0)Serial.println();
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
  Serial.println("HTTP server started");
  server.on("/LED_BUILTIN_on", []() {
    state = true;
    act();
    handleRoot();
  });
  server.on("/LED_BUILTIN_off", []() {
    state = false;
    act();
    handleRoot();
  });
  server.on("/LED_BUILTIN_toggle", []() {
    state = !state;
    act();
    handleRoot();
  });
}

void loop() {
  server.handleClient();
}
