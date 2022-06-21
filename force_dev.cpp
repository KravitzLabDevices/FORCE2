


void Force::start_deterministic() {
  start_timer = 0;
  trial_available = true;
  Tare();
  
  if (millis() - start_timer > trial_window) {
    trial_available = false
    }
  
  if (trial_available & (pressLength > hold_time)) {
    presses ++;
    if (presses == ratio) {
      Tone(500, 200);
      DispenseLeft();
      Timeout();
      presses = 0
      }
    else {
      Tone(1000,200);
      
      }
    
    }
    
  
  if (trial_available & pressLength > hold_time)
   
  }
  

///////////////////////////
///////Disepense Left//////
//////////////////////////

void Force::DispenseLeft() {
  dispensing = true;
  trial++;
  Tone();
  float successTime = millis();
  while ((millis() - successTime) < (dispense_delay * 1000)){
    tft.setCursor(85, 44);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Delay:");
    tft.setTextColor(ST7735_WHITE);
    tft.print((-(millis() - successTime - (dispense_delay*1000))/ 1000),1);
    run();
    tft.fillRect(84, 43, 80, 12, ST7735_BLACK); // remove Delay text when timeout is over
    if (grams > 1 or grams2 >1){ //only clear F1 and F2 values if levers are being pushed
      tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after label
    }
  }
  //digitalWrite(A2,HIGH); //A2 will be "reward dispensed" pin
  //digitalWrite(13,HIGH); // RED LED
  for (int i=0; i < 20; i++) {
    digitalWrite(PUMP1, HIGH);
    delayMicroseconds(100);
    digitalWrite(PUMP1, LOW);
  }
  DateTime now = rtc.now();
  dispenseTime = now.unixtime();
  //digitalWrite(A2, LOW);
  //digitalWrite(13, LOW);
  pressTime = millis();
  pressLength = 0;
  dispensing = false;
}

///////////////////////////
//////Disepense Right//////
//////////////////////////

void Force::DispenseRight() {
  dispensing = true;
  trial++;
  Tone();
  float successTime = millis();
  while ((millis() - successTime) < (dispense_delay * 1000)){
    tft.setCursor(85, 44);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Delay:");
    tft.setTextColor(ST7735_WHITE);
    tft.print((-(millis() - successTime - (dispense_delay*1000))/ 1000),1);
    run();
    tft.fillRect(84, 43, 80, 12, ST7735_BLACK); // remove Delay text when timeout is over
    if (grams > 1 or grams2 >1){ //only clear F1 and F2 values if levers are being pushed
      tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after label
    }
  }
  //digitalWrite(A2,HIGH); //A2 will be "reward dispensed" pin
  //digitalWrite(13,HIGH); // RED LED
  for (int i=0; i < 20; i++) {
    digitalWrite(PUMP2, HIGH);
    delayMicroseconds(100);
    digitalWrite(PUMP2, LOW);
  }
  DateTime now = rtc.now();
  dispenseTime = now.unixtime();
  //digitalWrite(A2, LOW);
  //digitalWrite(13, LOW);
  pressTime = millis();
  pressLength = 0;
  dispensing = false;
}

///////////////////////////
////////Sense Left/////////
//////////////////////////

void Force::SenseLeft() {
  gramsLeft = (scaleLeft.get_units());
  //grams2 = (scale2.get_units());
  if (gramsLeft < 0) gramsLeft = 0;
  //if (grams2 < 0) grams2 = 0;
  
  if (gramsLeft < reqLeft){
    pressTimeLeft = millis();
    pressLengthLeft = 0;
  }
  
  if (grams > reqLeft) {
    pressLengthLeft = (millis() - pressTime);
  }
  
    
  outputValue = map(gramsLeft, 0, 200, 0, 4095);
  //outputValue2 = map(grams2, 0, 200, 0, 4095);
 
  if (outputValue > 4000) outputValue = 4000;
  if (outputValue < 1) outputValue = 0;
  //if (outputValue2 > 4000) outputValue2 = 4000;
  //if (outputValue2 < 1) outputValue2 = 0;

  //analogWrite(A0, outputValue2);
  //analogWrite(A1, outputValue);
  
  scaleChange += abs(outputValue - lastReading);
  //scaleChange2 += abs(outputValue2 - lastReading2);

  lastReading = outputValue;
  //lastReading2 = outputValue2;
  
  //control pixel color based on load cells 
  //pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  //pixels.show();

  lick = digitalRead(18) == HIGH;
  Tare();
  check_buttons();
}

///////////////////////////
////////Sense Right////////
//////////////////////////

void Force::SenseRight() {
  gramsRight = (scaleRight.get_units());
  //grams2 = (scale2.get_units());
  if (gramsRight < 0) gramsRight = 0;
  //if (grams2 < 0) grams2 = 0;
  
  if (gramsRight < reqRight){
    pressTimeRight = millis();
    pressLengthRight = 0;
  }
  
  if (gramsRight > reqRight) {
    pressLengthRight = (millis() - pressTimeRight);
  }
  
    
  outputValue = map(grams, 0, 200, 0, 4095);
  outputValue2 = map(grams2, 0, 200, 0, 4095);
 
  if (outputValue > 4000) outputValue = 4000;
  if (outputValue < 1) outputValue = 0;
  //if (outputValue2 > 4000) outputValue2 = 4000;
  //if (outputValue2 < 1) outputValue2 = 0;

  //analogWrite(A0, outputValue2);
  //analogWrite(A1, outputValue);
  
  scaleChange += abs(outputValue - lastReading);
  //scaleChange2 += abs(outputValue2 - lastReading2);

  lastReading = outputValue;
  lastReading2 = outputValue2;
  
  //control pixel color based on load cells 
  pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  pixels.show();

  lick = digitalRead(18) == HIGH;
  Tare();
  check_buttons();
}