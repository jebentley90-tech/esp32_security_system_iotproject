/*
  Melody

  Plays a Strongbadian melody!
  Crazy Go Nuts University!
  Go Dumples!

  circuit:
  Uses a passive speaker! (Is open on bottom)
  - 8 ohm speaker on specified pin

*/

#include "pitches.h"
#define PIN_BUZZER 32
//#define TEMPO 120 Haven't figured out tempo quite yet

const float MILLI_PER_4B = 1100;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_A4,     0, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_AS4, 0,
  NOTE_G4, NOTE_G4,     0, NOTE_G4, NOTE_F4,       0, NOTE_F4, NOTE_E4, NOTE_F4, NOTE_G4, 0,
  NOTE_C4, NOTE_A4,     0, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_AS4, 0,
  NOTE_G4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_F4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 4, 8, 8,  8, 8, 4, 4, 2,
  8, 8, 8, 8, 8, 16, 16, 8, 8, 2, 2,
  4, 4, 8, 8, 8, 8, 4, 3, 3,
  8, 4, 8, 4, 4, 1
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
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(PIN_BUZZER);
  }
  delay(3000);
}
