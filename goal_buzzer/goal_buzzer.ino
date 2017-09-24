#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

//------- Replace the following! ------
char ssid[] = "CheCarino";       // your network SSID (name)
char password[] = "tesoro1234";  // your network key
char* test = "";
char* feed_url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp?loadScoreboard=jQuery110105207217424176633_1428694268811&_=1428694268812";
WiFiClientSecure client;
String data = "None";

void setup() {
  Serial.begin(115200);
  
  //Setup WIFI
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  IPAddress ip = WiFi.localIP();
  Serial.println('Connected!');
  Serial.println(ip);
}

void loop() {
  data = http_get(feed_url);
  Serial.println(data);
  delay(5000);
  
}

String http_get(String url){
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload;
}
