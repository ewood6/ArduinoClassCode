//For a slave in a master slave pair of arduinos. 
//The master requests sensor data from the slave. This slave reads from a temperature
//sensor and then sends the data back to the master

#include <Wire.h>  //Must include the I2C library
#define LED 9
//Addresses for the Slave and Master.
#define THIS_ADDRESS 0x9
#define OTHER_ADDRESS 0x8
int b=0;
byte val[]={16};  //Array for sending the first 16 bytes of data to the Master Arduino            

float temperature = 0.0;
float temperatureC = 0.0;
float temperatureF = 0.0;
int temperatureFfirst = 0;
int temperatureFsecond = 0;
#define temperaturePin A0 //Just like in the previous Temp Meas sketch     
 
int inputoriginal = 0.0;
double inputconverted = 0.0;
 
void setup() {
  Serial.begin(9600);           // start serial for output
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  Wire.begin(THIS_ADDRESS);   //Begins the I2C communication between the arduinos
  Wire.onReceive(receiveEvent);  //Defines a function that interrupts when data is received
  Wire.onRequest(requestEvent);  //Defines a function that interrupts when a request is 
                                                              //received
}
void loop() {
  if(b==1){
    digitalWrite(LED,HIGH);    //Turns on LED
    for(int i=0;i<8;i++){
      TempMeas();
      val[i]=temperatureFfirst;
      val[i+8]=temperatureFsecond;
    }
    b=0;
  }
}
//********************************************************************
//*********************************************************************
//function that runs when Master Sends data to this Arduino
void receiveEvent(int howMany){
  while(Wire.available()){    
    b = Wire.read();        //While data is comming in, read values into b
  }
  
}
//*********************************************************************
//function that runs when Master sends request for data
void requestEvent(){
  delay(20);  //Wait a bit, not needed?
  Wire.write (val,16);  //Send 16 bytes of data to master
  digitalWrite(LED,LOW);     //Turn off LED
}
//*********************************************************************
//function that measures the temperature from the sensor
void TempMeas(){
  delay(10);
  inputoriginal = 0.0;
  temperature = 0.0;
  temperatureF = 0.0;
  temperatureFfirst = 0;
  temperatureFsecond = 0;
  inputoriginal = analogRead(temperaturePin);
  temperature = inputoriginal * .004882814;
  temperatureF = (temperature - .5) * 100;   //converting from 10 mv per degree wit 500 mV offset
  temperatureFfirst = temperatureF;
  temperatureFsecond = 100*(temperatureF - temperatureFfirst);
  //These serial prints are not necessary, but good to see if data is correct before transmission.
      Serial.print(inputoriginal);
      Serial.print(",");
      Serial.print(temperatureF);
      Serial.print(",");
      Serial.print(temperatureFfirst);
      Serial.print(",");
      Serial.println(temperatureFsecond);

}
