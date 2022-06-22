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
  
  pinMode(LICKOMETER, INPUT_PULLDOWN);
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
  scalLefte.set_scale(calibration_factor);
  
  scaleRight.begin(DOUT2, CLK2);
  scaleRight.tare();
  scaleRight.set_scale(calibration_factor);  

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
      tft.println("Solenoid flush");   
      pixels.show();
      digitalWrite(SOLENOID, HIGH);
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
      digitalWrite(SOLENOID, LOW);
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
    if ((grams > 1.5) or (grams2 > 1.5)) { //reset timeout if either lever pushed
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
  tone(A5, frequency, duration);
}

void Force::Click() {
  tone(A5, 800, 8);
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
  pressTime = millis();
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
  pressLengthRight = 0;
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

