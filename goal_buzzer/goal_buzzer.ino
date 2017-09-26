#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//------- Replace the following! ------
char ssid[] = "#";       // your network SSID (name)
char password[] = "#";  // your network key
String team = "Florida";
String team_type = "None";
char* feed_url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp?loadScoreboard=jQuery110105207217424176633_1428694268811&_=1428694268812";

String data;
bool game_started = false;
int interval = 10000;
int current_game = 0;
int team_goals = 1;
int current_goals = 1;
WiFiClientSecure client;


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
      String game_status = game_result["bs"];

      //A given team may have previous and upcoming games listed.  
      //Get the game id of the live game
      if (game_started != true) {
        if ((away_team == team) || (home_team == team)){
          if (game_status == "LIVE"){
            Serial.println("game is on now!!");
            current_game = game_result["id"];
            game_started = true;
            if (away_team == team){
              team_type = "away";
            }
            if (home_team == team){
              team_type = "home";
            }
          }else if (game_status != "FINAL"){
            Serial.println("waiting for game to start");
          }
        }
      }

      //
      if ((away_team == team) || (home_team == team)){
//      if (current_game == game_result["id"]){
        String away_team_goals = game_result["ats"];
        String home_team_goals = game_result["hts"];
        Serial.println(game_status);
        Serial.println(away_team);
        Serial.println(away_team_goals);
        Serial.println(home_team);
        Serial.println(home_team_goals);
        Serial.println("---");
//        if (team_type == "away"){
//          current_goals = game_result["ats"].toInt();          
//        }
//        if (team_type == "home"){
//          current_goals = game_result["hts"].toInt(); 
//        }
//        if (current_goals > team_goals){
//          announce_goal();
//          team_goals = current_goals;
//        }
      }
      data = data.substring(end_index+2);
  }  
  delay(interval);  
}

void announce_goal(){
  Serial.println("Goal scored!");
}

String http_get(String url){
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload;
}
