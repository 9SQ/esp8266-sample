#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define LED 13

const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "ESP-WROOM-02";
DNSServer dnsServer;
ESP8266WebServer webServer(80);

void handleRoot() {
  String s = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  s += "<script>function rest(cmd,method){var http = new XMLHttpRequest();var url = \"/led\";http.open(method, url, true);http.send(\"c=\"+cmd);http.onreadystatechange=function(){if(http.readyState == 4 && http.status == 200){alert(http.responseText);}}}</script>";
  s += "</head><body><div style=\"text-align: center\">";
  s += "<h1>LED</h1>";
  s += "<button type=\"button\" onclick=\"rest('on','POST');\">ON</button>";
  s += "<hr>";
  s += "<button type=\"button\" onclick=\"rest('off','POST');\">OFF</button>";
  s += "</div></body></html>";
  webServer.send(200, "text/html", s);
}

void handleControl() {
  String s = "";
  if (webServer.method() == HTTP_POST) {
    if (webServer.arg("c") == "on") {
      Serial.println("LED ON");
      digitalWrite(LED, HIGH);
      s = "ON!";
    }
    else if (webServer.arg("c") == "off") {
      Serial.println("LED OFF");
      digitalWrite(LED, LOW);
      s = "OFF!";
    }
  }
  webServer.send(200, "text/plain", s);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  dnsServer.start(53, "*", apIP);
  webServer.on("/", handleRoot);
  webServer.on("/led", handleControl);
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
