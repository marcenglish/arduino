
#include <SD.h>          /            // need to include the SD library
#include <TMRpcm.h>                  //  also need to include this library...
TMRpcm tmrpcm;                      // create an object for use in this sketch
#define SD_ChipSelectPin 4         //This is how we debug the SD card

void setup()
{
  Serial.begin(9600);
  Serial.println("Moose training");
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
      Serial.println("SD fail");
      Serial.println("First Check ok!");  
  }      
  tmrpcm.setVolume(5);      //Since we are using PWM as an Analog Output the sound 
                            //is very quiet. Need to Bump up the Jams
  tmrpcm.play("test.wav");  //Play the Audio file under the conditions that Moose is Jumping
  delay(2000);
}

void loop()
{
  Serial.println("ok");
  delay(10000);
}

