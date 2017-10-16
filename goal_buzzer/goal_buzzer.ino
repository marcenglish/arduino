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

String data;
String gamedata;
int buttonState = 0;         // variable for reading the pushbutton status
bool game_started = false;
int interval = 10000;
int current_game_id = 0;
int team_goals = 0;
int opponent_goals = 0;
String spread = "None";
int spread_prev = 0;
long previousMillis = 0; 
long previousMillis_display = 0; 
String team = "Tampa Bay";
String teams[] = {"Anaheim", "Arizona", "Boston", "Buffalo", "Calgary", "Carolina", "Chicago", 
                  "Colorado", "Columbus", "Dallas", "Detroit", "Edmonton", "Florida", "Los Angeles", 
                  "Minnesota", "Montreal", "Nashville", "New Jersey", "NY Rangers", "NY Islanders", 
                  "Ottawa", "Philadelphia", "Pittsburgh", "San Jose", "St. Louis", 
                  "Tampa Bay", "Toronto", "Vancouver", "Vegas", "Washington", "Winnipeg"};

WiFiClientSecure client;
int count = 0;
int buzzes = 0;
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
  

  announce_start();
  
}

void loop() {
    delay(100);

    unsigned long currentMillis = millis();

    buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH) {
      announce_goal(); 
    }

    if(currentMillis - previousMillis_display > 10000) {
        previousMillis_display = currentMillis;  

        if (game_started == true){
            display_clear();
            display(team, 0);
            display("Game is on now.", 1);
            display(String(count), 2);
            display(String(buzzes), 3);
        }else{
            display_clear();
            display(team, 0);
            display("Waiting for game.", 1);
            display(String(count), 2);
            display(String(buzzes), 3);
        }
    }
    if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;  

        check_wifi();
        
        get_live_spread();
    
        Serial.println(spread);

        if (spread == "None"){ //If no live game is current available
            if (game_started == true && spread_prev > 0){
                announce_win();
            }
            game_started = false;
            interval = 1800000;
            
        }else{
            interval = 10000;
            announce_islive();

            if (spread.toInt() > spread_prev){
                announce_goal();
            }
        }

        spread_prev = spread.toInt();

        count +=1;
    }
}    


void get_live_spread(){

    data = http_get(feed_url);
    spread = "None";
    //JSON Parsing Prep
    //Shave start
    int start_index = data.indexOf('[');
    data = data.substring(start_index+1);

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
        // if (game_started != true) {
        if ((away_team == team) || (home_team == team)){
            if (game_status == "LIVE"){
                game_started = true;

                if (away_team == team){
                    team_goals = game_result["ats"];
                    opponent_goals = game_result["hts"];
                }
                
                if (home_team == team){
                    team_goals = game_result["hts"];
                    opponent_goals = game_result["ats"];
                }
            
                int num_spread = team_goals - opponent_goals;
                spread = String(num_spread);
            }
        }        
        // }
        data = data.substring(end_index+2);
    }


}

void announce_game_start(){
    Serial.println("Game Started!");
    tone(buzzer, 100, 1000);
    tone(buzzer, 200, 1000);
}

void announce_start(){
    Serial.println("Program Started!");
    tone(buzzer, 100, 100);
    delay(500);
    tone(buzzer, 100, 100);
    delay(500);
    tone(buzzer, 500, 500);
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
    buzzes += 1; 
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

void announce_islive(){
        digitalWrite(islive_pin, HIGH);
        delay(100);
        digitalWrite(islive_pin, LOW);

}
void display(String chars, int line){
    lcd.setCursor(0,line);
    lcd.print(chars);

}

void display_clear(){
    for (int i=0; i < 5; i++){
        lcd.setCursor(0,i);
        lcd.print("                    ");    
    }
}
