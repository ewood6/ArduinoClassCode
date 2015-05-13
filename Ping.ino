/* YourDuino SKETCH UltraSonic Serial 1.0
 Runs HC-04 and hopefully SRF-06 Ultrasonic Modules
 Uses: Ultrasonic Library (Copy to Arduino Library folder)
 http://iteadstudio.com/store/images/produce/Sensor/HCSR04/Ultrasonic.rar
 terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include "Ultrasonic.h"
/*-----( Declare Constants and Pin Numbers )-----*/
#define  TRIG_PIN  13
#define  ECHO_PIN  12
/*-----( Declare objects )-----*/
Ultrasonic OurModule(TRIG_PIN, ECHO_PIN);
/*-----( Declare Variables )-----*/

void setup()   /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(9600);
  Serial.println("UltraSonic Distance Measurement");
  Serial.println("YourDuino.com  terry@yourduino.com");

}//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  Serial.print(OurModule.Ranging(CM));
  Serial.print("cm   ");
  delay(100);  //Let echos from room dissipate
  Serial.print(OurModule.Ranging(INC));
  Serial.println("inches");

  delay(400);

}//--(end main loop )---

