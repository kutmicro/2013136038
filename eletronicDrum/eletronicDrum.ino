////////////////////////////////마이크로 프로세서 및 실습//////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////2013136038 박성준//2013136062 신찬욱////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////ELECTRONIC DRUM//////////////////////////////////////////////////


#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <SoftwareSerial.h>

///////////////////////VS1053///////////////////////////////
SoftwareSerial mySerial(2, 3); // RX, TX

byte note = 0; //연주의 음의 값을 저장하는 변수입니다.
byte resetMIDI = 4; //리셋값 입니다.
byte ledPin = 13; //연주중일때 midi에 내장되어있는 led에 빛이 들어옵니다.
int  instrument = 0; //악기의 코드 입니다. 드럼소리는 기본형이라서 0으로 초기화합니다.
////////////////////////////////////////////////////////////


////////////////////////MPR121//////////////////////////////
Adafruit_MPR121 cap = Adafruit_MPR121();//mpr전동센서의 객체를 생성합니다. Adafruit_MPR121 클래스의 함수를 사용할 수 있습니다.

uint16_t recenttouched = 0; //직전에 연주된 번호을 나타냅니다. 
uint16_t currtouched = 0;  //현재 터치된 번호를 나타냅니다.
uint16_t lasttouched = 0; //직접의 입력을 나타냅니다.

//recenttouched와 lasttouched의 차이점
//recenttouched는 직전에 연주된 번호를 가리킵니다. 몇번의 루프문이 돌아도 입력이 없으면 그 값을 유지합니다.
//lasttouched는 직전의 입력, 만약에 한음이 연주된후 다음 루프문 까지 입력이 없으면 null값을 갖게 됩니다. 
////////////////////////////////////////////////////////////


/////////////////////소리별 할당 값/////////////////////////
//note에 저장될 악기 값과 종류를 변수로 선언했습니다.
#define CRASH_CYMBAL    (49)
#define RIDE_CYMBAL     (51)
#define HIGH_HAT        (44)
#define LOW_FLOOR_TOM   (41)
#define HIGH_MID_TOM    (48)
#define HIGH_TOM        (50)
#define ACOUSTIC_SNARE  (38)
#define BASS_DRUM       (35)
////////////////////////////////////////////////////////////

    
//////////////////사용자 정의 변수//////////////////////////
//녹음기능의 구현을 위해서 필요한 세가지인 음의 종류, 음의 크기, 직전 음과의 시간차를 저장하는 배열입니다.
unsigned short recordArr[900][3];
int rows = 0;

