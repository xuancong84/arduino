#include <Esp.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <IPAddress.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <NTPClient.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <mbedtls/md5.h>

AsyncWebServer server(80);
DNSServer *dnsServer = NULL;
WiFiUDP *ntpUDP = NULL;
NTPClient *timeClient = NULL;
float timezone = 8;
int all_ports[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,33,34,35,36,37,38,39,40};
int N_ports = sizeof(all_ports)/sizeof(int);
char server_html[] = R"abc(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>ESP32 tester</title>
</head>
<body>
<h1>My First Web Server with ESP32 - Station Mode &#128522;</h1>
Datetime: <input type='text' id='datetime' size=32 readonly>
<script>
var xmlHttp = new XMLHttpRequest();
function update_status(){
  xmlHttp.open('GET', window.location+"status");
  xmlHttp.onload = (e)=>{document.getElementById("datetime").value=e.target.responseText};
  xmlHttp.send(null);
}
setInterval(update_status, 1000);
</script>
</body>
</html>
)abc";

String getTimeString(){
  char buf[16];
  if(!timeClient) return "00:00:00";
  sprintf(buf, "%02d:%02d:%02d", timeClient->getHours(), timeClient->getMinutes(), timeClient->getSeconds());
  return String(buf);
}
String getDateString(bool showDay=true){
  if(!timeClient)
    return showDay?"0000-00-00":"0000-00";
  time_t epochTime = timeClient->getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  char buf[16];
  showDay?sprintf(buf, "%04d-%02d-%02d", currentYear, currentMonth, monthDay):sprintf(buf, "%04d-%02d", currentYear, currentMonth);
  return String(buf);
}
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String getWeekdayString(){
  if(!timeClient) return "";
  return weekDays[timeClient->getDay()];
}
String getFullDateTime(){
  if(!timeClient) return "";
  return getDateString()+" ("+getWeekdayString()+") "+getTimeString();
}

void initNTP(){
  if(timeClient){
    timeClient->end();
    delete timeClient;
  }
  if(ntpUDP){
    ntpUDP->stop();
    delete ntpUDP;
  }
  ntpUDP = new WiFiUDP();
  timeClient = new NTPClient(*ntpUDP, "pool.ntp.org", timezone*3600, 7200);
  timeClient->begin();
  timeClient->update();
  Serial.printf("Current datetime = %s\n", getFullDateTime().c_str());
}

boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9'))
      return false;
  }
  return true;
}

void handleRoot(AsyncWebServerRequest *request) {
  if(dnsServer && !isIp(request->host())){
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
    response->addHeader("Location", "http://" + request->client()->localIP().toString());
    request->send(response);
    request->client()->stop();
    return;
  }
  request->send_P(200, "text/html", server_html);
}

void initServer(){
  server.on("/", handleRoot);
  server.on("/status", [](AsyncWebServerRequest *request) {request->send(200, "text/html", getFullDateTime());});
  server.onNotFound([](AsyncWebServerRequest *request){request->send(404, "text/plain", "Content not found.");});
  server.begin();
  Serial.println("HTTP server started");
}

