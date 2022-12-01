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

int prob_left = 100;                                     // Probability of pellet on the high-reward poke                                               
int prob_right = 100;                                    // Probability of pellet on the low-reward poke
int trialsToSwitch = 20;                                // # of trials before probabilities on the pokes switch
int trial_counter = 0;                                 // Tracks how many pellets have been obtained in this set of probabilities
int highp_counter = 0;                                  // Tracks how many pokes in a row in high probability poke
int trialTimeout = 5;                                   //timeout duration after each poke, set to 0 to remove the timeout
int probs[5] = {10, 30, 50, 70, 90};                
bool press = false;

void setup() {
  force.log_lite = true;
  force.begin(force.log_lite);                                        //setup FORCE
  force.trial_window = 60000;
  force.ver = prob_left;
  force.FRC = prob_right;
  force.trials_per_block = trialsToSwitch;
}

void loop() {  

  ///////////////////////////////////////////////////////////////////////
  // This is the non-stationary part of the task                      ///
  // meaning that probabilities change every trialsToSwitch trials    ///
  ///////////////////////////////////////////////////////////////////////
  force.run();
  if(trial_counter == trialsToSwitch) {
    highp_counter = 0;
    trial_counter = 0;
    prob_left = probs[random(0,5)];
    prob_right = 100-prob_left;
    force.ver = prob_left;
    force.FRC = prob_right;
  }

  else if (highp_counter == 8) {
    highp_counter = 0;
    trial_counter = 0;
    prob_left = probs[random(0,5)];
    prob_right = 100-prob_left;
    
    force.ver = prob_left;
    force.FRC = prob_right;
    //force.dispense_delay = highp_counter;
    force.dispense_amount = trial_counter;
  }

  /////////////////////////////////////////////////////////////////
  /// Here we start the task. Mouse has to poke on the left    ////
  /// to start a trial, and then the bandit task starts        ////
  /// within 5 seconds of the start of the tone                ///
  ////////////////////////////////////////////////////////////////
  force.readPoke();
  if (force.poke) {
    force.run();
    force.event = "CENTER";
    force.logdata_lite();
    force.trial_start = millis();
    force.Tone();
    force.trial_available = true;
  }
    
  while (((millis()-force.trial_start) < force.trial_window) && force.trial_available) {
    force.run();

    /////////////////////////////////////////////////////////////
    ////          If mouse presses the left lever             ///
    /////////////////////////////////////////////////////////////    
    if ((force.pressLengthLeft > force.hold_timeLeft) && force.LeftActive) {
      if (prob_left > 50) {
        highp_counter ++;
      }
      else {
        highp_counter = 0;
      }
      //force.dispense_delay = highp_counter;
      force.loglite_Left();
      
      if (random(100) < prob_left) {
        force.event = "RewardLeft";
        force.DispenseLeft();
        trial_counter ++;
        force.dispense_amount = trial_counter;
        force.logdata_lite();
      }
      else {
        force.Tone(300,600);
      }
      press = true;
      force.trial_available = false;
    }

    //////////////////////////////////////////////////////////////
    ////          If mouse presses the right lever            ////
    ///////////////////////////////////////////////////////////// 
    if ((force.pressLengthRight > force.hold_timeRight) && force.RightActive) {
      if (prob_right > 50) {
        highp_counter ++;
      }
      else {
        highp_counter = 0;
      }
      //force.dispense_delay = highp_counter;
      force.loglite_Right();

      if (random(100) < prob_right) {
        force.event = "RewardRight";
        force.DispenseRight();
        trial_counter ++;
        force.dispense_amount = trial_counter;
        force.logdata_lite();
        
      }
      else {
        force.Tone(300,600);
      }
      press = true;
      force.trial_available = false;
    }
  }
      
  //////////////////////////////////////////////////////////////
  ////     If there was no press during the time window     ////
  ///////////////////////////////////////////////////////////// 
  if (force.trial_available && press == false) {
    force.Tone(300,600);
    force.trial_available = false;
  }

  //////////////////////////////////////////////////////////////
  ////   Finish trial and start inter-trial timeout         ////
  ///////////////////////////////////////////////////////////// 
  if (press) {
    press = false;
    force.trial_available = false;
    force.Timeout(trialTimeout);
  }

}
