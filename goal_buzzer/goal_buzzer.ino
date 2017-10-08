#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
const int buttonPin = 2;     // the number of the pushbutton pin
const int wifi_led_pin = 15;
const int islive_pin = 13;
const int pot_pin = 0;
int prev_sensor_value = 0;
#define buzzer 14

#include <LiquidCrystal_I2C.h>  
#include <Wire.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

//------- Replace the following! ------
char ssid[] = "#";       // your network SSID (name)
char password[] = "#";  // your network key
char* feed_url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp?loadScoreboard=jQuery110105207217424176633_1428694268811&_=1428694268812";

String team = "";
String data;
int buttonState = 0;         // variable for reading the pushbutton status
bool game_started = false;
int interval = 10000;
int current_game_id = 0;
int team_goals = 0;
int opponent_goals = 0;
int spread = 0;
int spread_prev = 0;
String teams[] = {"Anaheim", "Arizona", "Boston", "Buffalo", "Calgary", "Carolina", "Chicago", 
                  "Colorado", "Columbus", "Dallas", "Detroit", "Edmonton", "Florida", "Los Angeles", 
                  "Minnesota", "Montreal", "Nashville", "New Jersey", "NY Rangers", "NY Islanders", 
                  "Ottawa", "Philadelphia", "Pittsburgh", "San Jose", "St. Louis", 
                  "Tampa Bay", "Toronto", "Vancouver", "Vegas", "Washington", "Winnipeg"};

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(buttonPin, INPUT);
  pinMode(wifi_led_pin, OUTPUT);
  pinMode(islive_pin, OUTPUT);
  
  //Setup WIFI
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  display("Connecting to ",0);
  display(ssid, 1);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  IPAddress ip = WiFi.localIP();
  display("Connected!", 2);
  display(WiFi.localIP().toString(), 3);
  delay(1500);
  display_clear();

  announce_goal();
  
}

void loop() {
    buttonState = digitalRead(buttonPin);
    lcd.setCursor(19,0);
    lcd.print(buttonState);
    //Initialize the thing.  Prompt user for team selection.
    if (team == ""){
        display("Please select a team", 0);
        int sensor_value = analogRead(pot_pin);
        sensor_value = map(sensor_value, 0, 1023, 0, 30);

        if (sensor_value != prev_sensor_value){
            display("                    ", 1);
            display(teams[sensor_value],1);
            prev_sensor_value = sensor_value;
        }

        if (buttonState == HIGH) {
            team = teams[sensor_value];
            display_clear();
            delay(1000);
        }

    }else{
        delay(interval);
        display("Team is:", 0);
        display(team, 1);
        if (buttonState == HIGH) {
            announce_goal();
        }

    }

    check_wifi();

    check_islive();

    data = http_get(feed_url);
  
    //JSON Parsing Prep
    //Shave start
    int start_index = data.indexOf('[');
    data = data.substring(start_index+1);

    //Clip end of game entry
    int end_index;
    String test_data;

    while (data.length() > 10) {
        //Extract home team/away team/status
        //Processing entire data block overruns memory, so it is split per-game.
        //Get dict closing bracket, use as split token
        end_index = data.indexOf('}');
        test_data = data.substring(0,end_index)+"}";
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
                    current_game_id = game_result["id"];
                    game_started = true;
                    announce_game_start();
                    display_clear();
            //}else if ((game_status != "FINAL") && (game_status != "FINAL OT") && (game_status != "FINAL SO")) {
                }else{

                    display("                    ",2);
                    // display(game_status, 2);
                    display("waiting to start", 3);
                }
            }
        }
        
        
        if (current_game_id == game_result["id"]){
            
            if (away_team == team){
                team_goals = game_result["ats"];
                opponent_goals = game_result["hts"];
            }
            
            if (home_team == team){
                team_goals = game_result["hts"];
                opponent_goals = game_result["ats"];
            }
            
            
            spread = (team_goals - opponent_goals);
            display("Spread is: ", 2);
            display(String(spread), 3);
    
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
                spread = 0;
                game_started = false;
                current_game_id = 0;
            }
            spread_prev = spread;
        }
        data = data.substring(end_index+2);
    }  

      
}

void announce_game_start(){
    Serial.println("Game Started!");
    tone(buzzer, 100, 1000);
    tone(buzzer, 200, 1000);
}

void announce_goal(){
    Serial.println("Goal scored!");
    display_clear();
    display("Goal Scored!!", 2);
    display_clear();
    tone(buzzer, 100, 100);
    delay(200);
    tone(buzzer, 100, 100);
    delay(200);
    tone(buzzer, 100, 100);
    delay(200);
    tone(buzzer, 200, 300);
    delay(200);    
}

void announce_win(){
    Serial.println("We won!");
    tone(buzzer, 300, 100);
    delay(200);
    tone(buzzer, 100, 100);
    delay(200);
    tone(buzzer, 300, 100);
    delay(200);
    tone(buzzer, 200, 300);
    delay(200);    
    tone(buzzer, 300, 600);
    delay(200);
}

String http_get(String url){
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    String payload = http.getString();
    http.end();
    return payload;
}

void check_wifi(){
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(wifi_led_pin, HIGH);
        delay(100);
        digitalWrite(wifi_led_pin, LOW);
  }
}

void check_islive(){
    if (game_started == true) {
        digitalWrite(islive_pin, HIGH);
        delay(100);
        digitalWrite(islive_pin, LOW);

    }
}


void display(String chars, int line){
    // Serial.println(chars);
    // Serial.println(line);

    // lcd.setCursor(0,line);
    // lcd.print("                    ");
    lcd.setCursor(0,line);
    lcd.print(chars);

}

void display_clear(){
    for (int i=0; i < 5; i++){
        lcd.setCursor(0,i);
        lcd.print("                    ");    
    }
}