/*
  Melody

  Plays a familiar melody

  circuit:
  Uses a passive speaker! (Is open on bottom)
  - 8 ohm speaker on specified pin

*/

#include "pitches.h"
#define PIN_BUZZER 32

#define TEMPO 120

const float MILLI_PER_4B = 1100;

// notes in the melody:
int melody[] = {
  0, NOTE_CS4, NOTE_CS4, NOTE_CS4, NOTE_CS4, 0, NOTE_CS4, NOTE_B3, NOTE_A3, NOTE_E3, 0, NOTE_CS4, NOTE_B3, NOTE_E3, NOTE_CS4, NOTE_CS4, NOTE_B3, NOTE_A3, NOTE_B3, 0,
  0, NOTE_E4, NOTE_B3, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_A3, 0, NOTE_A3, NOTE_CS4, NOTE_CS4, NOTE_B3, NOTE_A3, NOTE_CS4, 0,
  //Pokemon!
  0, NOTE_CS4, NOTE_E4, NOTE_FS4, 0, NOTE_CS4, NOTE_CS4, NOTE_E4, NOTE_FS4, NOTE_FS4, NOTE_E4, 0,
  0, NOTE_FS4, NOTE_FS4, NOTE_GS4, NOTE_A4, NOTE_GS4, NOTE_FS4, NOTE_FS4,
  NOTE_E4, NOTE_CS4, NOTE_E4, NOTE_FS4, 0, NOTE_FS4, 0, NOTE_FS4, NOTE_E4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_A3,
  0, NOTE_A3, NOTE_B3, NOTE_CS4, NOTE_CS4, NOTE_B3, NOTE_A3, NOTE_CS4, 0

};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  8, 8, 8, 8, 4, 8, 8, 4, 8, 8, 8, 8, 8, 8, 4, 4, 8, 8, 2, 8,
  8, 4, 4, 4, 8, 4, 3, 8, 4, 8, 8, 4, 8, 8, 4, 8, 4, 8, 2, 4,
  //Pokemon!
  8, 8, 8, 4, 8, 8, 8, 8, 4, 4, 2, 2,
  8, 8, 8, 8, 8, 4, 4, 8,
  8, 8, 8, 8, 4, 2, 8, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 4, 8, 4, 8, 2, 2
      
};

void setup() {
  
}

void loop() {
  // iterate over the notes of the melody:
  int melodyLength = sizeof(melody) / sizeof(melody[0]);
  for (int thisNote = 0; thisNote < melodyLength; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = MILLI_PER_4B / noteDurations[thisNote];
    tone(PIN_BUZZER, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    //int pauseBetweenNotes = noteDuration * 1.30;
    int pauseBetweenNotes = noteDuration * 1.3;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(PIN_BUZZER);
  }
  delay(3000);
}