/*
 * Required libraries:
 * - HX711 Arduino Library (v0.7.4) by Bogdan Necula, Andreas Motl
 * - SdFat - Adafruit Fork (v1.2.3) by Bill Greiman
 * - RTClib (v1.13.0) by Adafruit
 * - Adafruit ST7735 and ST7789 Library (v1.7.2) by Adafruit
 * - Adafruit seesaw Library (v1.4.2) by Adafruit
 * - Adafruit Neopixel (v1.8.0) by Adafruit
 * - Adafruit GFX Library (v1.10.7) by Adafruit
 * - Adafruit Adafruit_SPIFlash (v3.4.1) by Adafruit
*/

#ifndef FORCE_H
#define FORCE_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_miniTFTWing.h>
#include <Adafruit_ST7735.h>
#include <HX711.h>
#include <SdFat.h>
#include "RTClib.h"
#include "Adafruit_SPIFlash.h"

// Pin definitions
#define TFT_RST      -1     // TFT display shield
#define TFT_CS       5      // TFT display shield
#define TFT_DC       6      // TFT display shield

#define DOUT1        13     // Load cell1 left
#define CLK1         12     // Load cell1 left
#define LICK1        17     // Lick-o-meter Left
#define PUMP1        4      // Pump left
#define BNC_OUT1     14     // BNC output left

#define DOUT2        11     // Load cell2 right
#define CLK2         9      // Load cell2 right 
#define LICK2        16     // Lick-o-meter Right
#define PUMP2        18     // Pump right
#define BNC_OUT2     15     // BNC output right

#define POKE         19     // Nosepoke
#define BEEPER       1     // Beeper

    
void dateTime(uint16_t* date, uint16_t* time);

class Force {
  public:
    Force(String ver);
    String ver = "undef";
    String library_version = "undef";
            
    // --- Basic functions --- //
    void begin(bool log_lite = true);
    void run();
    void check_buttons();
    void readPoke();
    
    // --- TFT display wing --- //
    Adafruit_miniTFTWing ss;
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    int divideLine = 57;
    int x_speed = 1;
    int x = 0;
    int y;
    int lasty;
    int y2;
    int lasty2;
    void graphLegend();
    void graphData();
    void graphDateTime();
    void UpdateDisplay();

    // --- Neopixel --- //
    Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);

    // --- Load Cells --- //
    HX711 scaleLeft;
    int calibration_factor_Left = -3300;    
    float scaleChangeLeft = 0;    // Load cell 1
    float lastReadingLeft = 0;    // Load cell 1        
    float gramsLeft;    
    float outputValueLeft;        // Load cell 1

        
    HX711 scaleRight;
    int calibration_factor_Right = -3300;    
    float scaleChangeRight = 0;   // Load cell 2
    float lastReadingRight = 0;   // Load cell 2
    float outputValueRight;       // Load cell 2
    float gramsRight;

    
    void SenseLeft();
    void SenseRight();
    void Tare();
    void Calibrate();
    bool calibrate_active = false;
    bool calibrated = false;

    // --- SD File --- //
    SdFat SD;
    File logfile;               // Create file object
    char filename[21];          // Array for file name data logged to named in setup
    const int chipSelect = 10;
    void CreateDataFile();
    void writeHeader(bool log_lite = true);
    void error(uint8_t errno);
    void getFilename(char *filename);
    void logdata();
    void logdata_lite();
    void loglite_Left();
    void loglite_Right();
    unsigned long unixtime = 0;
    
    bool log_lite = true;
    bool dispensing = true;


    // --- Pump functions --- //
    float dispense_time = 0;
    int dispense_amount = 2000;
    unsigned long last_dispense = 0;
    void DispenseLeft();
    void DispenseRight();
    void prime_dispense();
    void check_lastDispense();
    void checkLicks();


    // --- Lever functions --- //
    int dispense_delay = 4;
    
    unsigned long pressLengthLeft = 0;
    unsigned long pressTimeLeft = 0;
    unsigned long pressLengthRight = 0;
    unsigned long pressTimeRight = 0;  
      

    // --- Trial functions--- //
    int FRC = 1;          // This is the unique # of the device
    int LeftActive = 1;
    int RightActive = 1;     
    bool poke = false;
    int PR = 0;
    void Tone(int frequency = 500, int duration = 200);
    void Click();
    void Timeout(int timeout_length = 10);
    int timeout_length = 10;
    bool lickLeft = false;
    bool lickRight = false;
    int start_timer = 0;
    int trial_start = 0;
    int trial_window = 10000; // Time window between possible start of the trial and lever press
    bool trial_available = false;
    
    int pressesLeft = 0;
    int reqLeft = 2;    
    int ratioLeft = 1;
    int hold_timeLeft = 350;
    int rewardLeft = 0;
    int trials_left = 0;
            
    int pressesRight = 0;
    int reqRight = 2;
    int ratioRight = 1;
    int hold_timeRight = 350;
    int rewardRight = 0;
    int trials_right = 0;
            
    unsigned long pressStart = 0;    
    int trials_per_block = 10;
    int max_force = 20; 
    unsigned long start_time = 0;
    unsigned long lickTime = 0;
    unsigned long dispenseTime = 0;
    
    String event = "None";
    
    // --- Serial out--- //
    void SerialOutput();

    // --- start up menu--- //
    void start_up_menu();
    bool start_up = true;
    void save_settings();
    
    // --- save and load settings from QSPI flash --- //
    int settings[18];  //array for saving settings on QSPI flash
    int settings_recalled[18]; //array for reading from QSPI flash
    void load_settings();
    void reset_settings();
    void print_settings();
};

#endif  // FORCE_H
