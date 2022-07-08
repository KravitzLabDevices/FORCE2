/*
  FORCE example code
  by Lex Kravitz and Bridget Matikainen-Ankney

  alexxai@wustl.edu
  May, 2021
  
 KravitzLabDevices/FORCE_library is licensed under the GNU General Public License v3.0

 Permissions of this strong copyleft license are conditioned on making available complete source code of licensed works 
 and modifications, which include larger works using a licensed work, under the same license. Copyright and license 
 notices must be preserved. Contributors provide an express grant of patent rights.
  
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        DON'T EDIT THESE LINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Force.h>                                      //Include FORCE library 
String ver = "Force";                                  //unique identifier text
Force force(ver);                                       //start FORCE object


void setup() {
  force.begin();                                        //setup FORCE
  force.trial_available = false;
  force.LeftActive = true;
  force.RightActive = true;
  Serial.begin(9600);
}

void loop() {  
  force.run();                                          //call force.run() at least once per loop
  if (digitalRead(POKE) == LOW) {
    force.Tone();
    force.trial_available = true;
    force.trial_start = millis();
    force.trial_length = millis() - force.trial_start;
    force.Tare();
    while (force.trial_length < force.trial_window) {
      force.trial_length = millis() - force.trial_start;
      Serial.println(force.trial_length);
      force.run();
      force.SenseLeft();
      if (force.pressLengthLeft > force.hold_timeLeft) {
        //force.Tone(2000, 200);
        force.DispenseLeft();
        force.pressesLeft = 0;                                                                         
        force.Timeout(force.timeout_length);
        }
      force.SenseRight();
      if (force.pressLengthRight > force.hold_timeRight) {
        //force.Tone(500, 200);
        force.DispenseRight();
        force.pressesRight = 0;
        force.Timeout(force.timeout_length);
      }
    force.trial_available = false;
    
    }  
  }
}
