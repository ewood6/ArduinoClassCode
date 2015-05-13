//Code for detecting motion from an IRmotion sensor
//This is a digital only piece of hardware. Either there is motion or there isn’t

//Declare Variables
int pirPin = 2; //digital 2 for input from motion sensor

void setup(){
 Serial.begin(9600); 
 pinMode(pirPin, INPUT);
}

void loop(){
  int pirVal = digitalRead(pirPin);

  if(pirVal == LOW){ //was motion detected
    Serial.println("Motion Detected"); 
    delay(500); 
  }
  if(pirVal == HIGH){ //was motion detected
    Serial.println("No Motion"); 
    delay(500); 
  }
  

}
