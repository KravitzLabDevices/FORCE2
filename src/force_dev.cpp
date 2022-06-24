#include "Arduino.h"
#include "Force.h"

//SETUP QSPI FLASH
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
File myFile;

/////////////////////////////////////////////////////////////////////////
// Initialize FORCE!
/////////////////////////////////////////////////////////////////////////
Force::Force(String ver) {
  library_version = ver; 
}

/////////////////////////////////////////////////////////////////////////
// RTC Functions 
/////////////////////////////////////////////////////////////////////////
RTC_PCF8523 rtc;

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

/////////////////////////////////////////////////////////////////////////
// Load from settings.txt on SPI flash
/////////////////////////////////////////////////////////////////////////
void Force::load_settings() {
  Serial.println("*****************************");
  Serial.println("Loading device Settings:");
  //read settings from SPI flash
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    calibrated = true;
    Serial.println ("settings.txt found. Contents:");
    while (myFile.available()) {
      for (int i = 0; i < 12; i++) {
        settings_recalled[i] = myFile.parseInt();
        Serial.println(settings_recalled[i]);
      }
      myFile.read();
      // close the file:
      myFile.close();

      FRC = settings_recalled[0];
      reqLeft = settings_recalled[1];
      reqRight = settings_recalled[2];
      dispense_amount = 2000;
      dispense_delay = settings_recalled[4];
      timeout_length = settings_recalled[5] ;
      ratioLeft = settings_recalled[6];
      ratioRight = settings_recalled[7];
      hold_timeLeft = settings_recalled [8];
      hold_timeRight = settings_recalled[9];
      calibration_factor_Left = settings_recalled[10];
      calibration_factor_Right = settings_recalled[11];
      PR = settings_recalled[12];
      trials_per_block = settings_recalled[13];
      max_force = settings_recalled[14];
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// Save to settings.txt on SPI flash
/////////////////////////////////////////////////////////////////////////
void Force::save_settings() {
  Serial.println("*****************************");
  Serial.println("Saving device Settings:");
  //open and delete the settings file
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  if (myFile) {
    Serial.print ("settings.txt found, deleting.... ");
    myFile.remove();
    myFile.close();
    Serial.println ("done.");
  }

  settings[0] = FRC;
  settings[1] = reqLeft;
  settings[2] = reqRight;
  settings[3] = dispense_amount;
  settings[4] = dispense_delay;
  settings[5] = timeout_length;
  settings[6] = ratioLeft;
  settings[7] = ratioRight;
  settings[8] = hold_timeLeft;
  settings[9] = hold_timeRight;
  settings[10] = calibration_factor_Left;
  settings[11] = calibration_factor_Right;
  settings[12] = PR;
  settings[13] = trials_per_block;
  settings [14] = max_force;

  //rewrite settings file
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  Serial.print ("re-creating settings.txt file.");
  if (myFile) {
    for (int i = 0; i < 12; i++) {
      myFile.print(settings[i]);   // These are my settings
      myFile.print(",");   // These are my settings
      Serial.print(".");
    }
    myFile.close();
    Serial.println("done.");
  }
  
  //reopen and read back file on QSPI flash 
  Serial.print("Reading settings.txt back...");
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("opened...contents: ");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  }
}

/////////////////////////////////////////////////////////////////////////
// reset settings
/////////////////////////////////////////////////////////////////////////
void Force::reset_settings() {
  Serial.println("*****************************");
  Serial.println("Reseting device settings:");
  FRC = 1;
  reqLeft = 2;
  reqRight = 2;
  dispense_amount = 4;
  dispense_delay = 4;
  timeout_length = 10;
  ratioLeft = 1;
  ratioRight = 1;
  hold_timeLeft = 350;
  hold_timeRight = 350;
  calibration_factor_Left = -3300;
  calibration_factor_Right = -3300;
  PR = 0;
  trials_per_block = 10;
  max_force = 20;
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(40, 35);
  tft.setTextColor(ST7735_WHITE);
  tft.println("Settings reset");
  start_up_menu();
}

/////////////////////////////////////////////////////////////////////////
// print settings
/////////////////////////////////////////////////////////////////////////
void Force::print_settings() {
  Serial.println("*****************************");
  Serial.println("Printing local device settings:");
  Serial.print("Device#: "); Serial.println(FRC);
  Serial.print("Req_Left: "); Serial.println(reqLeft);
  Serial.print("Req_Right: "); Serial.println(reqRight);
  Serial.print("dispense_amount: "); Serial.println(dispense_amount);
  Serial.print("dispense_delay: "); Serial.println(dispense_delay);
  Serial.print("timeout_length: ");  Serial.println(timeout_length);
  Serial.print("ratio_Left: "); Serial.println(ratioLeft);
  Serial.print("ratio_Right: "); Serial.println(ratioRight);
  Serial.print("hold_time_Left: "); Serial.println(hold_timeLeft);
  Serial.print("hold_time_Right: "); Serial.println(hold_timeRight);
  Serial.print("calibration_factor_Left: "); Serial.println(calibration_factor_Left);
  Serial.print("calibration_factor_Right: "); Serial.println(calibration_factor_Right);
  if (PR==0) Serial.println("Fixed Ratio");
  if (PR==1) Serial.println("Prog Ratio");
  Serial.print ("Trials per block: "); Serial.println(trials_per_block);
  Serial.print ("Max force: "); Serial.println(max_force);
  Serial.println(" ");
  
  Serial.print("Reading from SPI flash...");
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("reading contents of settings.txt...");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  }
}


