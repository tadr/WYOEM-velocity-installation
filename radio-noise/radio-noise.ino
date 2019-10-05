//
// Simple noise mixer firmware for Radio Music.
//

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>
 

#define LED0 6
#define LED1 5
#define LED2 4
#define LED3 3
#define NOISE_SELECT_POT_PIN 9 // pin for Channel pot
#define CHAN_CV_PIN 6 // pin for Channel CV 
#define MIX2_POT_PIN 7 // pin for Time pot
#define TIME_CV_PIN 8 // pin for Time CV
#define RESET_BUTTON 8 // Reset button 
#define RESET_LED 11 // Reset LED indicator 
#define RESET_CV 9 // Reset pulse input 

// GUItool: begin automatically generated code
AudioSynthNoiseWhite     noise2;         //xy=196,366
AudioSynthNoiseWhite     noise3;         //xy=196,411
AudioSynthNoiseWhite     noise1;         //xy=344,270
AudioSynthNoisePink      pink1;          //xy=344,317
AudioFilterBiquad        biquad1;        //xy=348,367
AudioFilterBiquad        biquad2;        //xy=350,411
AudioSynthWaveformDc     dc1;            //xy=378,509
AudioMixer4              mixer1;         //xy=520,342
AudioMixer4              mixer2;         //xy=688,361
AudioRecordQueue         queue;
AudioOutputAnalog        dac1;           //xy=825,361

AudioConnection          patchCord1(noise2, biquad1);
AudioConnection          patchCord2(noise3, biquad2);
AudioConnection          patchCord3(noise1, 0, mixer1, 0);
AudioConnection          patchCord4(pink1, 0, mixer1, 1);
AudioConnection          patchCord5(biquad1, 0, mixer1, 2);
AudioConnection          patchCord6(biquad2, 0, mixer1, 3);
AudioConnection          patchCord7(mixer1, 0, mixer2, 0);
AudioConnection          patchCord8(mixer2, dac1);
AudioConnection          patchCord9(dc1, 0, mixer2, 1);
AudioConnection          patchCord10(mixer1, queue);



// GUItool: end automatically generated code

int LED_ON = 255;
int LED_OFF = 0;

int LEDS[4] = { LED3, LED2, LED1, LED0 };

float POT_SWEEP_SIZE = 1023.0 / 3.9;

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void setup() {

  Serial.begin(9600);

  AudioMemory(5);
  dac1.analogReference(EXTERNAL); //3.3V p-p vs default 1.2
  
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, INPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  mixer1.gain(0, 0.);
  mixer1.gain(1, 0.);
  mixer1.gain(2, 0.);
  mixer1.gain(3, 0.);
  mixer2.gain(0, .9); // enable noise initially
  mixer2.gain(1, 0.);
  mixer2.gain(2, 0.);
  mixer2.gain(3, 0.);

  noise1.amplitude(1.);
  noise2.amplitude(1.);
  noise3.amplitude(1.);
  pink1.amplitude(1.);
  // 12dB/oct lowpass starting from 100Hz
  biquad1.setLowpass(0, 100, .707);
  // 12dB/oct highpass from 10kHz
  biquad2.setHighpass(0, 10000, .707);
  
  queue.begin();
  
  Serial.print("setup done.");
}


enum NoiseType { White=0, Pink, Brown, Violet };

int button_state = LOW;
int button_read_last = LOW;
int n;

void loop() {
  // put your main code here, to run repeatedly:

  enum NoiseType { White=0, Pink, Brown, Violet };
  int noise_pot = analogRead(NOISE_SELECT_POT_PIN);
  NoiseType noise = (NoiseType)floor(noise_pot / POT_SWEEP_SIZE);

  // For the selected noise type...
  for (int i = 0; i < 4; i++) {
     // Turn up the mixer channel.
     mixer1.gain(i, .9 ? i == (int)noise : 0.);
     // Turn on the LED.
     digitalWrite(LEDS[i], HIGH ? i == (int)noise : LOW);
  }

  if (queue.available()) {
  int button = digitalRead(RESET_BUTTON);

  if (button != button_read_last && button) {
    button_state = ~button_state;    
    
    if (button_state) {
      
      Serial.print("button pressed...");
      analogWrite(RESET_LED, LED_ON);
      
      byte buffer[256];
      memcpy(buffer, queue.readBuffer(), 256);
      float amp = mapfloat(buffer[0] * 256 + buffer[1], 0., 65536., -1., 1.);
      dc1.amplitude(amp);
      mixer2.gain(0, 0.);
      mixer2.gain(1, .9);
    } else {
      analogWrite(RESET_LED, LED_OFF);
      mixer2.gain(0, .9);
      mixer2.gain(1, 0.);
    }
  }
  button_read_last = button;
  }
  queue.clear();

  if (n == 10000) {

    Serial.print("noise = ");
    Serial.println(noise);

    n = 0;
  } else { n++; }
}
