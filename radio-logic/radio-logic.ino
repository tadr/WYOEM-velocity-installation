//
// Logic for high hats firmware for Radio Music
//

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <String>
#include <math.h>

#include <Bounce2.h>


#define SHORT_PRESS_DURATION 10
#define LONG_PRESS_DURATION 600
// after LONG_PRESS_DURATION every LONG_PRESS_PULSE_DELAY milliseconds the update
// function will set BUTTON_PULSE
#define LONG_PRESS_PULSE_DELAY 600
 

#define LED0 6
#define LED1 5
#define LED2 4
#define LED3 3
#define LOGIC_POT_PIN 9 // pin for Logic pot
#define CHAN_A_PIN 6 // pin for Channel A gate 
#define OPEN_HAT_POT_PIN 7 // pin for open hat period pot
#define CHAN_B_PIN 8 // pin for Channel B gate
#define RESET_BUTTON 8 // Reset button 
#define RESET_LED 11 // Reset LED indicator 
#define RESET_CV 9 // Reset pulse input 

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dc1;            //xy=185,198
AudioOutputAnalog        dac1;           //xy=344,197
AudioConnection          patchCord1(dc1, dac1);
// GUItool: end automatically generated code

void setup() {
  Serial.begin(9600);

  AudioMemory(4);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, OUTPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  
  Serial.print("setup done.");
}

elapsedMillis update = 0;



// the loop routine runs over and over again forever:
int n = 0;

int trig_mod = 2;
int trig_cnt = 0;
int trig_state = LOW;
unsigned long trig_high_time = 0;
int TRIG_HIGH_DURATION = 450;
int AMP_HIGH_DURATION = 20;
float LOGIC_POT_SWEEP_SIZE = 1024 / 5.95;
float OPEN_HAT_POT_SWEEP_SIZE = 1022 / 13.95;

int CV_GATE_THRESHOLD = 100;

enum LOGIC_TYPE { LOGIC_OR=0,
                  LOGIC_AND,
                  LOGIC_XOR,
                  LOGIC_NOR,
                  LOGIC_NAND,
                  LOGIC_XNOR };

float amp_last = 0.;
unsigned long amp_high_time = 0.;


void loop() {

 int pot1 = analogRead(LOGIC_POT_PIN);
 int pot2 =  analogRead(OPEN_HAT_POT_PIN);  
 int cv1 = analogRead(CHAN_A_PIN); 
 int cv2 = analogRead(CHAN_B_PIN);

  // Translate pot 2 to OP
  // or, and, xor, nor, nand, xnor

  boolean op;
  float amp = float(cv1 - 1) / 511. - 1.;
  boolean cv1_ = cv1 > CV_GATE_THRESHOLD;
  boolean cv2_ =  cv2 > CV_GATE_THRESHOLD;

  int logic = floor(pot1 / LOGIC_POT_SWEEP_SIZE);
  String name;
  switch(logic) {

  case LOGIC_OR:
    op = cv1_ || cv2_;                // or
    name = "or";
    break;
  case LOGIC_AND:
    op = cv1_ && cv2_;                // and  
    name = "and";
    break;
  case LOGIC_XOR:
    op = cv1_ ^ cv2_;                 // xor
    name = "xor";
    break;
  case LOGIC_NOR:
    op = !cv1_ || !cv2_;              // nor
    name = "nor";
    break;
  case LOGIC_NAND:
    op = !cv1_ && !cv2_;              // nand
    name = "nand";
    break;
  case LOGIC_XNOR:
    op = !(cv1_ ^ cv2_);              // xnor
    name = "xnor";
    break;
  default:
    op = false;
    name = "none";
  }
  
  amp = 1. ? op : 0.;
  bool retrigged = (amp != 0.) && (amp_last == 0.);
  amp_last = amp;

  // Trig
  int trig = LOW;
  if (trig_high_time > 0) {
    if (millis() > TRIG_HIGH_DURATION + trig_high_time) {
        trig_high_time = 0;
        trig = LOW;
    } else {
      trig = HIGH;
    }
  }

  // Amp
  if (amp_high_time > 0) {
    if (millis() > AMP_HIGH_DURATION + amp_high_time) {
        amp_high_time = 0;
        amp = 0.;
    } else {
      amp = 1.;
    }
  }

  trig_mod = floor(pot2 / OPEN_HAT_POT_SWEEP_SIZE);
  
  if (retrigged) {
    amp_high_time = millis();
    trig_cnt = (trig_cnt + 1) % trig_mod;
    if (trig_cnt == 0) {
        trig = HIGH;
        trig_high_time = millis();
    }
  }

  // Write
  dc1.amplitude(amp);
  digitalWrite(RESET_CV, trig);
  delay(20);

  // Debugging messages
  if (n == 100) {

    Serial.print("name = ");
    Serial.print(name);
    Serial.print("\tlogic = ");
    Serial.print(logic);
    Serial.print("\tcv1 = ");
    Serial.print(cv1);
    Serial.print("\tcv2 = ");
    Serial.println(cv2);
    //Serial.print("\ttrig = ");
    //Serial.print(trig);
    Serial.print("op = ");
    Serial.print(op);
    Serial.print("\tamp = ");
    Serial.print(amp);
    Serial.print("\tpot1/mod/tr = ");
    Serial.print(pot1);
    Serial.print("/");
    Serial.print(trig_mod);
    Serial.print("/");
    Serial.print(trig_cnt);
    Serial.print(" -> ");
    Serial.print(trig);
    
    Serial.print("\tretr = ");
    Serial.println(retrigged);
    n = 0;
  } else { n++; }
}