unsigned int vol = 0;//음의 크기
unsigned int pre = 0;//앞의 음이 연주되었을때 절대시간
unsigned int next = 0;//뒤의 음이 연주되었을때 절대시간 
//pre와 next를 빼서 시간차를 구합니다.
bool exeRecord = false;//현재 녹음중인지 아닌지를 알려주는 상태변수
unsigned int count = 0;//같은 명령이 또 입력되었는지 알 수 있는 변수
/////////////////////////////////////////////////////////////
void talkMIDI(byte cmd, byte data1, byte data2);//rx에 연결된 midi에 메시지를 보냅니다. 
//cmd : 명령어의 종류, 종류에 따라 data1만 전할지(8,9,A,B), data2까지 전할지(C,D) 결정 됩니다.
void PercussionMessage(byte type, byte sound);//입력받은 번호에 할당된 소리를 기본값 + sound의 크기로 재생합니다.
void record(int note,int volume,int interval);//현재 연주되는 음을 녹음배열에 저장합니다.
void noteOn(byte channel, byte note, byte attack_sound);//악기마다 channel이 정해져있는데, 드럼은 기본값입니다.
//note는 악기종류, attack_sound는 음의 크기를 설정합니다. 내부에서 talkMIDI를 불러 사용합니다. 이 때 명령어는 
//소리를 내라는 명령어인 0x90입니다
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/*                        S  E  T  U  P                    */
/////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(19200);
  Serial.println("hard");
  if (!cap.begin(0x5A)) {/*0x5A를 디폴트 주소값으로 I2C통신을 초기화 하고, 활성화 합니다. 
  그리고 MPR121센서의 위치별(0~12) 주소를 할당합니다. 완료후 true반환*/
    Serial.println("MPR121 not found, check wiring?");//연결이 안되면 할당이 안됩니다.
    while (1);
  }
  Serial.println("MRP121 Connected");

  mySerial.begin(31250);//미디 프로토콜의 기본 통신속도입니다.

   for(int i= 0;i<8;i++){
         for(int j =0 ;j<3;j++){
             recordArr[i][j] = 0;
         }
   }//녹음배열을 초기화합니다. 

  ////////////////////////////midi를 초기화합니다.
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0, 0x78);//0xB0(제어모드 설정명령어) 제어모드를 기본값으로하고, 최고 크기를 max127에 가까운 120으로 설정했습니다.

   pinMode(8, OUTPUT);//연속 재생 알림
   pinMode(12, OUTPUT);//녹음 알림
}
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
/*                        L  O  O  P                        */
//////////////////////////////////////////////////////////////
void loop() {
  talkMIDI(0xC0, instrument, 0); //data1만 전하는 명령어 입니다. 악기 종류를 입력합니다. 드럼은 기본값이므로 0입니다.
  currtouched = cap.touched();
  //Serial.println(currtouched);
  //현재 감지된 위치의 슬레이브 주소값을 받아와서 비트연산자를 통해  uint16_t 형식으로 바꿉니다, 
  //후에 변수 t에 저장 한 후, t & 0x0FFF를 리턴한다. 다음에 _BV(i)와 비교하여 둘다 1인 값인 i값을 감지된걸로 칩니다.
  for (uint8_t i=0; i<11; i++) {
    pre = millis();
    if((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {  //#define _BV( bit ) ( 1<<(bit) )
           switch(i){  //8,9,10은 녹음과 관련된 명령어 들입니다.
             case 8:
                  if(exeRecord == true || rows == 0){return;}//녹음된 내용이 없거나 녹음이 안 끝났으면 명령을 듣지 않습니다.
                  Serial.println("play record");
                     for(int j= 0;j<rows;j++){//앞음과의 시간차 만큼 쉰다음에 다음음을 연주합니다.
                        delay(recordArr[j][2]);
                        PercussionMessage(recordArr[j][0],recordArr[j][1]);//저장된 배열을 연주합니다.
                     }
                  break;
             case 9: 
                  if(exeRecord == true || rows == 0){return ;}//녹음된 내용이 없거나 녹음이 안 끝났으면 명령을 듣지 않습니다.
                  Serial.println("replay");
                  digitalWrite(8, HIGH); //반복 재생중인걸 알리는 led on
                  while(1){
                      int j;
                      for(j= 0;j<rows;j++){
                        delay(recordArr[j][2]);
                        PercussionMessage(recordArr[j][0],recordArr[j][1]);//저장된 배열을 연주합니다.
                      }
                      delay(recordArr[j-2][2]);
                      if(cap.touched() == 512){break;}//같은 명령어가 다시 반복되면 반복재생을 멈춥니다.
                     }
                  delay(1000);
                  digitalWrite(8, LOW); //led off
                  break;
             case 10: 
                if(count % 2 == 0){//처음 눌렀을 때 exeRecor를 true로 해서 녹음을 킵니다.
                 rows = 0;//앞에있던 녹음이 초기화됩니다.
                 digitalWrite(12, HIGH);  //녹음 알리는 led on
                 Serial.println("record start");
                 exeRecord = true;
                 count++; // 다시 한번 누르면 녹음이 종료되도록 했습니다.
                 break;
                }
                else{
                 recordArr[0][2] = 0; //처음이 녹음이 시작되었을 때 음의 앞음과의 시간차는 프로그램 시작부터 현재까지 시간차 이므로 0으로 리셋합니다.
                 digitalWrite(12, LOW);  //led off
                 Serial.println("record end");
                 exeRecord = false; //녹음을 끕니다.
                 count++;//다시 한번 누르면 녹음이 시작되도록했습니다.
                 break;
                }
             default:
                 if(recenttouched == currtouched && (pre-next < 300)){//직전에 연주된 음과 현재 연주된 음이 같은 경우
                     //그 시간차가 0.3초 이내인 경우
                        if(vol < 50){vol += 5;}//소리 크기 범위 : 0~120
                        PercussionMessage(i,vol);//소리를 조금 키웁니다. 반복될 경우 소리가 점점 커지는 효과가 납니다.
                        if(exeRecord == true){record(i,vol,pre-next);}//녹음이 켜있는 상태면 현재 연주된 음을 배열에 저장합니다.
                 } 
                 else{//그 외의 경우는 다시 소리 크기를 원상태로 하고 연주를 진행합니다.
                     vol = 0;
                     PercussionMessage(i, vol);
                     if(exeRecord == true){record(i,vol,pre-next);}
                 }
                 recenttouched = currtouched;//다음에 연주될 음과 비교하도록 현재 연주된 음을 recenttouched에 저장합니다.
                 next = millis();
          }
    }
   /*if(!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ){
        PercussionMessage(i,0);
    }*/
   }
  lasttouched = currtouched;//이 코드를 쓰지 않으면 터치 센서에 접촉되어있는 동안에 계속해서 소리가 새로 납니다.
  return;
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/*                          F U N C T I O N                        */
/////////////////////////////////////////////////////////////////////
void PercussionMessage(byte type, byte sound) {
    switch(type){//선택된 번호에 할당된 악기음이 연주되도록했습니다.
      case 0: noteOn(0,RIDE_CYMBAL, 50+sound); Serial.println("0touched"); break;
      case 1: noteOn(0,LOW_FLOOR_TOM, 50+sound); Serial.println("1touched"); break;
      case 2: noteOn(0,CRASH_CYMBAL, 50+sound); Serial.println("2touched"); break;
      case 3: noteOn(0,HIGH_MID_TOM, 50+sound); Serial.println("3touched"); break;
      case 4: noteOn(0,HIGH_TOM, 50+sound); Serial.println("4touched");break;
      case 5: noteOn(0,HIGH_HAT, 50+sound); Serial.println("5touched");break;
      case 6: noteOn(0,ACOUSTIC_SNARE, 50+sound); Serial.println("6touched");break;
      case 7: noteOn(0,BASS_DRUM, 50+sound); Serial.println("7touched");break;
  }
}
void record(int note,int volume,int interval){
  recordArr[rows][0] = note;//음의 종류
  recordArr[rows][1] = volume;//음의 크기
  recordArr[rows++][2] = interval;//앞 음과의 시간차
}
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);//명령이 들어왔음을 알 수 있습니다.
  mySerial.write(cmd);
  mySerial.write(data1);

  //명령어의 종류에 따라 data2를 사용할지 말지 결정합니다. 
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}
void noteOn(byte channel, byte note, byte attack_sound) {
  talkMIDI( (0x90 | channel), note, attack_sound);
  //note on 명령어를 보냅니다. channel은 0이 입력되므로 기본 값으로 연주됩니다.
}



