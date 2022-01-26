#include <PsxControllerBitBang.h>
#include <PsxNewLib.h>
#include <NintendoSwitchControlLibrary.h>
#include <Psx.h>
#include <Joystick.h>

/*
 * Using PsxNewLib NewSwitchControlLibrary and Arduino Joystick Library
 * Will act as joycon for nintento switch without any bridge adapter or console tuner
 * 
 * [ 1 2 3 4 5 6 7 8 9 ]  -- refer to google images PS1 controller pin for diagram
 * PIN  DESCRIPTION
 * 1    DATA
 * 2    COMMAND
 * 3    VIBRATION MOTOR POWER (7.2-9v)
 * 4    GROUND
 * 5    VCC POWER 3.3v
 * 6    ATTENTION
 * 7    CLOCK
 * 8    NO CONNECTION
 * 9    ACKNOWLEDGE
 * 
 * references
 * @ceclow @tech-hatao @SukkoPera
 * (c) 2021 2022 @rcvalkyrie
 * 
 */
 
const int DataPin = 8;
const int CmndPin = 9;
const int ClockPin = 10;
const int AttPin = 11;

const int StickXPin = 0;
const int StickYPin = 1;
const int StickSwPin = 20;

#define ultrasoundTriggerPin 2
#define ultrasoundEchoPin 3
#define dataPin 8
#define cmndPin 9
#define attPin 11
#define clockPin 10

#define pedalPin 6

#define ANALOG_MIN_VALUE 0U
#define ANALOG_MAX_VALUE 255U
#define ANALOG_IDLE_VALUE 128U

const unsigned long POLLING_INTERVAL = 1000U / 50U;

Psx Psx;
unsigned int data = 0;  // data stores the controller response

Joystick_ usbStick (
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_GAMEPAD, 
  12,     // buttonCount  // my change
  1,      // hatSwitchCount (0-2) // my change
  true,   // includeXAxis
  true,   // includeYAxis
  false,    // includeZAxis
  true,   // includeRxAxis
  true,   // includeRyAxis
  false,    // includeRzAxis
  false,    // includeRudder
  false,    // includeThrottle
  false,    // includeAccelerator
  false,    // includeBrake
  false   // includeSteering
);


#ifdef ENABLE_SERIAL_DEBUG
  #define dstart(spd) do {Serial.begin (spd); while (!Serial) {digitalWrite (LED_BUILTIN, (millis () / 500) % 2);}} while (0);
  #define debug(...) Serial.print (__VA_ARGS__)
  #define debugln(...) Serial.println (__VA_ARGS__)
#else
  #define dstart(...)
  #define debug(...)
  #define debugln(...)
#endif

boolean haveController = false;


#define toDegrees(rad) (rad * 180.0 / PI)

#define deadify(var, thres) (abs (var) > thres ? (var) : 0)

const byte ANALOG_DEAD_ZONE = 50U;

void setup () {
  // Lit the builtin led whenever buttons are pressed
  pinMode (LED_BUILTIN, OUTPUT);

  // Init Joystick library
  usbStick.begin (false);   // We'll call sendState() manually to minimize lag

  // Init Psx library
  Psx.setupPins(dataPin, cmndPin, attPin, clockPin, 20);  // Defines what each pin is used
                                                          // (Data Pin #, Cmnd Pin #, Att Pin #, Clk Pin #, Delay)
                                                          // Delay measures how long the clock remains at each state,
                                                          // measured in microseconds.
                                                          // too small delay may not work (under 5)
  
  // This way we can output the same range of values we get from the PSX controller
    usbStick.setXAxisRange (ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);
    usbStick.setYAxisRange (ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);
    usbStick.setRxAxisRange (ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);
    usbStick.setRyAxisRange (ANALOG_MIN_VALUE, ANALOG_MAX_VALUE);

  // Init ultrasound
  pinMode(ultrasoundTriggerPin, OUTPUT);
  pinMode(ultrasoundEchoPin, INPUT);
  digitalWrite(ultrasoundTriggerPin, LOW);  

  dstart (115200);

  debugln (F("Ready!"));
}

