//This code is for the master in the SPI pair with a slave. It communicates text from the master
//to the slave and the slave prints the text. This transfer occurs when the button is pressed
//on the master's circuit.

#include <SPI.h> //must include the spi library
int SSpin=43; //Slave Select pin. Used to tell the slave to listen to the message
int ButtonPin=22; //This pin provides power to the button
int ButtonRead=A15; //This pin detects the button press

void setup (void) {
  Serial.begin(9600);
  pinMode(SSpin, OUTPUT);
  pinMode(ButtonPin, OUTPUT);
  digitalWrite(SSpin, HIGH);  // ensure SS stays high for now
  digitalWrite(ButtonPin, HIGH);
  // SPI.begin() Puts SCK, MOSI, SS pins into output mode also puts SCK, MOSI into LOW state, 
  // and SS into HIGH state. Then puts SPI hardware into Master mode and turn SPI on
  SPI.begin ();

  // Slow down the master a bit for less chance of lost data.
  SPI.setClockDivider(SPI_CLOCK_DIV8); 
}

void loop (void) {
  if(analogRead(ButtonRead)<900){
    char c;
    digitalWrite(SSpin, LOW); // enable Slave Select
    // send test string. It is broken in to characters and sent individually
    for (const char * p = "Hello, world!\n" ; c = *p; p++){
      SPI.transfer (c);
    }
    digitalWrite(SSpin, HIGH); // disable Slave Select
    delay (500);
  } 
}
