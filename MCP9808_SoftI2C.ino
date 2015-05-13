//Sketch to read the MCP9808 using the SoftI2CMaster library. Adapted from BMA example.
//This library has many advantages over the hardware version, including ability to make
//multiple I2C buses on the same Arduino, embedded timeout functions to prevent a less than 
//perfect bus from freezing when a packet is dropped, and ability to alter transmission speeds.

// use low processor speed (you have to change the baud rate to 2400!) 
// #define I2C_CPUFREQ2 (F_CPU/8)
#define NO_INTERRUPT 0     //?
#define I2C_TIMEOUT 100    //wait 100ms for a response. If none, then break and restart
#define I2C_SLOWMODE 1     //operates at 25kHz no matter the CPU speed
#define FAC 1              //variable to use on the next line
#define I2C_CPUFREQ (F_CPU/FAC)  //ability to scale the CPU speed used by I2C

int a = 0;

int I2Cpin1 = 22;
int I2Cpin2 = 23;

//Pins you use for I2C (SDA and SCL) must be defined using the port and pin number
#define SDA_PORT PORTC  //includes pins 30-37
#define SDA_PIN 7       //pin 30
#define SCL_PORT PORTC  //includes pins 30-37
#define SCL_PIN 6       //pin 31

#include <SoftI2CMaster.h>  //Must include the I2C software library
#include <avr/io.h>         //Required for I2C soft

#define BMAADDR 0x30        //Address for the MCP9808

int xval, yval, zval;
float zval1;

/*
* Example of how to add a second I2C bus to the same board.
* In addition to the following lines, the library must be altered as seen in 
* SoftI2CMaster2.h, each variable must be altered because the variable names
* cannot be used in two different libraries. I did this by adding a '2' to
* each name. This can be done as many times as you have pins available for buses.
#define NO_INTERRUPT2 0
#define I2C_TIMEOUT2 100
#define I2C_SLOWMODE2 1
#define FAC2 1
#define I2C_CPUFREQ2 (F_CPU/FAC2)

#define SDA_PORT2 PORTC
#define SDA_PIN2 5
#define SCL_PORT2 PORTC
#define SCL_PIN2 4

#include <SoftI2CMaster2.h>
*/
//************************************************************
//Slows down the processor. Not sure if needed.
void CPUSlowDown(int fac2) {
  // slow down processor by a fac2
    CLKPR = _BV(CLKPCE);
    CLKPR = _BV(CLKPS1) | _BV(CLKPS0);
}
//***********************************************************  
//Writes initialization commands to the MCP9808, telling it that
//we will be sampling tempterature as normal.
boolean setControlBits(uint8_t cntr) {
  Serial.println(F("Soft reset"));
  if (!i2c_start(BMAADDR | I2C_WRITE)) {  //if it cannot communicate, false
    return false;
  }
  if (!i2c_write(0x0A)) {  //If it could not write command, false
    return false;
  }
  if (!i2c_write(cntr)) {  //if it could not write command, false
    return false;
  }
  i2c_stop();       //stops i2c for the time being
  return true;
}
//**********************************************************
//Initialize MCP9808. Calls setControlBits above
boolean initBma(void) {
  if (!setControlBits(B00000010)) return false;;
  delay(100);
  return true;
}
//*********************************************************
//read a value from the MCP9808 in the form of two 8bit bytes
//The two are then combined into a 16bit byte.
int readOneVal(boolean last) {
  uint16_t msb, lsb;           //define two unsigned ints; msb and lsb
  msb = i2c_read(false);       //store first 8bits into msb
  msb <<= 8;                   //shift those 8 bits to beginning of msb
  msb |= i2c_read(last);       //read last 8bits and concatanate with first 8bits
  if (last) i2c_stop();        //if reached end of values, stop I2C
  return (int)(msb);           //return the 16bit value
}
/*
//**********************************************************
//If a second i2c bus is used, this will read from it, same as above
int readOneVal2(boolean last) {
  uint16_t msb, lsb;
  msb = i2c_read2(false);
  msb <<= 8;
  msb |= i2c_read2(last);
  if (last) i2c_stop2();
  return (int)(msb);
}
*/
//***********************************************************
//Reads values from MCP9808 via I2C and stores values
boolean readBma(void) {
  zval = 0xFFFF;          //variable for storing bytes from MCP9808
  zval1 = 0xFFFF;         //variable for conversion of byte to temperature
  //If I2C cannot communicate with sensor, return false
  if (!i2c_start(BMAADDR | I2C_WRITE)) return false;
  if (!i2c_write(0x05)) return false;
  if (!i2c_rep_start(BMAADDR | I2C_READ)) return false;
  zval = readOneVal(true);  //calls readOneVal above
  zval1 = zval & 0x0FFF;    //bitwise operation for conversion to temperature
  zval1 = zval1/16.0;       //final conversion operation
  return true;
}
/*
//*************************************************************
//Reads values from second I2C bus if used. same as above
boolean readBma2(void) {
  zval = 0xFFFF;
  zval1 = 0xFFFF;
  if (!i2c_start2(BMAADDR | I2C_WRITE2)) return false;
  if (!i2c_write2(0x05)) return false;
  if (!i2c_rep_start2(BMAADDR | I2C_READ2)) return false;
  zval = readOneVal2(true);
  zval1 = zval & 0x0FFF;
  zval1 = zval1/16.0;
  return true;
}
*/
//**************************************************************
//Setup
void setup(void) {
  Serial.begin(9600); // in case of CPU slow down, change to baud rate / 8!
  #if FAC != 1
    CPUSlowDown(FAC);
  #endif
  Serial.println(F("Intializing 0..."));
  Serial.print("I2C0 delay counter: ");
  Serial.println(I2C_DELAY_COUNTER);
  if (!i2c_init()) 
    Serial.println(F("Initialization error0. SDA or SCL are low"));
  else
    Serial.println(F("...done0"));
  /*
  //needed only if using a second I2C bus
  #if FAC2 != 1
    CPUSlowDown(FAC2);
  #endif
  Serial.println(F("Intializing2 ..."));
  Serial.print("I2C delay counter2: ");
  Serial.println(I2C_DELAY_COUNTER2);
  if (!i2c_init2()) 
    Serial.println(F("Initialization error2. SDA or SCL are low"));
  else
    Serial.println(F("...done2"));
  */
  //Needed?  
  pinMode(I2Cpin1,OUTPUT);
  pinMode(I2Cpin2,OUTPUT);
  digitalWrite(I2Cpin1, HIGH);
  digitalWrite(I2Cpin2, LOW);
  //?
}
//********************************************************
//Main loop
void loop(void){
  digitalWrite(I2Cpin1, HIGH);   //Needed?
  digitalWrite(I2Cpin2, LOW);    //Needed?
  a++;                           //Needed?
  
  if (!readBma()) Serial.println(F("READ ERROR0"));  //If read fails,
  //Print the values
  Serial.print("time= ");
  float time=millis()/1000.0;
  Serial.print(time);
  Serial.print("     ");
  Serial.print(F("  Z1="));
  Serial.println(zval1);
  delay(150);
  
  /* Needed only if using a second I2C bus
  digitalWrite(I2Cpin2, HIGH);    //Needed?
  digitalWrite(I2Cpin1, LOW);     //Needed?

  if (!readBma2()) Serial.println(F("READ ERROR2"));
  Serial.print("time= ");
  time=millis()/1000.0;
  Serial.print(time);
  Serial.print("     ");
  Serial.print(F("  Z2="));
  Serial.println(zval1);
  delay(150);
  */
}
