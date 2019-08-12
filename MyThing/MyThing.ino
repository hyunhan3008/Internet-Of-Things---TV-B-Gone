#include <WiFi.h>
#include <ESPWebServer.h>
 
const char *ssid = "hyunhan";
const char *password = "hyunhan123";
 
ESPWebServer server(80);

const int led = 13;
 
void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
  //server.send(200, "text/html", "Hello from ESP8266 AP!");
}
 
void setup() {
 
  Serial.begin(115200);
  
  WiFi.mode(WIFI_AP_STA);
  
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
 
  server.on("/", handleRoot);
  server.begin();
 
  Serial.println("Server listening");
}
 
void loop() {
  server.handleClient();
}
