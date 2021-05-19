#include <string.h>
#include <SPI.h>
#include <Wire.h>

#include "SSD1306.hpp"

/*
byte placeholder_DDRB = 0;
byte placeholder_PORTB = 0;

#define DDRB placeholder_DDRB
#define PORTB placeholder_PORTB
*/

static byte width = 128;
static byte height = 64;

const SPISettings spiconf = SPISettings(20000000, MSBFIRST, SPI_MODE0);

byte char_to_font_index(char chr)
{
  if (chr == -2)
  {
    return 96;
  }
  else if (chr == -3)
  {
    return 97;
  }
  else if (chr == -4)
  {
    return 98;
  }
  else if (chr < 32)
  {
    return 255;
  }
  else
  {
    return chr - 32;
  }
}

void setupOled_SPI()
{
  SPI.begin();
 
  // setting pins D8, D9 & D10
  //             Res, DC, CS      to output
  DDRB |= 0b00000111;
  
  // clearing reset and D/C
  PORTB &= ~(0b00000011);
  
  delay(10);
  
  // setting reset pin and CS to high
  PORTB |= 0b00000101;
  
  delay(10);
  

  SPI.beginTransaction(spiconf);
  CS_LOW
  SPI.transfer(0xAF); // turn display on
  SPI.transfer(0xB0); // set page address
  SPI.transfer(0x81); // set contrast
  SPI.transfer(0x0F); // to 0x07F
  SPI.transfer(0x20); // set addressing mode
  SPI.transfer(0x00); // to horizontal
  SPI.transfer(0x8D); // chargepump
  SPI.transfer(0x14); // enable
  SPI.transfer(0xA1); // mirror around vertical axis
  SPI.transfer(0xC8); // mirror around horizontal axis?
  CS_HIGH
  SPI.endTransaction();

  setWriteAddress_SPI(0, 0);

  clearDisplay_SPI();
}

void clearDisplay_SPI()
{
  int n = 1024;
  DATA_MODE
  SPI.beginTransaction(spiconf);
  CS_LOW
  while (n--)  SPI.transfer(0);
  CS_HIGH
  SPI.endTransaction();
}

void setTestPattern_SPI()
{
  int n = 1024;
  DATA_MODE

  SPI.beginTransaction(spiconf);
  CS_LOW
  while (n--)  SPI.transfer(n);
  CS_HIGH
  SPI.endTransaction();
}

void writeFont_SPI()
{
  DATA_MODE
  
  byte char_count = 91;
  SPI.beginTransaction(spiconf);
  CS_LOW
  while (char_count--)
  {
    SPI.transfer(font[90-char_count][0]);
    SPI.transfer(font[90-char_count][1]);
    SPI.transfer(font[90-char_count][2]);
    SPI.transfer(font[90-char_count][3]);
    SPI.transfer(font[90-char_count][4]);
    SPI.transfer(0);
  }
  CS_HIGH
  SPI.endTransaction();
}

void writeString_SPI(const char* string)
{
  int len = strlen(string);
  int i = 0;
  char chr;
  byte index; 

  DATA_MODE
  
  SPI.beginTransaction(spiconf);
  CS_LOW
  while (len--)
  {
    index = -1;
    chr = string[i++];
    if (chr < 32)
    {
      continue;
    }
    else if (chr < 123)
    {
      index = chr - 32;
    }
    else 
    {
      continue;
    }
    SPI.transfer(font[index][0]);
    SPI.transfer(font[index][1]);
    SPI.transfer(font[index][2]);
    SPI.transfer(font[index][3]);
    SPI.transfer(font[index][4]);
    SPI.transfer(0);
  }
  CS_HIGH
  SPI.endTransaction();
}

void initScroll_SPI()
{
  COMMAND_MODE
  
  SPI.beginTransaction(spiconf);
  CS_LOW
  SPI.transfer(0x26); // right horizontal scroll
  SPI.transfer(0x00); // dummy dum dum
  SPI.transfer(0x00); // starting page address?
  SPI.transfer(0x00); // time between scroll steps?
  SPI.transfer(0x07); // end page address?
  SPI.transfer(0x00); // dummy dum
  SPI.transfer(0xFF); // dum dummy
  CS_HIGH
  SPI.endTransaction();
}

void startScroll_SPI()
{
  COMMAND_MODE
  
  SPI.beginTransaction(spiconf);
  CS_LOW
  SPI.transfer(0x2F);
  CS_HIGH
  SPI.endTransaction();
}

void stopScroll_SPI()
{
  COMMAND_MODE
  
  SPI.beginTransaction(spiconf);
  CS_LOW
  SPI.transfer(0x2E);
  CS_HIGH
  SPI.endTransaction();
}


// set write coordinates 
// x in {0,127}, y in {0,7}
void setWriteAddress_SPI(byte x, byte y)
{
  COMMAND_MODE

  SPI.beginTransaction(spiconf);
  CS_LOW
  SPI.transfer(0x21); // column
  SPI.transfer(x); // start address
  SPI.transfer(0x7F); // and end address
  SPI.transfer(0x22); // page
  SPI.transfer(y); // start address
  SPI.transfer(0x07); // and end address
  CS_HIGH
  SPI.endTransaction();
}

