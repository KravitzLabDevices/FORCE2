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
      reqLeft = 2;
      reqRight = 2;
      dispense_amount = 2000;
      dispense_delay = 4;
      timeout_length = 10;
      ratioLeft = 1;
      ratioRight = 1;
      hold_timeLeft = 250;
      hold_timeRight = 250;
      calibration_factor_Left = -3300;
      calibration_factor_Right = -3300;
      PR = false;
      trials_per_block = 10;
      max_force = 20;
    }
  }
}


/*void Force::load_settings() {
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
}*/

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
// Logging Functions 
/////////////////////////////////////////////////////////////////////////
void Force::CreateDataFile() {
  //put this next line *Right Before* any file open line:
  SdFile::dateTimeCallback(dateTime);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect, SD_SCK_MHZ(4))) {
     error(1);
  }

  // Name filename in format F###_MMDDYYNN, where MM is month, DD is day, YY is year, and NN is an incrementing number for the number of files initialized each day
  strcpy(filename, "FRC_____________.CSV");  // placeholder filename
  getFilename(filename);

  logfile = SD.open(filename, FILE_WRITE);
  if ( ! logfile ) {
    Serial.println("SD Create error");
    error(2);
  }
}

// Write data header to file of uSD.
void Force::writeHeader() {
  logfile.println("MM:DD:YYYY hh:mm:ss, Seconds, Library_Version, Program, Device_Number, ProgressiveRatio, Grams_req, Hold_time, Ratio, Dispense_amount, Dispense_delay, Timeout, Trials_per_block, Max_force, Trial, Press, Lever1_Grams, Lever2_Grams, Licks, Dispense, Random_Num, Shock_trial");
}

// Print data and time followed by pellet count and motorturns to SD card
void Force::WriteToSD() {
  DateTime now = rtc.now();
  logfile.print(now.month());
  logfile.print("/");
  logfile.print(now.day());
  logfile.print("/");
  logfile.print(now.year());
  logfile.print(" ");
  logfile.print(now.hour());
  logfile.print(":");
  if (now.minute() < 10)
    logfile.print('0');      // Trick to add leading zero for formatting
  logfile.print(now.minute());
  logfile.print(":");
  if (now.second() < 10)
    logfile.print('0');      // Trick to add leading zero for formatting
  logfile.print(now.second());
  logfile.print(",");
  
  logfile.print((millis()-start_time)/1000.0000); //print seconds since start
  logfile.print(",");

  logfile.print(ver); // Print library version
  logfile.print(",");
  
  logfile.print(library_version); // Print code or program version
  logfile.print(",");

  logfile.print(FRC); // Print device name
  logfile.print(",");
  
  if (PR==1) logfile.print("true"); // Print 
  if (PR==0) logfile.print("false"); // Print 
  logfile.print(",");
  
  logfile.print(reqLeft); // Print for requirement
  logfile.print(",");
  
  logfile.print(hold_timeLeft); 
  logfile.print(",");
  
  logfile.print(ratioLeft);
  logfile.print(",");
  
  logfile.print(dispense_amount);
  logfile.print(",");
  
  logfile.print(dispense_delay);
  logfile.print(",");
  
  logfile.print(timeout_length);
  logfile.print(",");
  
  logfile.print(trials_per_block);
  logfile.print(",");
  
  logfile.print(max_force);
  logfile.print(",");
 
  logfile.print(trial);
  logfile.print(",");
  
  logfile.print(pressesLeft);
  logfile.print(",");
  
  logfile.print(gramsLeft);
  logfile.print(",");
  
  logfile.print(gramsRight);
  logfile.print(",");
  
  logfile.print(lick);
  logfile.print(",");

  logfile.print(dispensing);
  logfile.print(",");

  logfile.print(random_number);
  logfile.print(",");
  
  logfile.println(shock);

  logfile.flush();

  if ( ! logfile ) {
    error(2);
  }
}

/********************************************************
  If any errors are detected with the SD card print on the screen
********************************************************/
void Force::error(uint8_t errno) {
  tft.setCursor(5, 48);
  tft.print("Check SD card");
}

