//MouseLEDs for Processing
//This code captures the coordinates of the mouse and sends them through the serial
//port to the Arduino. 

import processing.serial.*;
import cc.arduino.*;

// Global variables
int new_sX, old_sX;
int nX, nY;
Serial myPort;

// Setup the Processing Canvas
void setup(){
  size( 800, 400 );
  strokeWeight( 10 );
 
  //Open the serial port for communication with the Arduino
  //Make sure the COM port is correct
  myPort = new Serial(this, "COM14", 9600);
  myPort.bufferUntil('\n'); 
}

// Draw the Window on the computer screen
void draw(){
  
  // Fill canvas grey
  background( 100 );
    
  // Set the stroke colour to white
  stroke(255); 
  
  // Draw a circle at the mouse location
  ellipse( nX, nY, 10, 10 );

  //Draw Line from the top of the page to the bottom of the page
  //in line with the mouse.
  line(nX,0,nX,height);  
}


// Get the new mouse location and send it to the arduino
void mouseMoved(){
  nX = mouseX;
  nY = mouseY; 
  
  //map the mouse x coordinates to the LEDs on the Arduino.
  new_sX=(int)map(nX,0,800,0,10);

  if(new_sX==old_sX){
    //do nothing
  } else {
    //only send values to the Arduino when the new X coordinates are different.
    old_sX = new_sX;
    myPort.write(""+new_sX);
  }
}
