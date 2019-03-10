#include "buzzer.h"


void initBuzzer()
{
  pinMode(MELODY_PIN, OUTPUT);//buzzer
}
void startBuzzer()
{
  //sing the tunes
  sing(MARIO);
  sing(MARIO);
  sing(UNDERWORLD);
}

void sing(int song) {
    switch(song){
        case UNDERWORLD:
            SerialUSB.println(" 'Underworld Theme'");
            int size = sizeof(underworld_melody) / sizeof(int);
            for (int thisNote = 0; thisNote < size; thisNote++) {
                // to calculate the note duration, take one second
                // divided by the note type.
                //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
                int noteDuration = 1000 / underworld_tempo[thisNote];
                buzz(MELODY_PIN, underworld_melody[thisNote], noteDuration);
                // to distinguish the notes, set a minimum time between them.
                // the note's duration + 30% seems to work well:
                int pauseBetweenNotes = noteDuration * 1.30;
                delay(pauseBetweenNotes);
                // stop the tone playing:
                buzz(MELODY_PIN, 0, noteDuration);
            }
            break;
        case MARIO:
            SerialUSB.println(" 'Mario Theme'");
            int size = sizeof(melody) / sizeof(int);
            for (int thisNote = 0; thisNote < size; thisNote++) {
                // to calculate the note duration, take one second
                // divided by the note type.
                //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
                int noteDuration = 1000 / tempo[thisNote];
                buzz(MELODY_PIN, melody[thisNote], noteDuration);
                // to distinguish the notes, set a minimum time between them.
                // the note's duration + 30% seems to work well:
                int pauseBetweenNotes = noteDuration * 1.30;
                delay(pauseBetweenNotes);
                // stop the tone playing:
                buzz(MELODY_PIN, 0, noteDuration);
            }
            break;
    }
}

void buzz(int targetPin, long frequency, long length) {
  long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
}
