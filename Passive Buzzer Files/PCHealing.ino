/*
  Melody Code for 1 Buzzer

  Plays a familiar Pokemon melody

  circuit:
  Uses a passive speaker! (Is open on bottom)
  - 8 ohm speaker on specified pin

  by Joseph Bentley

*/

#include "pitches.h"
#define PIN_BUZZER 32
#define TEMPO 120

const float MILLI_PER_4B = 1500;

// notes in the melody:
int melody[] = {
  NOTE_B4, 0, NOTE_B4, 0, NOTE_B4, NOTE_GS4, NOTE_E5

};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  8, 8, 8, 8, 8, 8, 4
  
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