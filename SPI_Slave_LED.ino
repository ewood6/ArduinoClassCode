//This code is for the slave and paired with a master arduino through the SPI bus.
//When text is sent from the master, the slave captures each character and forms 
//a full message which is then printed out.

#include <SPI.h>  //Must include the SPI library
int LEDpin=8; //pin to light the LED when the message is being recieved.
char buf [100]; //This buffer compiles the message into a coherent whole.
volatile byte pos;  //A counter that says what position in the buffer we are at.
volatile boolean process_it; //Tells when message is complete

void setup (void) {
  Serial.begin (9600);
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,LOW);
  pinMode(MISO, OUTPUT); //have to send on master in, *slave out*
  
  SPCR |= _BV(SPE); //turn on SPI in slave mode
  
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;

  SPI.attachInterrupt(); // now turn on interrupts
}
// main loop - wait for flag set in interrupt routine below
void loop (void) {
  if (process_it){  
    digitalWrite(LEDpin, HIGH);
    buf [pos] = 0;  
    Serial.println (buf);
    pos = 0;
    process_it = false;
  }  // end of flag set
    delay(500);
    digitalWrite(LEDpin, LOW);
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
  byte c = SPDR;  // grab byte from SPI Data Register
 
  if (pos < sizeof buf){  // add to buffer if room
    buf [pos++] = c;
    
    if (c == '\n'){  //newline means time to process buffer
      process_it = true;
    }
  }
}
