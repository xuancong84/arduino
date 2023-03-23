#include <Esp.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <IPAddress.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <NTPClient.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include "ESPAsyncUDP.h"
#include <md5.h>

AsyncWebServer server(80);
DNSServer *dnsServer = NULL;
WiFiUDP *ntpUDP = NULL;
AsyncUDP Udp;
NTPClient *timeClient = NULL;
float timezone = 8;
int all_ports[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, A0};
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
  delay(1000);
  initNTP();
  initServer();
}
void help(){
  Serial.println(R"aa(Commands: <port> number can be 'all', [] means optional argument
  help: print this manual
  pinMode <port/int> <mode/int|str> : change pin mode
  set_high <port/int> : digitalWrite 1 to <port>
  set_low <port/int> : digitalWrite 0 to <port>
  set_value <port/int> <value/int> : analogWrite <value> to <port>
  connect_wifi <ssid> <password> : connect to WiFi
  disconnect_wifi : disconnect WiFi
  wifi_sleep : WiFi.forceSleepBegin()
  wifi_wake : WiFi.forceSleepWake()
  hotspot [SSID=ArduinoTest] : create WiFi hotspot with SSID
  udp_send <IP> <port> <msg>: send a UDP message
  udp_bc <port> <msg>: broadcast a UDP message)aa");
}
void setup() {
  // 1. initialize serial comm
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(115200);
  delay(3000); // avoid soft brick
  Serial.printf("\nSystem started: %d ports in total\n", N_ports);
  Serial.println("Command set: help/pinMode/set_high/set_low/set_value/digitalRead/analogRead/connect_wifi/hotspot/disconnect_wifi/wifi_sleep/wifi_wake/udp_send/udp_bc");
  Serial.flush();

  digitalWrite(LED_BUILTIN, 1);
}

String get_pin(String name){
  if(name!="all") return name;
  String s;
  for(int x=0;x<N_ports;x++) s += String(all_ports[x])+" ";
  s.trim();
  return s;
}

void loop() {
  // Hotspot captive portal
  if(dnsServer)
    dnsServer->processNextRequest();

  if(!Serial.available()){
    delay(1);
    return;
  }
  int sp;
  String s = Serial.readStringUntil('\n');
  s.trim();

  if(s.startsWith("help")){
    help();
  }else if(s.startsWith("pinMode ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if((sp=val.indexOf(' '))<0) return;
    String port_s = val.substring(0, sp);
    String mode_s = val.substring(sp);
    mode_s.trim();
    int mode_i=0;
    if(mode_s=="INPUT") mode_i=INPUT;
    else if(mode_s=="OUTPUT") mode_i=OUTPUT;
    else if(mode_s=="INPUT_PULLUP") mode_i=INPUT_PULLUP;
    else mode_i = mode_s.toInt();
    if(port_s=="all")
      for(int x=0;x<N_ports;x++) pinMode(all_ports[x], mode_i);
    else
      pinMode(port_s.toInt(), mode_i);
    Serial.printf("PIN %s set to mode %d\n", get_pin(port_s).c_str(), mode_i);
  }else if(s.startsWith("set_high ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if(val=="all")
      for(int x=0;x<N_ports;x++) digitalWrite(all_ports[x], 1);
    else
      digitalWrite(val.toInt(), 1);
    Serial.printf("PIN %s set to high\n", get_pin(val).c_str());
  }else if(s.startsWith("set_low ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if(val=="all")
      for(int x=0;x<N_ports;x++) digitalWrite(all_ports[x], 0);
    else
      digitalWrite(val.toInt(), 0);
    Serial.printf("PIN %s set to low\n", get_pin(val).c_str());
  }else if(s.startsWith("set_value ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if((sp=val.indexOf(' '))<0) return;
    String pin_s = val.substring(0, sp);
    int pin = pin_s.toInt();
    int value = val.substring(sp+1).toInt();
    if(pin_s=="all")
      for(int x=0;x<N_ports;x++) analogWrite(all_ports[x], value);
    else
      analogWrite(pin, value);
    Serial.printf("PIN %s set to %d\n", get_pin(pin_s).c_str(), value);
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
    if((sp=val.indexOf(' '))<0) return;
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
    Serial.println("Wifi disconnected!");
  }else if(s.startsWith("wifi_sleep")){
    WiFi.forceSleepBegin();
    Serial.println("Wifi force sleep begin!");
  }else if(s.startsWith("wifi_wake")){
    WiFi.forceSleepWake();
    Serial.println("Wifi wakeup!");
  }else if(s.startsWith("udp_send ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if((sp=val.indexOf(' '))<0) return;
    String IP = val.substring(0, sp);
    val = val.substring(sp);
    val.trim();

    if((sp=val.indexOf(' '))<0) return;
    int port = val.substring(0, sp).toInt();
    String msg = val.substring(sp);
    msg.trim();

    IPAddress ipa;
    if(!ipa.fromString(IP)) return;

    AsyncUDPMessage udp_msg;
    udp_msg.write((const uint8_t*)msg.c_str(), msg.length());

    int res = Udp.sendTo(udp_msg, ipa, port);
    Serial.printf("UDP msg (%d bytes) sent to IP=%s port=%d\n", res, IP.c_str(), port);
  }else if(s.startsWith("udp_bc ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    if((sp=val.indexOf(' '))<0) return;
    int port = val.substring(0, sp).toInt();
    String msg = val.substring(sp);
    msg.trim();

    int res = Udp.broadcastTo(msg.c_str(), port);
    Serial.printf("UDP msg (%d bytes) broadcast to 255.255.255.255:%d\n", msg.length(), port);
  }else if(s.startsWith("udp_listen ")){
    String val = s.substring(s.indexOf(' '));
    val.trim();
    int port = val.toInt();
    if(Udp.listen(port)){
      Udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length());
        });
      Serial.printf("UDP listen on port %d successfully\n", port);
    }else{
      Serial.printf("UDP listen on port %d failed\n", port);
    }
  }else
    Serial.println("Unknown command!");
}
