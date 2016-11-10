///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////마이크로 프로세서 및 실습 ///////////////////////////////
/////////2013136038 박성준//2013136062 신찬욱//////////////////////////////////////////////////////
///////////////////////////////////////////////Electronic Drum/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include<SoftwareSerial.h>

Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;


#define CRASH_CYMBAL    (49)//
#define RIDE_CYMBAL     (51)//
#define HIGH_HAT        (44)//
#define LOW_FLOOR_TOM   (41)
#define HIGH_FLOOR_TOM  (43)
#define LOW_MID_TOM     (47)
#define HIGH_MID_TOM    (48)
#define LOW_TOM         (45)
#define HIGH_TOM        (50)
#define ACOUSTIC_SNARE  (38)
#define ELECTRIC_SNARE  (40)
#define BASS_DRUM       (35)//

/*VS1053 setup*/
#define VS1053_RX  2
#define VS1053_RESET 9

SoftwareSerial VS1053_MIDI(0, 2);// TX only, do not use the 'rx' side
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
   /*VS1053 setup*/
  VS1053_MIDI.begin(31250);
  /*미디쉴드 리셋*/
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);
}

/*---------------------------------------------------------
    
    L  O  O  P
  
---------------------------------------------------------*/
void loop() {
  currtouched = cap.touched();
  /*현재 감지된 위치의 주소값을 슬레이브모드에서 받아와서 비트연산자를 통해  uint16_t 형식으로 바꾼다, 
  후에 변수 t에 저장 한 후, t & 0x0FFF를 리턴한다. 그러면 0~11 중의 숫자가 리턴된다.*/
  for (uint8_t i = 0; i < 12; i++) {
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {/*#define _BV(bit) (1 << (bit))*/
       Serial.print(i); Serial.println(" touched");
      PercussionMessage(i, 127);/*소리 재생*/
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
      PercussionMessage(i, 0);/*때면 소리 멈춤*/
    }
  }
  lasttouched = currtouched;
  return;
}

void PercussionMessage(byte type, byte velocity) {
  VS1053_MIDI.write(0x90 | 9);//미디프로토콜에서 0x90은 Note On command, channel은 b3~b0 에 있습니다. channel 9는 타악기 입니다.
  switch (type) {
    case 0: VS1053_MIDI.write(CRASH_CYMBAL); break;
    case 1: VS1053_MIDI.write(RIDE_CYMBAL); break;
    case 2: VS1053_MIDI.write(HIGH_HAT); break;
    case 3: VS1053_MIDI.write(LOW_FLOOR_TOM); break;
    case 4: VS1053_MIDI.write(HIGH_FLOOR_TOM); break;
    case 5: VS1053_MIDI.write(LOW_MID_TOM); break;
    case 6: VS1053_MIDI.write(HIGH_MID_TOM); break;
    case 7: VS1053_MIDI.write(LOW_TOM); break;
    case 8: VS1053_MIDI.write(HIGH_TOM); break;
    case 9: VS1053_MIDI.write(ACOUSTIC_SNARE); break;
    case 10: VS1053_MIDI.write(ELECTRIC_SNARE); break;
    case 11: VS1053_MIDI.write(BASS_DRUM); break;
  }
  VS1053_MIDI.write(velocity);//타악기를 치는 속도, 즉 음의 크기가 됩니다.값이 클 수록 큰 소리가 됩니다.
}



