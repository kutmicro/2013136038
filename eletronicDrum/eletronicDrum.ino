///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////마이크로 프로세서 및 실습 ///////////////////////////////
/////////2013136038 박성준//2013136062 신찬욱//////////////////////////////////////////////////////
///////////////////////////////////////////////Electronic Drum/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////j
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <SoftwareSerial.h>
/////////////////MIDI////////////////////////////////
SoftwareSerial mySerial(2, 3); // RX, TX

byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line
byte ledPin = 13; //MIDI traffic inidicator
int  instrument = 0;

//////////////////mpr121///////////////////////////
Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
///////////////////////////////////////////////////


#define CRASH_CYMBAL    (49)
#define RIDE_CYMBAL     (51)
#define HIGH_HAT        (44)
#define LOW_FLOOR_TOM   (41)
#define HIGH_FLOOR_TOM  (43)
#define LOW_MID_TOM     (47)
#define HIGH_MID_TOM    (48)
#define LOW_TOM         (45)
#define HIGH_TOM        (50)
#define ACOUSTIC_SNARE  (38)
#define ELECTRIC_SNARE  (40)
#define BASS_DRUM       (35)

void PercussionMessage(byte type, byte velocity) {
  Serial.write(0x90 | 9);
  //미디프로토콜에서 0x90은 Note On command, channel은 b3~b0 에 있습니다. channel 9는 타악기 입니다.
    switch(type){
      case 0: noteOn(0,CRASH_CYMBAL, velocity);Serial.println("0touched"); break;
      case 1: noteOn(0,RIDE_CYMBAL, velocity);Serial.println("1touched"); break;
      case 2: noteOn(0,HIGH_HAT, velocity);Serial.println("2touched"); break;
      case 3: noteOn(0,LOW_FLOOR_TOM, velocity); Serial.println("3touched"); break;
      case 4: noteOn(0,HIGH_FLOOR_TOM, velocity); Serial.println("4touched"); break;
      case 5: noteOn(0,LOW_MID_TOM, velocity); Serial.println("5touched");break;
      case 6: noteOn(0,HIGH_MID_TOM, velocity);  Serial.println("6touched");break;
      case 7: noteOn(0,LOW_TOM, velocity); Serial.println("7touched");break;
      case 8: noteOn(0,HIGH_TOM, velocity);  Serial.println("8touched");break;
      case 9: noteOn(0,ACOUSTIC_SNARE, velocity); Serial.println("9touched");break;
      case 10: noteOn(0,ELECTRIC_SNARE, velocity);  Serial.println("10touched");break;
      case 11: noteOn(0,BASS_DRUM, velocity); Serial.println("11touched");break;
  }
}
//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}



  /*---------------------------------------------------------
      S  E  T  U  P
  ---------------------------------------------------------*/
void setup() {
  Serial.begin(19200);

  if (!cap.begin(0x5A)) {/*0x5A를 디폴트 주소값으로 I2C통신을 초기화 하고, 활성화 한다. 
  그리고 MPR121센서의 위치별 주소를 할당한다. 완료후 true반환*/
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MRP121 Connected");
  ///////////////////////////////////////////////////////////////////////////
  mySerial.begin(31250);

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0, 0x78);//0xB0 is channel message, set channel volume to near max (127)
}


  /*---------------------------------------------------------
      L  O  O  P
  ---------------------------------------------------------*/
void loop() {
  currtouched = cap.touched();
  talkMIDI(0xC0, instrument, 0); 
  for (uint8_t i=0; i<12; i++) {
    /*현재 감지된 위치의 주소값을 슬레이브모드에서 받아와서 비트연산자를 통해  uint16_t 형식으로 바꾼다, 
    후에 변수 t에 저장 한 후, t & 0x0FFF를 리턴한다. 그러면 0~11 중의 숫자가 리턴된다.*/
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
     PercussionMessage(i, 60);
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
     PercussionMessage(i, 0);
    }d
  }
  lasttouched = currtouched;
  return;
}