void loop () {
  static unsigned long last = 0;
        
  data = Psx.read();

//FACE BUTTONS HERE

  static bool btnAPressed = false;
  static bool btnBPressed = false;
  static bool btnXPressed = false;
  static bool btnYPressed = false;
    
  if(data & psxO){
    if(!btnAPressed){
      SwitchControlLibrary().pressButton(Button::A);
      SwitchControlLibrary().sendReport();
      btnAPressed = true;
    }
  }else{
    if(btnAPressed){
      SwitchControlLibrary().releaseButton(Button::A);
      SwitchControlLibrary().sendReport();
      btnAPressed = false;
    }
  }

  if(data & psxX){
    if(!btnBPressed){
      SwitchControlLibrary().pressButton(Button::B);
      SwitchControlLibrary().sendReport();
      btnBPressed = true;
    }
  }else{
    if(btnBPressed){
      SwitchControlLibrary().releaseButton(Button::B);
      SwitchControlLibrary().sendReport();
      btnBPressed = false;
    }
  }
 
  if(data & psxTri){
    if(!btnXPressed){
      SwitchControlLibrary().pressButton(Button::X);
      SwitchControlLibrary().sendReport();
      btnXPressed = true;
    }
  }else{
    if(btnXPressed){
      SwitchControlLibrary().releaseButton(Button::X);
      SwitchControlLibrary().sendReport();
      btnXPressed = false;
    }
  }
 
  if(data & psxSqu){
    if(!btnYPressed){
      SwitchControlLibrary().pressButton(Button::Y);
      SwitchControlLibrary().sendReport();
      btnYPressed = true;
    }
  }else{
    if(btnYPressed){
      SwitchControlLibrary().releaseButton(Button::Y);
      SwitchControlLibrary().sendReport();
      btnYPressed = false;
    }
  }
  
//SELECT START

  static bool btnPlusPressed = false;
  
  if(data & psxStrt){
    if(!btnPlusPressed){
      SwitchControlLibrary().pressButton(Button::PLUS);
      SwitchControlLibrary().sendReport();
      btnPlusPressed = true;
    }
  }else{
    if(btnPlusPressed){
      SwitchControlLibrary().releaseButton(Button::PLUS);
      SwitchControlLibrary().sendReport();
      btnPlusPressed = false;
    }
  }

  static bool btnMinusPressed = false;
  
  if(data & psxSlct){
    if(!btnMinusPressed){
      SwitchControlLibrary().pressButton(Button::MINUS);
      SwitchControlLibrary().sendReport();
      btnMinusPressed = true;
    }
  }else{
    if(btnMinusPressed){
      SwitchControlLibrary().releaseButton(Button::MINUS);
      SwitchControlLibrary().sendReport();
      btnMinusPressed = false;
    }
  }

//SHOULDER BUTTONS HERE MGA BAKLA

  static bool btnRPressed = false;
  static bool btnLPressed = false;
  static bool btnZlPressed = false;  
  static bool btnZrPressed = false;  
  
  if(data & psxR1){
    
    if(!btnRPressed){
        SwitchControlLibrary().pressButton(Button::R); 
        SwitchControlLibrary().sendReport(); 
        btnRPressed = true;
      }
    }else{
      if(btnRPressed){
        SwitchControlLibrary().releaseButton(Button::R);
        SwitchControlLibrary().sendReport(); 
        btnRPressed = false;
      }
    }

  if(data & psxL1){
    
    if(!btnLPressed){
        SwitchControlLibrary().pressButton(Button::L); 
        SwitchControlLibrary().sendReport(); 
        btnLPressed = true;
      }
    }else{
      if(btnLPressed){
        SwitchControlLibrary().releaseButton(Button::L);
        SwitchControlLibrary().sendReport(); 
        btnLPressed = false;
      }
    }

  if(data & psxL2){
    
    if(!btnZlPressed){
        SwitchControlLibrary().pressButton(Button::ZL); 
        SwitchControlLibrary().sendReport(); 
        btnZlPressed = true;
      }
    }else{
      if(btnZlPressed){
        SwitchControlLibrary().releaseButton(Button::ZL);
        SwitchControlLibrary().sendReport(); 
        btnZlPressed = false;
      }
    }

  if(data & psxR2){
    
    if(!btnZrPressed){
        SwitchControlLibrary().pressButton(Button::ZR); 
        SwitchControlLibrary().sendReport(); 
        btnZrPressed = true;
      }
    }else{
      if(btnZrPressed){
        SwitchControlLibrary().releaseButton(Button::ZR);
        SwitchControlLibrary().sendReport(); 
        btnZrPressed = false;
      }
    }

//DIRECTIONAL BUTTONS HERE
  
    int readStickX = analogRead(StickXPin);
    int readStickY = analogRead(StickYPin);  
    static bool btnRightPressed = false;
    static bool btnLeftPressed = false; 
    static bool btnUpPressed = false;
    static bool btnDownPressed = false;

  if(data & psxRight){
    
  //if(readStickX < 200 ){
    if(!btnRightPressed){
        SwitchControlLibrary().pressHatButton(Hat::RIGHT);
        SwitchControlLibrary().sendReport(); 
        btnRightPressed = true;
      }
    }else{
      if(btnRightPressed){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport(); 
        btnRightPressed = false;
      }
    }

  if(data & psxLeft){
    
   //if(readStickX > 200 ){
      if(!btnLeftPressed){
        SwitchControlLibrary().pressHatButton(Hat::LEFT);
        SwitchControlLibrary().sendReport(); 
        btnLeftPressed = true;
      }
    }else{
      if(btnLeftPressed){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport(); 
        btnLeftPressed = false;
      }
    }

  if(data & psxUp){
    
   //if(readStickY < 200 ){
      if(!btnUpPressed){
        SwitchControlLibrary().pressHatButton(Hat::UP);
        SwitchControlLibrary().sendReport(); 
        btnUpPressed = true;
      }
    }else{
      if(btnUpPressed){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport(); 
        btnUpPressed = false;
      }
    }
 
  if(data & psxDown){
    
   //if(readStickY > 824 ){
      if(!btnDownPressed){
        SwitchControlLibrary().pressHatButton(Hat::DOWN);
        SwitchControlLibrary().sendReport(); 
        btnDownPressed = true;
      }
    }else{
      if(btnDownPressed){
        SwitchControlLibrary().releaseHatButton();
        SwitchControlLibrary().sendReport(); 
        btnDownPressed = false;
      }
    }
    int lStickX;
    int lStickY;
    int rStickX;
    int rStickY;

//YABADABADOO!!
  }
