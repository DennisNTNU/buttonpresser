#include "24AA02E48.hpp"
/*
 * GENERAL INFO ABOUT THE EEPROM CHIP
 * 
 * Can write up to 8 bytes to device. If more is written before a I2C
 * stop condition, the first bytes are overwritten.
 * The Host computer needs to wait a few milliseconds for a write buffer
 * clear after writing up to 8 bytes.
 *
 * A page is 8 bytes and one can only write to one page at a time.
 * I.e. if a write crosses a page boundary, bytes at the beginning of the
 * same page are overwritten.
 * 
 * The last half of the memory array (last 128 bytes) are unwritable
 * 
 * The last 6 bytes contain a EUI-48 node adress, whose first 3 bytes are
 * issued (upon application from Microchip) by IEEE, which is 00 04 A3 ?
 */
#include <Wire.h>

void testRead(byte i)
{ //make read multiple times, using smaller buffer
  byte bufr[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  byte addr = (i & 3) * 64;
  byte retval = read_24AA02E48(addr, bufr, 64);
  Serial1.print(addr, HEX);
  Serial1.print(" -- ");
  Serial1.println(retval, HEX);
  for (byte i = 0; i < 64; i++)
  {
    Serial1.print(bufr[i], HEX);
    Serial1.print(' ');
    if ((i & 0x7) == 0x7)
    {
      Serial1.println("");
    }
  }
  Serial1.println("");
  //Serial1.print(78, HEX) //gives "4E"
}

static byte write_8B_page(byte address, byte* data, byte count)
{
  int delays = 0;
  if (count != 0)
  {
    if (count > 8)
    {
      count = 8;
    }
    Wire.beginTransmission(E24AA02E48_I2C_ADDR);
    Wire.write(address);
    for (int i = 0; i < count; i++)
    {
      Wire.write(data[i]);
    }
    Wire.endTransmission();

    // loop to wait for page write finish, takes about 3ms according to datasheet
    // device does not ack while being busy writing
    byte ret = 2;
    while (ret == 2)
    {
      Wire.beginTransmission(E24AA02E48_I2C_ADDR);
      ret = Wire.endTransmission(); // this takes about 45 us
      delayMicroseconds(355); // plus this its 400 us
      delays++;
    } // total time is about 3500 us
  }

  return delays;
}

byte read_upto_32(byte address, byte* buf, byte count)
{
  byte i = 0;
  if (count != 0)
  {
    if (count > 32)
    {
      count = 32;
    }
    Wire.beginTransmission(E24AA02E48_I2C_ADDR);
    Wire.write(address);
    Wire.endTransmission();

    Wire.requestFrom(E24AA02E48_I2C_ADDR, count);
    while (Wire.available()) // slave may send less than requested
    {
      buf[i++] = Wire.read(); // receive a byte as character
    }
  }
  return i;
}

byte write_24AA02E48(byte address, byte* data, byte count)
{
  byte addr_8_plus = address / 8;
  addr_8_plus = 8*(addr_8_plus + 1);
  
  byte start_remainder = addr_8_plus - address;
  
  byte how_many_8s = (count - start_remainder) / 8;
  
  byte end_remainder = count - start_remainder - how_many_8s*8;

  
  byte written = write_8B_page(address, data, start_remainder);
  for (byte i = 0; i < how_many_8s; i++)
  {
    address = addr_8_plus + i*8;
    written += write_8B_page(address, &(data[start_remainder + i*8]), 8);
  }
  address = addr_8_plus + how_many_8s*8;
  written += write_8B_page(address, &(data[start_remainder + how_many_8s*8]), end_remainder);
  return written;
}

byte read_24AA02E48(byte address, byte* buf, byte count)
{
  byte how_many_32s = count / 32;
  byte remainder = count - how_many_32s * 32;
  byte readed = 0;
  for (byte i = 0; i < how_many_32s; i++)
  {
    readed += read_upto_32(address+i*32, &(buf[i * 32]), 32);
  }
  readed += read_upto_32(address + how_many_32s * 32, &(buf[how_many_32s * 32]), remainder);
  return readed;
}
