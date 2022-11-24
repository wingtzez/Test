#include "M5CoreInk.h"
#include "Numbers_55x40.h"

image_t num55[11] = {
  {40, 55, 1, image_num_01}, {40, 55, 1, image_num_02},
  {40, 55, 1, image_num_03}, {40, 55, 1, image_num_04},
  {40, 55, 1, image_num_05}, {40, 55, 1, image_num_06},
  {40, 55, 1, image_num_07}, {40, 55, 1, image_num_08},
  {40, 55, 1, image_num_09}, {40, 55, 1, image_num_10},
  {24, 55, 1, image_num_11},
};

Ink_Sprite TimePageSprite(&M5.M5Ink);
Ink_Sprite TimeSprite(&M5.M5Ink);

RTC_TimeTypeDef RTCtime, RTCTimeSave;
RTC_TimeTypeDef AlarmTime;
uint8_t second = 0, minutes = 0;

const int STATE_DEFAULT = 0;
const int STATE_EDIT_HOURS = 1;
const int STATE_EDIT_MINUTES = 2;
const int STATE_ALARM = 4;
int program_state = STATE_DEFAULT;

unsigned long rtc_timer = 0;

unsigned long underline_timer = 0;
bool underlineOn = false;

void setup() {
  M5.begin();
  Serial.begin(9600);

  M5.rtc.GetTime(&RTCTimeSave);
  AlarmTime = RTCTimeSave;
  AlarmTime.Minutes = AlarmTime.Minutes + 2;  // set alarm 2 minutes ahead 
  M5.update();
  
  M5.M5Ink.clear();
  delay(500);

  checkRTC();
  
  TimePageSprite.creatSprite(0, 0, 200, 200);
  drawTimePage();
}

void loop() {
  
  if(Serial.available() > 0)
  {
    // input format is "hh:mm" 
    char input[6];
    int charsRead = Serial.readBytes(input, 6);    // read 6 characters 
    if(charsRead == 6 && input[2] == ':') {
      int mm = int(input[4] - '0') + int(input[3] - '0')*10; 
      Serial.print("minutes: ");
      Serial.println(mm);
      int hr = int(input[1] - '0') + int(input[0] - '0')*10; 
      Serial.print("hours: ");
      Serial.println(hr);
      RTCtime.Minutes = mm;
      RTCtime.Hours = hr;
      M5.rtc.SetTime(&RTCtime);      
    }
    else {
      Serial.print("received wrong time format.. ");
      Serial.println(input);
    }
  }

  //if((program_state != STATE_EDIT_HOURS) && (program_state != STATE_EDIT_MINUTES)) {
  if( program_state == STATE_DEFAULT) {
    // state behavior: check and update time every 100ms
    if(millis() > rtc_timer + 100) {
      updateTime();
      rtc_timer = millis();
    }
    // state transition: 
    if ( M5.BtnMID.wasPressed()) {
      AlarmTime = RTCtime;
      program_state = STATE_EDIT_MINUTES;
      Serial.println("program_state => STATE_EDIT_MINUTES");
    }
  }
  else if( program_state == STATE_EDIT_MINUTES) {
    // state behavior:
    if( millis() > underline_timer + 250 ) {
      underlineOn = !underlineOn;
      TimePageSprite.drawString(30, 20, "SET ALARM MINUTES:");
      drawTime(&AlarmTime);
      if(underlineOn)
        TimePageSprite.FillRect(110, 110, 80, 6, 0); // underline minutes black
      else
        TimePageSprite.FillRect(110, 110, 80, 6, 1); // underline minutes white
      TimePageSprite.pushSprite();
      underline_timer = millis();
    }
    // state transition: UP button
    if( M5.BtnUP.wasPressed()) {
      Serial.println("BtnUP wasPressed!");
      if(AlarmTime.Minutes < 60) {
        AlarmTime.Minutes++;
        drawTime(&AlarmTime);
        TimePageSprite.pushSprite();
      }        
    }
    // another state transition: DOWN button
    else if( M5.BtnDOWN.wasPressed()) {
      Serial.println("BtnDOWN wasPressed!");
      if(AlarmTime.Minutes > 0) {
        AlarmTime.Minutes--;
        drawTime(&AlarmTime);
        TimePageSprite.pushSprite();
      }        
    }
    // another state transition: MID button
    else if (M5.BtnMID.wasPressed()) {
      TimePageSprite.FillRect(110, 110, 80, 6, 1); // underline minutes white
      program_state = STATE_EDIT_HOURS;
      Serial.println("program_state => STATE_EDIT_HOURS");
    }
  }
  else if(program_state == STATE_EDIT_HOURS) {

    if(millis() > underline_timer + 250) {
      TimePageSprite.drawString(30, 20, " SET ALARM HOURS: ");
      underlineOn = !underlineOn;
      //drawTimePage();
      drawTime(&AlarmTime);
      if(underlineOn)
        TimePageSprite.FillRect(10, 110, 80, 6, 0); // underline hours black
      else
        TimePageSprite.FillRect(10, 110, 80, 6, 1); // underline hours white
      TimePageSprite.pushSprite();
      underline_timer = millis();
    }

    if( M5.BtnUP.wasPressed()) {
      Serial.println("BtnUP wasPressed!");
      if(AlarmTime.Hours < 24) {
        AlarmTime.Hours++;
        drawTime(&AlarmTime);
        TimePageSprite.pushSprite();
      }        
    }
    else if( M5.BtnDOWN.wasPressed()) {
      Serial.println("BtnDOWN wasPressed!");
      if(AlarmTime.Hours > 0) {
        AlarmTime.Hours--;
        drawTime(&AlarmTime);
        TimePageSprite.pushSprite();
      }        
    }
    else if (M5.BtnMID.wasPressed()) {
      //TimePageSprite.FillRect(10, 110, 80, 6, 1); // underline hours white
      M5.M5Ink.clear();
      TimePageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
      M5.rtc.GetTime(&RTCtime);
      drawTime(&RTCtime);
      TimePageSprite.pushSprite();
      
      program_state = STATE_DEFAULT;
      Serial.println("program_state => STATE_DEFAULT");
    }
  }
  
  M5.update();
}

