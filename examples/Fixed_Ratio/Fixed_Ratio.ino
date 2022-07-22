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
  force.trial_window = 60000;
  force.run(false);                                          //call force.run() at least once per loop
  force.readPoke();
  if (force.poke) {
    force.run(true);
    force.Tone();
    force.trial_start = millis();
    force.trial_length = millis() - force.trial_start;
    while (force.trial_length < force.trial_window) {
      force.run(false);
      force.trial_length = millis() - force.trial_start;
      if ((force.pressLengthLeft > force.hold_timeLeft) && (force.LeftActive)) {
        force.run(false);
        force.DispenseLeft();                                                        
        force.Timeout(force.timeout_length);
        }
      if ((force.pressLengthRight > force.hold_timeRight) && (force.RightActive)) {
        force.run(false);
        force.DispenseRight();
        force.Timeout(force.timeout_length);
      }
    }  
  }
}
