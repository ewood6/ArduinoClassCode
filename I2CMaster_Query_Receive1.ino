//Code for the master in a master slave pair for I2C.
//When a button is pressed on the master arduino, it queries the slave.
//The slave then sends sensor data back to the master which is printed.

#include <Wire.h>   //Must include the I2C library

#define LED 9
int BUTTON=10;

//Addresses for the master and slave. They must be swapped when uploading to the slave.
#define THIS_ADDRESS 0x8
#define OTHER_ADDRESS 0x9

int b=0;
int data[16];
boolean last_state = HIGH;

void setup() {
  Serial.begin(9600);  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);
  
  Wire.begin(THIS_ADDRESS);  //Starts the I2C communication between the arduinos
}

void loop() {
  if (digitalRead(BUTTON) == LOW){      //Is the button pressed?
    last_state = digitalRead(BUTTON);
    Serial.println(last_state);
    //Sends a 1 to the slave as a command to sample the sensor.
    Wire.beginTransmission(OTHER_ADDRESS);
    Wire.write(1);
    Wire.endTransmission();
    //******************************************
    delay(1000);    //Important delay to give slave time to sample sensor.
    requestData();  //function for requesting data from the slave
  }
}

void requestData(){
  int count=Wire.requestFrom(OTHER_ADDRESS, 16);  //Requests 16 bytes of data from slave
  if(count==16){             //If data is appropriate size, read the data.
    for(int i=0;i<16;i++){   //Reads the data one byte at a time.
      b=Wire.read();
      data[i]=b;             //Stores the bytes.
    }
    Serial.println();
    for(int i=0;i<8;i++){                             //Convert data to temperature.
      float temperature = data[i]+(data[i+8]/100.0);
      Serial.print(temperature);
      Serial.println();
    }
  }
  else{                         //If data was not appropriate size.
    Serial.println("Data was not received");
  }
}