/////////////////////////////////////////////////////////////////////////
// Begin
/////////////////////////////////////////////////////////////////////////
void Force::begin() {
  Serial.begin(9600);

  if (!ss.begin()) {
    Serial.println("seesaw couldn't be found!");
    while (1);
  }

  // Initialize pins
  pinMode(A0, OUTPUT);
  pinMode(BEEPER, OUTPUT);
  
  pinMode(LICKOMETER1, INPUT_PULLDOWN);
  pinMode(LICKOMETER2, INPUT_PULLDOWN);
  pinMode(PUMP1, OUTPUT);
  digitalWrite(PUMP1, LOW);
  
  pinMode(LICKOMETER2, INPUT_PULLDOWN);
  pinMode(PUMP2, OUTPUT) ;
  digitalWrite(PUMP2, LOW); 

  // Initialize display
  ss.tftReset();                  // Reset the display
  ss.setBacklight(1);             // Adjust backlight (this doesn't really seem to work unless you do -1 to turn it off)
  tft.initR(INITR_MINI160x80);    // Initialize a ST7735S chip, mini display
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  // Initialize RTC
  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();

  // Initialize neopixel
  pixels.begin();

  //start SPI flash
  Serial.print("flash begin...");
  flash.begin();
  // Open file system on the flash
  if ( !fatfs.begin(&flash) ) {
    Serial.println("Error: filesystem is not existed. Please try SdFat_format example to make one.");
    while (1) yield();
  }
  Serial.println("done.");

  load_settings();

  // Initialize load cells
  analogWriteResolution(12);  // turn on 12 bit resolution
  
  scaleLeft.begin(DOUT1, CLK1);
  scaleLeft.tare();
  scaleLeft.set_scale(calibration_factor_Left);
  
  scaleRight.begin(DOUT2, CLK2);
  scaleRight.tare();
  scaleRight.set_scale(calibration_factor_Right);  

  //start up menu
  start_up_menu();
  tft.fillScreen(ST77XX_BLACK);

  // Initialize SD
  SdFile::dateTimeCallback(dateTime);
  CreateDataFile();
  writeHeader();

}



/////////////////////////////////////////////////////////////////////////
//Run function to updates things on every loop///////////////////////////
/////////////////////////////////////////////////////////////////////////
void Force::run() {
  UpdateDisplay();
  WriteToSD();
  DateTime now = rtc.now();
  unixtime  = now.unixtime();
  //SerialOutput();
}
  


