#include <Wire.h>
#include <Keyboard.h>
#include <Mouse.h>

#include "SSD1306.hpp"
#include "24AA02E48.hpp"
#include "oled_ui.hpp"

void setup()
{
  //pinMode(2, INPUT_PULLUP); // these are I2C pins
  //pinMode(3, INPUT_PULLUP); // these are I2C pins
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  //pinMode(14, INPUT_PULLUP); // unused
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);

  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);

  Mouse.begin();
  Keyboard.begin();

  Wire.begin();
  Wire.setClock(400000);

  Serial1.begin(115200);
  //Serial1.begin(230400);
  //Serial1.begin(307200); // invalid baud
  //Serial1.begin(460800); // doesnt work
  //Serial1.begin(500000);
  //Serial1.begin(576000); // doesnt work
  //Serial1.begin(921600); // doesnt work
  //Serial1.begin(1000000); // doesnt work
  //Serial1.begin(1152000); // doesnt work
  //Serial1.begin(2000000); // doesnt work

  initOutputConf();

  testRead(0);

  Serial1.print("Hello\r\n");

  delay(2000);
  setupOled_I2C(128, 32);

}

void loop()
{
  read_switches();

  update_conf();

  handle_key_input();

  conf_to_display();

  

  while (Serial1.available() > 0)
  {
    Serial1.write(Serial1.read());
  }

  delay(20);
}




























/*

#include <Keyboard.h>

// media key HID ID definitions

#define VOL_MUTE 0x7f
#define VOL_UP 0x80
#define VOL_DOWN 0x81

#define KEY_MEDIA_PLAYPAUSE 0xe8
#define KEY_MEDIA_STOPCD 0xe9
#define KEY_MEDIA_PREVIOUSSONG 0xea
#define KEY_MEDIA_NEXTSONG 0xeb
#define KEY_MEDIA_EJECTCD 0xec
#define KEY_MEDIA_VOLUMEUP 0xed
#define KEY_MEDIA_VOLUMEDOWN 0xee
#define KEY_MEDIA_MUTE 0xef
#define KEY_MEDIA_WWW 0xf0
#define KEY_MEDIA_BACK 0xf1
#define KEY_MEDIA_FORWARD 0xf2
#define KEY_MEDIA_STOP 0xf3
#define KEY_MEDIA_FIND 0xf4
#define KEY_MEDIA_SCROLLUP 0xf5
#define KEY_MEDIA_SCROLLDOWN 0xf6
#define KEY_MEDIA_EDIT 0xf7
#define KEY_MEDIA_SLEEP 0xf8
#define KEY_MEDIA_COFFEE 0xf9
#define KEY_MEDIA_REFRESH 0xfa
#define KEY_MEDIA_CALC 0xfb


void setup()
{

  pinMode(2, INPUT_PULLUP);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  sendMediaKey(KEY_MEDIA_VOLUMEUP);
  sendMediaKey(VOL_UP);
  delay(200);
  sendMediaKey(KEY_MEDIA_VOLUMEUP);
  sendMediaKey(VOL_UP);
  delay(2000);
  sendMediaKey(VOL_MUTE);
  sendMediaKey(KEY_MEDIA_MUTE);
  
  
  sendMediaKey(KEY_MEDIA_VOLUMEDOWN);
  sendMediaKey(VOL_DOWN);
  delay(200);
  sendMediaKey(KEY_MEDIA_VOLUMEDOWN);
  sendMediaKey(VOL_DOWN);
  delay(2000);
  sendMediaKey(VOL_MUTE);
  sendMediaKey(KEY_MEDIA_MUTE);

  if (digitalRead(2) == LOW)
  {
    sendMediaKey(0x04);
  }
}





void sendKey(byte key, byte modifiers)
{
  KeyReport report = {0};  // Create an empty KeyReport
  
  // First send a report with the keys and modifiers pressed 
  report.keys[0] = key;  // set the KeyReport to key
  report.modifiers = modifiers;  // set the KeyReport's modifiers
  report.reserved = 1;
  Keyboard.sendReport(&report);  // send the KeyReport
  
  // Now we've got to send a report with nothing pressed
  for (int i=0; i<6; i++)
    report.keys[i] = 0;  // clear out the keys
  report.modifiers = 0x00;  // clear out the modifires
  report.reserved = 0;
  Keyboard.sendReport(&report);  // send the empty key report
}

void sendMediaKey(byte key)
{
  sendKey(key, 0);
}


*/
