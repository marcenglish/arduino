#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//------- Replace the following! ------
char ssid[] = "##";       // your network SSID (name)
char password[] = "##";  // your network key
char* test = "";
char* feed_url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp?loadScoreboard=jQuery110105207217424176633_1428694268811&_=1428694268812";
WiFiClientSecure client;
//String data = "None";

void setup() {
  Serial.begin(9600);
  
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
  String data = http_get(feed_url);
  Serial.println("Go...");
  //JSON Parsing Prep
  //Shave start
  int start_index = data.indexOf('[');
  data = data.substring(start_index+1);
  
  //Clip end of game entry
  int end_index;
  String test_data;

  while (data.length() > 10) {
      end_index = data.indexOf('}');
      test_data = data.substring(0,end_index)+"}";
      
      //Setup JSON
      StaticJsonBuffer<1000> jsonBuffer;  
      JsonObject& game_result = jsonBuffer.parseObject(test_data);
      String away_team = game_result["atn"];
      String home_team = game_result["htn"];
     
      Serial.println(away_team);
      Serial.println(home_team);
      Serial.println("---");
      data = data.substring(end_index+2);

  }
    
  
//  Serial.println("-----");
//  String game_result["Date"] = data["ts"];
//  Serial.println(game_result["Date"]);
//  String game_result["Status"] = data["bs"];
//  Serial.println(game_result["Status"]);
//  String game_result["Away Team"] = data["atn"];
//  Serial.println(game_result["Away Team"]);
//  String game_result["Away Goals"] = data["ats"];
//  Serial.println(game_result["Away Goals"]);
//  String game_result["Home Team"] = data["htn"];
//  Serial.println(game_result["Home Team"]);
//  String game_result["Home Goals"] = data["hts"];
//  Serial.println(game_result["Home Goals"]);
  delay(10000);
  
}



String http_get(String url){;
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload;
}
