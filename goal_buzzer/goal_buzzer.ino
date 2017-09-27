#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//------- Replace the following! ------
char ssid[] = "#";       // your network SSID (name)
char password[] = "#";  // your network key
String team = "Los Angeles";
String team_type = "None";
char* feed_url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp?loadScoreboard=jQuery110105207217424176633_1428694268811&_=1428694268812";

String data;
bool game_started = false;
int interval = 10000;
int current_game = 0;
int team_goals = 0;
int opponent_goals = 0;
int spread = 0;
int spread_prev = 0;
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
  //JSON Parsing Prep
  //Shave start
  int start_index = data.indexOf('[');
  data = data.substring(start_index+1);
  
  //Clip end of game entry
  int end_index;
  String test_data;

  while (data.length() > 10) {
      //Processing entire data block overruns memory, so it is split per-game.
      //Get dict closing bracket, use as split token
      end_index = data.indexOf('}');
      test_data = data.substring(0,end_index)+"}";
      
      //Setup JSON
      StaticJsonBuffer<1000> jsonBuffer;  
      JsonObject& game_result = jsonBuffer.parseObject(test_data);

      String away_team = game_result["atn"];
      String home_team = game_result["htn"];
      String game_status = game_result["bs"];

      //A given team may have previous and upcoming games listed.  
      //Filter out the live game, get its game id and set home/away
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
          }else if ((game_status != "FINAL") || (game_status != "FINAL OT")) {
            Serial.println("waiting for game to start");
          }
        }
      }

      if (current_game == game_result["id"]){
        if (team_type == "away"){
          team_goals = game_result["ats"];
          opponent_goals = game_result["hts"];
        }else{
          team_goals = game_result["hts"];
          opponent_goals = game_result["ats"];
        }
        
        spread = (team_goals - opponent_goals);
        
        Serial.println(game_status);
        Serial.println("Spread is: ");
        Serial.println(spread);

        if (spread > spread_prev){
          announce_goal();
        }
        if (spread > 0){
          Serial.println("We're ahead!");
        }
        if ((game_status == "FINAL") || (game_status == "FINAL OT")){
          if (spread > 0){
            announce_win();
          }
          game_started = false;
        }
        spread_prev = spread;
      }
      data = data.substring(end_index+2);
  }  
  delay(interval);  
}

void announce_goal(){
  Serial.println("Goal scored!");
}

void announce_win(){
  Serial.println("We won!");
}

String http_get(String url){
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  return payload;
}
