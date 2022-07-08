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
}

void loop() {  
  force.run();                                          //call force.run() at least once per loop
  force.readPoke();
  if (poke) {
    force.Tone();
    force.trial_available = true;
    force.trial_start = millis();
    force.Tare();
    while (millis()-(force.trial_start < force.trial_window)) {
      force.run();
      force.SenseLeft();
      if (force.pressLengthLeft > force.hold_timeLeft) && (force.LeftActive){
        force.DispenseLeft();
        force.pressesLeft = 0;
        force.Timeout(force.timeout_length);
        }
      force.SenseRight();
      if (force.pressLengthRight > force.hold_timeRight) && (force.RightActive){
        force.DispenseRight();
        force.pressesRight = 0;
        force.Timeout(force.timeout_length);
      }
    force.trial_available = false;
    }  
  }
}