/////////////////////////////////////////////////////////////////////////
// Buttons Functions 
/////////////////////////////////////////////////////////////////////////
void Force::check_buttons() {
  uint32_t buttons = ss.readButtons();
    
  if (! (buttons & TFTWING_BUTTON_A)) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 50)); //Light Neopixel blue
    pixels.show();
  }
  
 if ((! (buttons & TFTWING_BUTTON_A)) and ! (buttons & TFTWING_BUTTON_B)){
    delay (1000);
    uint32_t buttons = ss.readButtons();
    if ((! (buttons & TFTWING_BUTTON_A)) and ! (buttons & TFTWING_BUTTON_B)){
      pixels.setPixelColor(0, pixels.Color(50, 0, 0)); //Light Neopixel red
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 35);  
      tft.setTextColor(ST7735_WHITE);
      tft.println("Pump flush");   
      pixels.show();
      digitalWrite(PUMP1, HIGH);
      digitalWrite(PUMP2, HIGH);
      tft.setCursor(40, 50);  
      tft.print("500");
      delay (100);
      tft.print("400");
      delay (100);
      tft.print("300");
      delay (100);
      tft.print("200");
      delay (100);
      tft.print("1");
      delay (100);
      digitalWrite(PUMP1, LOW);
      digitalWrite(PUMP2, LOW);
      tft.fillScreen(ST77XX_BLACK);
    }
  }
}




/////////////////////////////////////////////////////////////////////////
/////////////////////////Timeout function////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void Force::Timeout(int timeout_length) {
  dispense_time = millis();
  while ((millis() - dispense_time) < (timeout_length * 1000)){
    tft.setCursor(85, 44);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Timeout:");
    tft.print((-(millis() - dispense_time - (timeout_length*1000))/ 1000),1);
    run();
    tft.fillRect(84, 43, 80, 12, ST7735_BLACK);
    if ((gramsLeft > 1.5) or (gramsRight > 1.5)) { //reset timeout if either lever pushed
      Timeout(timeout_length); 
      tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after F1 F2 labels
    }
  }
  tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after F1 F2 labels
}

/////////////////////////////////////////////////////////////////////////
// Sound Functions 
/////////////////////////////////////////////////////////////////////////
void Force::Tone(int frequency, int duration) {
  tone(BEEPER, frequency, duration);
}

void Force::Click() {
  tone(BEEPER, 800, 200);
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
    if (gramsLeft > 1 or gramsRight >1){ //only clear F1 and F2 values if levers are being pushed
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
  pressTimeLeft = millis();
  pressLengthLeft = 0;
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
    if (gramsLeft > 1 or gramsRight >1){ //only clear F1 and F2 values if levers are being pushed
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
  pressTimeRight = millis();
  pressLengthRight = 0;
  dispensing = false;
}

///////////////////////////
////////Sense Left/////////
//////////////////////////

void Force::SenseLeft() {
  gramsLeft = (scaleLeft.get_units());
  if (gramsLeft < 0) gramsLeft = 0;
  
  if (gramsLeft < reqLeft){
    pressTimeLeft = millis();
    pressLengthLeft = 0;
  }
  
  if (gramsLeft > reqLeft) {
    pressLengthLeft = (millis() - pressTimeLeft);
  }
  
    
 // outputValueLeft = map(gramsLeft, 0, 200, 0, 4095);
  //outputValue2 = map(grams2, 0, 200, 0, 4095);
 
  //if (outputValueLeft > 4000) outputValue = 4000;
  //if (outputValueLeft < 1) outputValueLeft = 0;

  //analogWrite(A0, outputValue2);
  //analogWrite(A1, outputValue);
  
  //scaleChangeLeft += abs(outputValueLeft - lastReadingLeft);
  //lastReadingLeft = outputValueLeft;
  
  //control pixel color based on load cells 
  //pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  //pixels.show();

  lick = digitalRead(LICKOMETER1) == HIGH;
  Tare();
  check_buttons();
}

///////////////////////////
////////Sense Right////////
//////////////////////////

void Force::SenseRight() {
  gramsRight = (scaleRight.get_units());
  if (gramsRight < 0) gramsRight = 0;

  if (gramsRight < reqRight){
    pressTimeRight = millis();
    pressLengthRight = 0;
  }
  
  if (gramsRight > reqRight) {
    pressLengthRight = (millis() - pressTimeRight);
  }
    
  //outputValueRight = map(grams, 0, 200, 0, 4095);
 
  //if (outputValueRight > 4000) outputValueRight = 4000;
  //if (outputValueRight < 1) outputValueRight = 0;

  //analogWrite(A0, outputValue2);
  //analogWrite(A1, outputValue);
  
  //scaleChangeRight += abs(outputValueRight - lastReadingRight);
  //lastReadingRight = outputValueRight;

  
  //control pixel color based on load cells 
  //pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  //pixels.show();

  lick = digitalRead(LICKOMETER2) == HIGH;
  Tare();
  check_buttons();
}