void drawImageToSprite(int posX, int posY, image_t *imagePtr,
    Ink_Sprite *sprite) {
  sprite->drawBuff(posX, posY, imagePtr->width, imagePtr->height,
    imagePtr->ptr);
}

void updateTime() {
  M5.rtc.GetTime(&RTCtime);
  if (minutes != RTCtime.Minutes) {
    M5.rtc.GetTime(&RTCtime);
    //M5.rtc.GetDate(&RTCDate);

    if (RTCtime.Minutes % 10 == 0) {
      M5.M5Ink.clear();
      TimePageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
    }
    drawTime(&RTCtime);
    //drawDate(&RTCDate);
    TimePageSprite.pushSprite();
    minutes = RTCtime.Minutes;
  }
  //delay(10);
}

void drawTime(RTC_TimeTypeDef *time) {
  drawImageToSprite(10, 48, &num55[time->Hours / 10], &TimePageSprite);
  drawImageToSprite(50, 48, &num55[time->Hours % 10], &TimePageSprite);
  drawImageToSprite(90, 48, &num55[10], &TimePageSprite);
  drawImageToSprite(110, 48, &num55[time->Minutes / 10], &TimePageSprite);
  drawImageToSprite(150, 48, &num55[time->Minutes % 10], &TimePageSprite);

  char buf[8];
  int hr_diff = AlarmTime.Hours - RTCtime.Hours;     
  if( hr_diff < 0) {
    hr_diff = 23 + hr_diff;
  }
  int min_diff = AlarmTime.Minutes - RTCtime.Minutes;
  if( min_diff < 0) {
    min_diff = 59 + min_diff;
    if( hr_diff == 0) {
      hr_diff = 23 + hr_diff;
    }
  }
  sprintf(buf, "%02d:%02d", hr_diff, min_diff);
  TimePageSprite.drawString(45, 130, buf);
  TimePageSprite.drawString(95, 130, "TO ALARM");
  //TimePageSprite.pushSprite();
}

void drawTimePage() {
  M5.rtc.GetTime(&RTCtime);
  drawTime(&RTCtime);
  minutes = RTCtime.Minutes;
  //M5.rtc.GetDate(&RTCDate);
  //drawDate(&RTCDate);
  TimePageSprite.pushSprite();
}

void flushTimePage() {
  // M5.M5Ink.clear();
  // TimePageSprite.clear( CLEAR_DRAWBUFF | CLEAR_LASTBUFF );
  drawTimePage();
  while (1) {
    M5.rtc.GetTime(&RTCtime);
    if (minutes != RTCtime.Minutes) {
      M5.rtc.GetTime(&RTCtime);
      //M5.rtc.GetDate(&RTCDate);

      if (RTCtime.Minutes % 10 == 0) {
        M5.M5Ink.clear();
        TimePageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
      }
      drawTime(&RTCtime);
      //drawDate(&RTCDate);
      TimePageSprite.pushSprite();
      minutes = RTCtime.Minutes;
    }

    delay(10);
    M5.update();
    if (M5.BtnPWR.wasPressed()) {
      digitalWrite(LED_EXT_PIN, LOW);
      M5.shutdown();
    }
    if (M5.BtnDOWN.wasPressed() || M5.BtnUP.wasPressed()) break;
  }
  M5.M5Ink.clear();
  TimePageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
}

void checkRTC() {
  M5.rtc.GetTime(&RTCtime);
  if (RTCtime.Seconds == RTCTimeSave.Seconds) {
    //drawWarning("RTC Error");
    Serial.println("RTC Error");
    while (1) {
      if (M5.BtnMID.wasPressed()) return;
      delay(10);
      M5.update();
    }
  }
}
