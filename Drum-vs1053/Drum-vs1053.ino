/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SoftwareSerial.h>
#include"pitches.h"


// define the pins used
#define VS1053_RX  2 // This is the pin that connects to the RX pin on VS1053
#define VS1053_RESET 9 // This is the pin that connects to the RESET pin on VS1053
// If you have the Music Maker shield, you don't need to connect the RESET pin!

// If you're using the VS1053 breakout:
// Don't forget to connect the GPIO #0 to GROUND and GPIO #1 pin to 3.3V
// If you're using the Music Maker shield:
// Don't forget to connect the GPIO #1 pin to 3.3V and the RX pin to digital #2

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31

#define VS1053_BANK_DEFAULT 0x00  //디폴트 소리값
#define VS1053_BANK_DRUMS1 0x78  //드럼 소리
#define VS1053_BANK_DRUMS2 0x7F  //드럼 소리
#define VS1053_BANK_MELODY 0x79  //멜로디를 낼 수 있는 악기 사운드
//미디 설정을 어떻게 하느냐에 따라서 다양한 악기의 소리를 낼 수 있도록 한다


// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!

#define VS1053_GM1_OCARINA 80 //80번 오카리나 소리로 설정
#define MIDI_NOTE_ON 0x90   //스위치 ON. channel은 0x90이다.
#define MIDI_NOTE_OFF 0x80  //스위치 OFF. channel은 0x80이다.
#define MIDI_CHAN_MSG 0xB0   //parameter(한도)
#define MIDI_CHAN_BANK 0x00    //bank select(default 값)
#define MIDI_CHAN_VOLUME 0x07    //볼륨 설정
#define MIDI_CHAN_PROGRAM 0xC0   //프로그래밍 채널



SoftwareSerial VS1053_MIDI(0, 2); // TX only, do not use the 'rx' side
// on a Mega/Leonardo you may have to change the pin to one that 
// software serial support uses OR use a hardware serial port!

void setup() {
  Serial.begin(9600);
  Serial.println("VS1053 MIDI test");
  
  VS1053_MIDI.begin(19200); // MIDI uses a 'strange baud rate', 기본 미디 프로토콜 통신 속도
  //reset VS1053
  pinMode(VS1053_RESET, OUTPUT);
  
digitalWrite(VS1053_RESET, LOW);//미디에다가 LOW(0V)를 준다 내부 20k의 풀업 저항이 설정되므로 외부회로에 의해서만 LOW, HIGH가 변경됨
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);
  
  
  midiSetChannelBank(0, VS1053_BANK_MELODY);  //디폴트 값일 때(아무 입력 없을 때)의 소리를 정한다.
  midiSetInstrument(0, VS1053_GM1_OCARINA);
  midiSetChannelVolume(0, 127);

   backGround();
}

void loop() {  
  for (uint8_t i=60; i<69; i++) {
    midiNoteOn(0, i, 127);
    delay(1000);
    midiNoteOff(0, i, 127);
  }
  
  delay(1000);
}

void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);  
  VS1053_MIDI.write(inst);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  Serial.println("midiNoteON");
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  Serial.println("midiNoteOff");
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void backGround() {
  int melody[] = {
    NOTE_G4, NOTE_C5, NOTE_AS4, NOTE_A4,                   
    NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_FS4, NOTE_DS4, NOTE_D4,
    NOTE_D4, NOTE_G4, NOTE_FS4, NOTE_A4,
    NOTE_C4, NOTE_D4,0,                                 
  
    NOTE_D4, NOTE_G4, NOTE_FS4, NOTE_A4,
    NOTE_G4, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_AS4, NOTE_C5, NOTE_AS4, NOTE_A4,      //29               //8
    NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_FS4, NOTE_DS4, NOTE_D4,
    NOTE_C4, NOTE_D4,0,                                       
  
    NOTE_D4, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_DS5, NOTE_D5,
    NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_C5,
    NOTE_C4, NOTE_D4, NOTE_DS4, NOTE_FS4, NOTE_D5, NOTE_C5,
    NOTE_AS4, NOTE_A4, NOTE_C5, NOTE_AS4,             //58
  
    NOTE_D4, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_DS5, NOTE_D5,
    NOTE_C5, NOTE_D5, NOTE_C5, NOTE_AS4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_C5, NOTE_G4,
    NOTE_A4, 0, NOTE_AS4, NOTE_A4, 0, NOTE_G4,
    NOTE_G4, NOTE_A4, NOTE_G4, NOTE_FS4, 0,
  
    NOTE_C4, NOTE_D4, NOTE_G4, NOTE_FS4, NOTE_DS4,
    NOTE_C4, NOTE_D4, 0,
    NOTE_C4, NOTE_D4, NOTE_G4, NOTE_FS4, NOTE_DS4,
    NOTE_C4, NOTE_D4,0
  
};

// note durations: 8 = quarter note, 4 = 8th note, etc.
  int noteDurations[] = {       //duration of the notes
    8,4,8,4,
    4,4,4,12,
    4,4,4,4,4,4,
    4,16,4,
  
    8,4,8,4,
    4,2,1,1,2,1,1,12,
    4,4,4,4,4,4,
    4,16,4,
  
    4,4,4,4,4,4,
    4,4,4,12,
    4,4,4,4,4,4,
    4,4,4,12,
  
    4,4,4,4,4,4,
    2,1,1,2,1,1,4,8,4,
    2,6,4,2,6,4,
  2,1,1,16,4,
  
  4,8,4,4,4,
  4,16,4,
  4,8,4,4,4,
  4,20,0
};

int speed=90;  //higher value, slower notes

  Serial.begin(9600);
  for (int thisNote = 0; melody[thisNote]!=-1; thisNote++) {

    int noteDuration = speed*noteDurations[thisNote];
    tone(2, melody[thisNote],noteDuration*.95);
    //Serial.println(melody[thisNote]);

    delay(noteDuration);

    noTone(2);
  }
}

