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
  force.RightActive = false;
}

void loop() {  
  force.run();                                          //call force.run() at least once per loop
  if (POKE == HIGH) {
    force.Tone();
    force.trial_available = true;
    trial_start = millis();
    force.Tare();
    while (millis()-trial_start < trial_window) {
      force.run()
      force.SenseLeft();
      force.SenseRight();
      if ((force.pressLengthLeft > force.hold_timeLeft) && (LeftActive == true)) {
        force.pressesLeft ++;
        if (force.pressesLeft == force.ratioLeft) {
          force.DispenseLeft();
          force.pressesLeft = 0
          }
        }
      if ((force.pressLengthRight > force.hold_timeRight) && (RightActive == true)) {
        force.pressesRight ++;
        if (force.pressesRight == force.ratioRight) {
          force.DispenseRight();
          force.pressesRight = 0
          }
        }
      }
    force.trial_available = false
    force.Timeout(force.timeout_length);  
  }
}