/********************************************************
  This function creates a unique filename for each file that
  starts with "FRC", then the date in MMDDYY,
  then an incrementing number for each new file created on the same date
********************************************************/
void Force::getFilename(char *filename) {
  DateTime now = rtc.now();

  filename[3] = FRC / 100 + '0';
  filename[4] = FRC / 10 + '0';
  filename[5] = FRC % 10 + '0';
  filename[7] = now.month() / 10 + '0';
  filename[8] = now.month() % 10 + '0';
  filename[9] = now.day() / 10 + '0';
  filename[10] = now.day() % 10 + '0';
  filename[11] = (now.year() - 2000) / 10 + '0';
  filename[12] = (now.year() - 2000) % 10 + '0';
  filename[16] = '.';
  filename[17] = 'C';
  filename[18] = 'S';
  filename[19] = 'V';
  for (uint8_t i = 0; i < 100; i++) {
    filename[14] = '0' + i / 10;
    filename[15] = '0' + i % 10;

    if (! SD.exists(filename)) {
      break;
    }
  }
  return;
}

void Force::logdata() {
  WriteToSD();
}

/////////////////////////////////////////////////////////////////////////
// Display Functions 
/////////////////////////////////////////////////////////////////////////
void Force::UpdateDisplay(){
  graphLegend();
  graphData();
  graphDateTime();
}

void Force::graphData() {
  //Calculate datapoints to graph
  lasty = y;
  lasty2 = y2;
  y = map(outputValueLeft, 0, 750, 0, divideLine);  //scale to the screen
  if (y > divideLine) y = divideLine;
  y2 = map(outputValueRight, 0, 750, 0, divideLine);  //scale to the screen
  if (y2 > divideLine) y2 = divideLine;

  // Clear display in front of graph
  if (x == 0) {
    tft.fillRect(x, 81 - divideLine, 6, divideLine, ST7735_BLACK); //To remove the first bar
    tft.fillRect(0, 0, 160, divideLine, ST7735_BLACK); //To remove the first bar
  }
  tft.drawLine(x + 7, 78, x + 7, 25, ST7735_RED);
  tft.fillRect(x + 1, 81 - divideLine, 6, divideLine, ST7735_BLACK);

  //Graph data load cell 1:
  tft.drawPixel(x + 1, 79 - y, ST7735_YELLOW);
  tft.drawLine(x, 80 - lasty , x + 1, 80 - y, ST7735_MAGENTA);

  //Graph data load cell 2:
  tft.drawPixel(x + 1, 79 - y2, ST7735_WHITE);
  tft.drawLine(x, 80 - lasty2 , x + 1, 80 - y2, ST7735_CYAN);

  //reset graphing position to left side of screen
  x++;
  if (x == 160) x = 0;
}

void Force::graphDateTime() {
  DateTime now = rtc.now();
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(85, 68);
  if (now.month() < 10)
    tft.print('0');      // Trick to add leading zero for formatting
  tft.print(now.month(), DEC);
  tft.print('/');
  if (now.day() < 10)
    tft.print('0');      // Trick to add leading zero for formatting
  tft.print(now.day(), DEC);
  tft.print(' ');
  tft.print(now.hour(), DEC);
  tft.print(':');
  if (now.minute() < 10)
    tft.print('0');      // Trick to add leading zero for formatting
  tft.print(now.minute(), DEC);
}