void writeText_SPI(const char* string, byte x, byte y)
{
  setWriteAddress_SPI(x,y);
  writeString_SPI(string);
}
















// set write coordinates 
// x in {0,127}, y in {0,3}
void setWriteAddress_I2C(byte x, byte y)
{
  if (y > 3)
  {
    y = 3;
  }
  Wire.beginTransmission(SSD1306_I2C_ADDR);
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(x & 0xF); // lower nibble
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(((x & 0xF0) >> 4) | 0x10); // higher nibble
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x21); // column ..
  Wire.write(x); // start address ..
  Wire.write(0x7F); // and end address
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x22); // page ..
  Wire.write(y); // start address ..
  Wire.write(0x03); // and end address
  Wire.endTransmission();
}

void setWriteRect_I2C(byte x, byte y, byte lx, byte ly)
{
  if (y > 3)
  {
    y = 3;
  }
  Wire.beginTransmission(SSD1306_I2C_ADDR);
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x21); // column ..
  Wire.write(x); // start address ..
  Wire.write(x+lx); // and end address
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x22); // page ..
  Wire.write(y); // start address ..
  Wire.write(y+ly); // and end address
  Wire.endTransmission();
}

void clearDisplay_I2C()
{
  setWriteAddress_I2C(0, 0);
  int byte_count = width*height/8;
  byte maxloops = byte_count/31 + 1;
  byte_count = 0;
  for (int i = 0; i < maxloops; i++)
  {
    Wire.beginTransmission(SSD1306_I2C_ADDR);
    Wire.write(SSD1306_I2C_DATA);

    for (int i = 0; i < 31; i++)
    {
      Wire.write(0x00);
      //byte_count++;
    }
    Wire.endTransmission();
  }
}

void setTestPattern_I2C()
{
  setWriteAddress_I2C(0, 0);
  int byte_count = width*height/8;
  byte maxloops = byte_count/31 + 1;
  byte_count = 0;
  for (int i = 0; i < maxloops; i++)
  {
    Wire.beginTransmission(SSD1306_I2C_ADDR);
    Wire.write(SSD1306_I2C_DATA);

    for (int i = 0; i < 31; i++)
    {
      Wire.write(byte_count);
      byte_count++;
    }
    Wire.endTransmission();
  }
}

void writeFont_I2C()
{
  byte char_count = 91;
  while (char_count--)
  {
      Wire.beginTransmission(SSD1306_I2C_ADDR);
      Wire.write(SSD1306_I2C_DATA);
      Wire.write(font[90 - char_count][0]);
      Wire.write(font[90 - char_count][1]);
      Wire.write(font[90 - char_count][2]);
      Wire.write(font[90 - char_count][3]);
      Wire.write(font[90 - char_count][4]);
      Wire.write(0);
      Wire.endTransmission();
  }
}

void setupOled_I2C(int width_, int height_)
{

  width = width_;
  height = height_;

  delay(50);

  Wire.beginTransmission(SSD1306_I2C_ADDR);
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0xAF); // turn display on
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x81); // set contrast
  Wire.write(0x0F); // to 0x07F
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x20); // set addressing mode
  Wire.write(0x00); // to horizontal
  //Wire.write(0x01); // to vertical
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0xDA); // COM pin configuration
  Wire.write(0x02); // 
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x8D); // chargepump
  Wire.write(0x14); // enable
  Wire.write(SSD1306_I2C_COMMAND);
  Wire.write(0x40); // Set display ram start line reg to 0
  
  Wire.write(SSD1306_I2C_COMMAND);
  //Wire.write(0xA5); // display entire display ON
  Wire.write(0xA4); // display RAM contents
  //Wire.write(SSD1306_I2C_COMMAND);
  //Wire.write(0xA1); // mirror around vertical axis, write A0 to disable
  //Wire.write(SSD1306_I2C_COMMAND);
  //Wire.write(0xC8); // mirror around horizontal axis, write C0 to disable
  Wire.endTransmission();

  delay(150);

  clearDisplay_I2C();
  setWriteAddress_I2C(0, 0);
}


void writeString_I2C(const char* string)
{
  int len = strlen(string);
  int i = 0;
  byte index;

  while (len--)
  {
    index = char_to_font_index(string[i++]);
    if (index == 255)
    {
      continue;
    }

    Wire.beginTransmission(SSD1306_I2C_ADDR);
    Wire.write(SSD1306_I2C_DATA);
    Wire.write(font[index][0]);
    Wire.write(font[index][1]);
    Wire.write(font[index][2]);
    Wire.write(font[index][3]);
    Wire.write(font[index][4]);
    Wire.write(0);
    Wire.endTransmission();
  }
}

void writeText_I2C(const char* string, byte x, byte y)
{
  setWriteAddress_I2C(x, y);
  writeString_I2C(string);
}
