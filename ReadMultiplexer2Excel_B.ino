//Code for running through each of the pins 0-7 on a 8-way multiplexer (like a 4067).
//Reads all values from possible input and stores them in an array and prints to computer.

int analogPin = 0;     // Could be any analog pin
int val = 0;           // variable to store the value read
int row = 0;
double sec = 0.0;

//Array for storing values read from sensors connected to multiplexer
byte Values[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  

//Integers for storing bits. These bits become the HIGH or LOW values 
//sent to the pins for activating the S0,S1,S2 pins on the multiplexter
int r0=0;
int r1=0;
int r2=0;

int i=0;     //Counter for loops


void setup()
{
  //Pins for activating S0,S1,S2 on the multiplexter. Can be any digital pins.
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  
  Serial.begin(9600);          //  setup serial
  Serial.println("CLEARDATA");  //For use with PLXDAQ
  Serial.println("LABEL,Time,Sec,Label1,Label2,Label3");
}
void loop()
{
  for (int i=0;i<=7;i++){  //Loop for writing to S0,S1,S2 on multiplexer
    r0=bitRead(i,0);
    r1=bitRead(i,1);
    r2=bitRead(i,2);
    digitalWrite(2,r0);
    digitalWrite(3,r1);
    digitalWrite(4,r2);
    delay(50);
    Values[i] = analogRead(analogPin);    // read the input pin
  }

  Serial.print("DATA,TIME,");
  sec=millis()/1000.0;
  Serial.print(sec);
  Serial.print(",");
  for (int i=0;i<=7;i++){    //Print all analog read values.
    Serial.print(Values[i]);
    Serial.print(",");
  }
  Serial.println(" ");
  row++;
  delay(10);
}
