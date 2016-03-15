#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

#define HDC1000 0x40
uint16_t temperature, humidity;
const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "ESP-WROOM-02";

DNSServer dnsServer;
ESP8266WebServer webServer(80);

os_timer_t countTimer;
int count = 0;

void timeCount(void *pArg) {
  count++;
}

void timeCountArm(int ms) {
  count = 0;
  os_timer_setfn(&countTimer, timeCount, NULL);
  os_timer_arm(&countTimer, ms, true);
}

void timeCountDisarm() {
  os_timer_disarm(&countTimer);
}

void handleRoot() {
  String s = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  s += "</head><body><div>";
  s += "<p>温度 : ";
  s += String(int((temperature / 65536.0) * 165.0 - 40.0));
  s += "</p><p>湿度 : ";
  s += String(int((humidity / 65536.0 * 100.0)));
  s += "</p></div></body></html>";
  webServer.send(200, "text/html", s);
}

void initHDC1000() {
  Wire.beginTransmission(HDC1000);
  Wire.write(0x02);
  Wire.write(0x10);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(HDC1000);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.beginTransmission(HDC1000);
  Wire.write(0xFB);
  Wire.endTransmission();
}

void readHDC1000() {
  Wire.beginTransmission(HDC1000);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(20);
  Wire.requestFrom(HDC1000, 4);
  if (4 <= Wire.available())
  {
    temperature = (Wire.read() << 8);
    temperature += Wire.read();
    humidity = (Wire.read() << 8);
    humidity += Wire.read();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(12, 14);
  temperature = 0;
  humidity = 0;
  initHDC1000();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  dnsServer.start(53, "*", apIP);
  webServer.on("/", handleRoot);
  webServer.begin();
  timeCountArm(1000);
  readHDC1000();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (count > 10) {
    readHDC1000();
    count = 0;
  }
}