void Force::graphLegend() {
  // Print force output on F1 and F2
  if (gramsLeft > 1 or gramsRight >1){ //only clear F1 ans F2 values if levers are being pushed
    tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after label
  }
  tft.setCursor(0, 5);  
  tft.setTextColor(ST7735_MAGENTA);
  tft.print("F1: ");   
  tft.println(gramsLeft,0);
  tft.setCursor(0, 17); 
  tft.setTextColor(ST7735_CYAN);
  tft.print("F2: ");
  tft.print(gramsRight,0);
 
  // Print force requirement
  tft.setCursor(45, 5);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Req: ");
  tft.print(reqLeft);
  tft.print("g");

  // Print trial 
  tft.setCursor(45, 17);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Trial:");
  if (gramsLeft > 1 or gramsRight >1){
    tft.fillRect(80, 17, 24, 12, ST7735_BLACK); // clear task data on each trial
  }
  tft.print(trial);

  // Print FR ratio
  tft.setCursor(110, 5);
  tft.setTextColor(ST7735_YELLOW);
  if (PR ==0) tft.print("FR:");
  if (PR ==1) tft.print("PR:");
  tft.print(ratioLeft);

  // Print current press
  tft.setCursor(110, 17);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Press:");
  if (gramsLeft > 1 or gramsRight >1){
    tft.fillRect(143, 17, 28, 12, ST7735_BLACK); // clear press data on each trial
  }
  tft.print(pressesLeft);

  //Indicate licks
  tft.fillRect(0, 27, 40, 12, ST7735_BLACK); // clear the text after label
  if (lick == true) {
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(0, 28);
    tft.print ("Lick");
    digitalWrite(A3, HIGH);  //CHECK THIS, MIGHT NOT BE THE RIGHT PIN
    DateTime now = rtc.now();
    lickTime = now.unixtime();
  }

  if (lick == false) {

    digitalWrite(A3, LOW); //CHECK THIS, MIGHT NOT BE THE RIGHT PIN

  }
  
  if (calibrated == false){
    tft.setCursor(85, 56);
    tft.print ("Uncalibrated");
  }
}


/////////////////////////////////////////////////////////////////////////
// Calibration function
/////////////////////////////////////////////////////////////////////////
void Force::Tare() {
  if (millis() - start_timer > 5000)  {
    if (scaleChangeLeft < 1000) {  // this sets sensitivity for delaying taring
      pixels.setPixelColor(0, pixels.Color(0, 10, 10));
      pixels.show();
      scaleLeft.tare();
    }
    if (scaleChangeRight < 1000) {
      pixels.setPixelColor(0, pixels.Color(10, 10, 0));
      pixels.show();
      scaleRight.tare();
    }
    start_timer = millis();
    scaleChangeLeft  = 0;
    scaleChangeRight  = 0;
  }
}

void Force::Calibrate(){
  bool lever1 = true;
  tft.fillScreen(ST77XX_BLACK);
  scaleLeft.tare();
  scaleRight.tare();
  while (calibrate_active==true){
    uint32_t buttons = ss.readButtons();
    if (! (buttons & TFTWING_BUTTON_A)) lever1 = true;
    if (! (buttons & TFTWING_BUTTON_B)) lever1 = false;
    
    float calibrate_timer = millis();
    tft.setCursor(40, 15);  
    tft.setTextColor(ST7735_WHITE);
    tft.println("Calibrate levers:");   
    gramsLeft = (scaleLeft.get_units());
    gramsRight = (scaleRight.get_units());
    tft.setCursor(40, 30);  
    tft.print("Lever Left:");   
    tft.println(gramsLeft,1);   
    tft.setCursor(40, 45);  
    tft.print("Lever Right:");   
    tft.println(gramsRight,1);   
    delay (100);
    if (lever1 == true){
        tft.fillRect(0, 30, 160, 12, ST7735_BLUE); // highlight active bar
        tft.fillRect(0, 43, 160, 12, ST7735_BLACK); // highlight active bar

        if (! (buttons & TFTWING_BUTTON_UP)) { 
          calibration_factor_Left += 100;
          scaleLeft.set_scale(calibration_factor_Left);
        }
        
        if (! (buttons & TFTWING_BUTTON_DOWN)) { 
          calibration_factor_Left -= 100;
          scaleLeft.set_scale(calibration_factor_Left);
        }
    }
    
    if (lever1 == false){
        tft.fillRect(0, 30, 160, 12, ST7735_BLACK); // highlight active bar
        tft.fillRect(0, 43, 160, 12, ST7735_BLUE); // highlight active bar
        if (! (buttons & TFTWING_BUTTON_UP)) { 
          calibration_factor_Right += 100;
          scaleRight.set_scale(calibration_factor_Right);
        }
        
        if (! (buttons & TFTWING_BUTTON_DOWN)) { 
          calibration_factor_Right -= 100;
          scaleRight.set_scale(calibration_factor_Right);
        }
    }

    if (! (buttons & TFTWING_BUTTON_SELECT)) {
      Click();
      start_timer = millis();
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 15);  
      tft.println("Remove weights");
      calibrated = true;
      delay (2000);
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 15);  
      tft.println("Calibrated!");
      delay (1000);
      start_up_menu();
    }
  }
}


