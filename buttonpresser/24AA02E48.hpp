#ifndef E24AA02E48_H
#define E24AA02E48_H

#include <Arduino.h>

// this has 256 Bytes storage. but only 128 bytes are actually writable
#define E24AA02E48_I2C_ADDR (byte)0x50

void testRead(byte i);
byte write_24AA02E48(byte address, byte* data, byte count);
byte read_24AA02E48(byte address, byte* buf, byte count);

#endif
