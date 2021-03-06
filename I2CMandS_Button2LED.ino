/**
 *
 * Sample Multi Master I2C implementation.  Sends a button state over I2C to another
 * Arduino, which flashes an LED correspinding to button state.
 * 
 * Connections: Arduino analog pins 4 and 5 are connected between the two Arduinos, 
 * with a 1k pullup resistor connected to each line.  Connect a push button between 
 * digital pin 10 and ground, and an LED (with a resistor) to digital pin 9.
 * 
 */

#include <Wire.h>

#define LED 9
#define BUTTON 10

#define THIS_ADDRESS 0x8
#define OTHER_ADDRESS 0x9

boolean last_state = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);
  
  Wire.begin(THIS_ADDRESS);
  Wire.onReceive(receiveEvent);
  Serial.println("Setup ran");
}

void loop() {
  
    delay(12);
    Wire.beginTransmission(OTHER_ADDRESS);
    Wire.write(LOW);
    Serial.println(LOW);
    Wire.endTransmission();
    delay(12);
    Wire.beginTransmission(OTHER_ADDRESS);
    Wire.write(HIGH);
    Serial.println(HIGH);
    Wire.endTransmission();
      
  if (digitalRead(BUTTON) != last_state){
    last_state = digitalRead(BUTTON);
    Wire.beginTransmission(OTHER_ADDRESS);
    Wire.write(last_state);
    Serial.println(last_state);
    Wire.endTransmission();
  }
}

void receiveEvent(int howMany){
  while (Wire.available() > 0){
    boolean b = Wire.read();
    Serial.print(b, DEC);
    digitalWrite(LED, !b);
  }
  Serial.println(); 
}
