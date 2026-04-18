/*
  Melody

  Hey StinkoMan! Everyone says you're the guy.
  I wanna be the guy, too!

  No way, you're just a kid!

  circuit:
  Uses a passive speaker! (Is open on bottom)
  - 8 ohm speaker on specified pin

  This sketch uses a single passive buzzer and is organized
  accordingly. In order to use more, how setup is handled would
  need to be changed.

*/

#include "pitches.h"
#define BUZZER1_PIN 32
#define BUZZER2_PIN 14
#define BUZZER3_PIN 21
#define BUZZER4_PIN 19

const int8_t CH1 = 0;
const int8_t CH2 = 2;
const int8_t CH3 = 4;
const int8_t CH4 = 6;

const uint8_t   V1_SEED_RES  = 12;
const uint32_t  V1_SEED_FREQ = 19531;

const uint8_t   V2_SEED_RES  = 13;  // 12-bit PWM resolution (0..4095)
const uint32_t  V2_SEED_FREQ = 9765; // any placeholder; we change per note with ledcWriteTone()

const uint8_t   V3_SEED_RES  = 14;
const uint32_t  V3_SEED_FREQ = 15000;

const uint8_t   V4_SEED_RES  = 15;
const uint32_t  V4_SEED_FREQ = 5000;

#define TEMPO 120

const float MILLI_PER_4B = 1550;

// notes in the melody:
int melody[] = {

  NOTE_E3, NOTE_E4, 0, NOTE_B3, NOTE_D4, NOTE_E4, NOTE_E3, NOTE_E4, NOTE_B3, NOTE_D4, NOTE_E4, NOTE_FS4,
  NOTE_E4, NOTE_G4, NOTE_FS4, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_E4, NOTE_FS4, NOTE_G4, NOTE_B4, NOTE_A4,
  NOTE_G4, NOTE_FS4, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_A4, NOTE_A4, NOTE_G4, NOTE_A4, 
  NOTE_C5, 0, NOTE_B4, 0, NOTE_A4, NOTE_G4, 0, NOTE_FS4, 0, NOTE_E4, NOTE_G4, NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_FS4, NOTE_E4,
  NOTE_C4, 0, NOTE_E4, 0, NOTE_C4, NOTE_D4, 0, NOTE_G4, 0, NOTE_FS4, NOTE_E4, 0, NOTE_D4, 0, NOTE_B3, NOTE_C4, 0, NOTE_E4, NOTE_D4,
  NOTE_E4, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_D4, NOTE_B3, NOTE_E4, NOTE_D4, NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_FS4, NOTE_E4,
  NOTE_G4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4

};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {

  8, 8, 16, 16, 16, 16, 8, 8, 16, 16, 16, 16,
  4, 16, 16, 16, 16, 4, 16, 16, 16, 16, 4,
  16, 16, 16, 16, 4, 16, 16, 16, 16,
  8, 16, 8, 16, 8, 8, 16, 8, 16, 8, 16, 8, 8, 8, 16, 2,
  8, 16, 8, 16, 8, 8, 16, 8, 16, 8, 8, 16, 8, 16, 8, 4, 8, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 8, 16, 16, 16, 8, 2
  
};

void setup() {
  //ledcSetClockSource((ledc_clk_cfg_t)LEDC_APB_CLK);

  //ledcAttachChannel(BUZZER1_PIN, V1_SEED_FREQ, V1_SEED_RES, CH1);
  //ledcAttachChannel(BUZZER2_PIN, V2_SEED_FREQ, V2_SEED_RES, CH2);
}

void loop() {
  // iterate over the notes of the melody:
  int melodyLength = sizeof(melody) / sizeof(melody[0]);
  for (int thisNote = 0; thisNote < melodyLength; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = MILLI_PER_4B / noteDurations[thisNote];
    tone(BUZZER1_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    //int pauseBetweenNotes = noteDuration * 1.30;
    int pauseBetweenNotes = noteDuration * 1.3;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER1_PIN);
  }
  delay(3000);
}