/////////////////////////////////////////////////////////////////////////
// FORCE menu
/////////////////////////////////////////////////////////////////////////
void Force::start_up_menu() {
  calibrate_active = false;
  print_settings();
  float start_timer = millis();
  int option = 0;
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
  while (start_up == true) {
    int page = 1;
    //////////////////////////////////////////////
    ////////////////////// PAGE 1 ////////////////
    //////////////////////////////////////////////
    while (page == 1) {
      if ((millis() - start_timer) > 10000) start_up = false; //after 10 seconds of start up menu, start session
      uint32_t buttons = ss.readButtons();
      tft.setCursor(40, 5);
      tft.setTextColor(ST7735_MAGENTA);
      tft.println("FR Menu");

      tft.setCursor(0, 20);
      tft.setTextColor(ST7735_CYAN);

      //option 0
      tft.print("device #:       ");
      tft.println(FRC);
      if (option == 0) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          FRC ++;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          FRC --;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 1
      tft.print("ratio Left:       ");
      tft.println(ratioLeft);
      if (option == 1) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          ratioLeft ++;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          ratioLeft --;
          if (ratioLeft < 0) ratioLeft = 0;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 2
      tft.print("force_req_Left:   ");
      tft.print(reqLeft);
      tft.println(" g");
      if (option == 2) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          reqLeft ++;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          reqLeft --;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 3
      tft.print("hold_time:      ");
      tft.print(hold_timeLeft);
      tft.println(" ms");
      if (option == 3) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          Click();
          hold_timeLeft += 10;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          Click();
          hold_timeLeft -= 10;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
      
      //option 4
      tft.print("dispense_delay: ");
      tft.print(dispense_delay);
      tft.println(" s");
      if (option == 4) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          dispense_delay += 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          dispense_delay -= 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
            
      //option 5
      tft.print("timeout:        ");
      tft.print(timeout_length);
      tft.println(" s");
      if (option == 5) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          Click();
          timeout_length += 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          Click();
          timeout_length -= 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }


      //button up
      if (! (buttons & TFTWING_BUTTON_UP)) {
        option --;
        start_timer = millis();
        Click();
        if (option < 0) {
          tft.fillScreen(ST77XX_BLACK);
          option = 11;
          page = 2;
        }
        tft.fillRect(0, ((option+1) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
        tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        delay (150);
        save_settings();
        print_settings();
      }

      //button down
      if (! (buttons & TFTWING_BUTTON_DOWN)) {
        option ++;
        start_timer = millis();
        Click();
        tft.fillRect(0, ((option-1) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
        tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        delay (150);
        if (option > 5) {
          tft.fillScreen(ST77XX_BLACK);
          page = 2;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        save_settings();
        print_settings();
      }

      //button select
      if (! (buttons & TFTWING_BUTTON_SELECT)) {
        delay (500);
        uint32_t buttons = ss.readButtons();
        if (! (buttons & TFTWING_BUTTON_SELECT)) {
            Click();
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(40, 35);
            tft.setTextColor(ST7735_WHITE);
            save_settings();
            print_settings();
            tft.println("Starting FORCE!");
            delay (250);
            start_time = millis();
            start_up = false;
            page = 0;
         }
      }
    }
    
    //////////////////////////////////////////////
    ////////////////////// PAGE 2 ////////////////
    //////////////////////////////////////////////
    tft.fillScreen(ST77XX_BLACK);
    tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
    print_settings();
    while (page == 2) {
      if ((millis() - start_timer) > 10000) start_up = false; //after 10 seconds of start up menu, start session
      uint32_t buttons = ss.readButtons();
      tft.setCursor(40, 5);
      tft.setTextColor(ST7735_MAGENTA);
      tft.println("PR Menu");

      tft.setCursor(0, 20);
      tft.setTextColor(ST7735_CYAN);

      //option 6
      tft.print("Prog ratio: ");
      if (PR == 0) tft.println("off");
      if (PR == 1) tft.println("on");
      if (option == 6) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          PR = 1;
          ratioLeft = 1; //all PR sessions will have the FR ratio of 1
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          
        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          PR = 0;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
      
      //option 7
      tft.print("Trials per block: ");
      tft.println(trials_per_block);
      if (option == 7) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          trials_per_block ++;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          trials_per_block --;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 8
      tft.print("Max force: ");
      tft.print (max_force);
      tft.println(" g");
      if (option == 8) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          max_force ++;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          max_force--;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 9
      tft.println(" ");
      if (option == 9) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          tft.fillRect(0, (((option - 6) - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 10
      tft.setTextColor(ST7735_RED);
      tft.println("Calibrate FORCE");
      if (option == 10) {
        if (! (buttons & TFTWING_BUTTON_RIGHT) or ! (buttons & TFTWING_BUTTON_SELECT)) {
          start_timer = millis();
          Tone();
          delay (250);
          calibrate_active = true;
          Calibrate();
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
      
      //option 11
      tft.setTextColor(ST7735_RED);
      tft.println("Reset settings");
      if (option == 11) {
        if (! (buttons & TFTWING_BUTTON_RIGHT) or ! (buttons & TFTWING_BUTTON_SELECT)) {
          start_timer = millis();
          delay (250);
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          reset_settings();
        }
      }

      //button up
      if (! (buttons & TFTWING_BUTTON_UP)) {
        option --;
        start_timer = millis();
        Click();
        if ((option <= 11) and (option > 5)){
          tft.fillRect(0, ((option - 5) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        delay (150);
        if (option < 6) {
          tft.fillScreen(ST77XX_BLACK);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          page = 1;
        }
        save_settings();
        print_settings();
      }

      //button down
      if (! (buttons & TFTWING_BUTTON_DOWN)) {
        option ++;
        start_timer = millis();
        Click();
        if (option <=11){
          tft.fillRect(0, ((option - 7) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        delay (150);
        if (option > 11) {
          tft.fillScreen(ST77XX_BLACK);
          option = 0;
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          page = 1;
        }
        save_settings();
        print_settings();
      }

      //button select
      if (! (buttons & TFTWING_BUTTON_SELECT)) {
        delay (500);
        uint32_t buttons = ss.readButtons();
        if (! (buttons & TFTWING_BUTTON_SELECT)) {
            Click();
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(40, 35);
            tft.setTextColor(ST7735_WHITE);
            save_settings();
            print_settings();
            tft.println("Starting FORCE!");
            delay (250);
            start_time = millis();
            start_up = false;
            page = 0;
         }
      }
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
  
    
  outputValueLeft = map(gramsLeft, 0, 200, 0, 4095);
 
  if (outputValueLeft > 4000) outputValueLeft = 4000;
  if (outputValueLeft < 1) outputValueLeft = 0;

  //analogWrite(A1, outputValueLeft);
  
  scaleChangeLeft += abs(outputValueLeft - lastReadingLeft);
  lastReadingLeft = outputValueLeft;
  
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
    
  outputValueRight = map(grams, 0, 200, 0, 4095);
 
  if (outputValueRight > 4000) outputValueRight = 4000;
  if (outputValueRight < 1) outputValueRight = 0;

  //analogWrite(A0, outputValueRight);
  
  scaleChangeRight += abs(outputValueRight - lastReadingRight);
  lastReadingRight = outputValueRight;

  
  //control pixel color based on load cells 
  //pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  //pixels.show();

  lick = digitalRead(LICKOMETER2) == HIGH;
  Tare();
  check_buttons();
}