void hotspot(String ssid){
  IPAddress apIP(172, 0, 0, 1);
  Serial.println("Creating hotspot '"+ssid+"' ...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  Serial.print("Hotspot IP address: ");
  Serial.println(apIP);
  dnsServer = new DNSServer();
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(53, "*", apIP);
  initNTP();
  initServer();
}

void connect_wifi(String wifi_ssid, String wifi_password){
  if(dnsServer){
    dnsServer->stop();
    delete dnsServer;
    dnsServer = NULL;
  }

  Serial.print("Connecting to WiFi ...");

  // Configures static IP address
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  for(int x=0; x<=60; ++x){
    if(WiFi.status() == WL_CONNECTED) break;
    delay(1000);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }else{
    Serial.println("\nUnable to connect to WiFi");
    return;
  }

  // Update time from Internet
  initNTP();
  initServer();
}

void setup() {
  // 1. initialize serial comm
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  Serial.begin(115200);
  delay(3000); // avoid soft brick
  Serial.printf("\nSystem started: %d ports in total\n", N_ports);
  Serial.println("Command set: pinMode/set_high/set_low/set_value/digitalRead/analogRead/connect_wifi/hotspot/disconnect_wifi, port number can be 'all'");
  Serial.flush();

  digitalWrite(LED_BUILTIN, 0);
}

void loop() {
  // Hotspot captive portal
  if(dnsServer)
    dnsServer->processNextRequest();

  if(!Serial.available()){
    delay(1);
    return;
  }
  String s = Serial.readStringUntil('\n');
  s.trim();

  if(s.startsWith("pinMode ")){
    String val1 = s.substring(s.indexOf(' '));
    val1.trim();
    int sp = val1.indexOf(' ');
    String port_s = val1.substring(0, sp);
    String mode_s = val1.substring(sp);
    mode_s.trim();
    int mode_i=0;
    if(mode_s=="INPUT") mode_i=INPUT;
    else if(mode_s=="OUTPUT") mode_i=OUTPUT;
    else if(mode_s=="PULLUP") mode_i=PULLUP;
    else if(mode_s=="INPUT_PULLUP") mode_i=INPUT_PULLUP;
    else if(mode_s=="PULLDOWN") mode_i=PULLDOWN;
    else if(mode_s=="INPUT_PULLDOWN") mode_i=INPUT_PULLDOWN;
    else if(mode_s=="OPEN_DRAIN") mode_i=OPEN_DRAIN;
    else if(mode_s=="OUTPUT_OPEN_DRAIN") mode_i=OUTPUT_OPEN_DRAIN;
    else if(mode_s=="ANALOG") mode_i=ANALOG;
    else mode_i = mode_s.toInt();
    if(port_s=="all")
      for(int x=0;x<N_ports;x++) pinMode(all_ports[x], mode_i);
    else
      pinMode(port_s.toInt(), mode_i);
  }else if(s.startsWith("set_high ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if(val=="all")
      for(int x=0;x<N_ports;x++) digitalWrite(all_ports[x], 1);
    else
      digitalWrite(val.toInt(), 1);
  }else if(s.startsWith("set_low ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if(val=="all")
      for(int x=0;x<N_ports;x++) digitalWrite(all_ports[x], 0);
    else
      digitalWrite(val.toInt(), 0);
  }else if(s.startsWith("set_value ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    int sp = val.indexOf(' ');
    String pin_s = val.substring(0, sp);
    int pin = pin_s.toInt();
    int value = val.substring(sp).toInt();
    if(pin_s=="all")
      for(int x=0;x<N_ports;x++) analogWrite(all_ports[x], value);
    else
      analogWrite(pin, value);
  }else if(s.startsWith("digitalRead ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    Serial.println(String("OUTPUT=")+digitalRead(val.toInt()));
  }else if(s.startsWith("analogRead ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    Serial.println(String("OUTPUT=")+analogRead(val.toInt()));
  }else if(s.startsWith("connect_wifi ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    int sp = val.indexOf(' ');
    String wifi_ssid = val.substring(0, sp);
    String wifi_password = val.substring(sp);
    wifi_ssid.trim();
    wifi_password.trim();
    connect_wifi(wifi_ssid, wifi_password);
  }else if(s.startsWith("hotspot")){
    int posi = s.indexOf(' ');
    String ssid = posi>7?s.substring(posi):"ArduinoTest";
    ssid.trim();
    hotspot(ssid);
  }else if(s.startsWith("disconnect_wifi")){
    WiFi.disconnect(false, true);
    WiFi.mode(WIFI_OFF);
    Serial.println("Wifi disconnected and turned off!");
  }else
    Serial.println("Unknown command!");
}
