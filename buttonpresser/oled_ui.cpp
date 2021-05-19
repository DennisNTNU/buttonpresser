#include "oled_ui.hpp"  

#include <Wire.h>
#include <Keyboard.h>
#include <Mouse.h>
#include "ssd1306.hpp"
#include "24AA02E48.hpp"

// *********************************************
// ************* Global Variables **************
// *********************************************

// what input pins have buttons
// or, what input channel maps to which input pin
// input channels:  0-cursor back;  1-cursor next
//                  2-prev char;    3-next char
//                  4-period down;  5-period up
//                  6-reps down;    7-reps up
//                  8-hold;         9-pressing
//                 10-press repeats
byte switchPins[] = {4, 5, 6, 7, 8, 9, 10, 16, A0, A1, 15};
// storing whether a button is pressed or not
byte switchStates[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
byte switchStates_prev[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};


struct OutputChannelConf outputChannels
{
  .key = {0, 1, 1, 1, 1, 1, 1, 1}, // the length of this member variable could change, resulting in a ncompiler error. do changes accordingly
  .period = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
  .repeats = {1, 1, 1, 1, 1, 1, 1, 1},
};
// state for each output channel
byte outputState[OUTPUT_CHANNEL_COUNT] = {RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED, RELEASED};
// for periodic pressing, need timestamps to time the delay
unsigned long output_channel_timestamps[OUTPUT_CHANNEL_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
byte output_repeats_left[OUTPUT_CHANNEL_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};

// cursor position on oled screen
byte cursorPos = 0;
//
unsigned long display_timestamp = 0;
unsigned long last_input_timestamp = 0;

byte oled_off = 0;

// *********************************************
// **************** Functions ******************
// *********************************************

void loadOutputConf()
{
  //write_24AA02E48(addr, data, byte_count);
  read_24AA02E48(0, (byte*)(&outputChannels), sizeof(outputChannels));
}

void saveOutputConf()
{
  write_24AA02E48(0, (byte*)(&outputChannels), sizeof(outputChannels));
}

void initOutputConf()
{
  loadOutputConf();
  outputState[0] = RELEASED;
  outputState[1] = RELEASED;
  outputState[2] = RELEASED;
  outputState[3] = RELEASED;
  outputState[4] = RELEASED;
  outputState[5] = RELEASED;
  outputState[6] = RELEASED;
  outputState[7] = RELEASED;
}

// convert an integer of size up to 9999 to a string
void num2str4positive(int num, char* str)
{
  // constrain num to below 10000
  if (num > 10000)
  { 
    int temp = num / 10000;
    num -= temp*10000;
  }

  char leading_zeros = 0;
  char first_non_zero = -1;
  byte i = 0;

  // most sig digit
  byte dig = num / 1000;
  if (dig == 0 && first_non_zero == -1)
  {
    leading_zeros++;
  }
  else
  {
    first_non_zero = i;
  }
  str[i++] = dig + 48;
  num -= dig*1000;
  
  
  // second most sig digit
  dig = num / 100;
  if (dig == 0 && first_non_zero == -1)
  {
    leading_zeros++;
  }
  else
  {
    first_non_zero = i;
  }
  str[i++] = dig + 48;
  num -= dig*100;
  
  // third most sig digit
  dig = num / 10;
  if (dig == 0 && first_non_zero == -1)
  {
    leading_zeros++;
  }
  else
  {
    first_non_zero = i;
  }
  str[i++] = dig + 48;
  num -= dig*10;

  // least sig digit
  str[i++] = num + 48;

  str[i++] = 0;

  while (leading_zeros > 0)
  {
    str[--leading_zeros] = ' ';
  }
}


// convert an integer of size up to 9999 to a string
void num2str2positive(int num, char* str)
{
  // constrain num to below 100
  if (num > 100)
  { 
    int temp = num / 100;
    num -= temp*100;
  }

  char leading_zeros = 0;
  char first_non_zero = -1;
  byte i = 0;
  
  // third most sig digit
  byte dig = num / 10;
  if (dig == 0 && first_non_zero == -1)
  {
    leading_zeros++;
  }
  else
  {
    first_non_zero = i;
  }
  str[i++] = dig + 48;
  num -= dig*10;

  // least sig digit
  str[i++] = num + 48;

  str[i++] = 0;

  while (leading_zeros > 0)
  {
    str[--leading_zeros] = ' ';
  }
}

char enumOutputKeytoChar(enum OutputKey ik)
{
  if (ik > 43)
  {
    return -1;
  }
  if (ik < 0)
  {
    return -1;
  }

  // alphabeth characters a - z
  if (ik < 26) // (0 ... 25)
  {
    return 97 + ik; // 97 - 122
  }

  // number characters 0 - 9
  if (ik < 36) // (26 ... 35)
  {
    return 48 + ik - 26; // 48 - 57
  }

  if (ik == 41)
  {
    return -2;
  }
  if (ik == 42)
  {
    return -3;
  }
  if (ik == 43)
  {
    return -4;
  }

  // special chars representing mouse buttons, and ctrl
  // enums 36, 37, 38, 39, 40,  41
  return 123 + ik - 36; // 123 - 127
}


// the parameter 'change' is either positive or negative, indicating a desired
// increase or decrease of the period respectively.
void change_period(int* period, char change)
{
  if (change > 0)
  {
    if (*period >= 9800)
    {
      // do nothing
    }
    else if (*period >= 5000)
    {
      *period += 400;
    }
    else if (*period >= 1000)
    {
      *period += 200;
    }
    else if (*period >= 400)
    {
      *period += 50;
    }
    else if (*period >= 200)
    {
      *period += 20;
    }
    else
    {
      *period += 10;
    }
  }
  else
  {
    if (*period <= 20)
    {
      // do nothing
    }
    else if (*period <= 200)
    {
      *period -= 10;
    }
    else if (*period <= 400)
    {
      *period -= 20;
    }
    else if (*period <= 1000)
    {
      *period -= 50;
    }
    else if (*period <= 5000)
    {
      *period -= 200;
    }
    else
    {
      *period -= 400;
    }
  }
}


void read_switches()
{
  // current switch state becomes old switch state
  memcpy(switchStates_prev, switchStates, SWITCH_COUNT);

  for (byte i = 0; i < SWITCH_COUNT; i++)
  {
    int button = digitalRead(switchPins[i]);
    if (button == LOW)
    { // input is active low. Low input means button was pressed, makes
      // switch state go high
      switchStates[i] = HIGH;
    }
    else
    {
      switchStates[i] = LOW;
    }
  }
}








void update_conf()
{
  byte anyConfChange = 0;
  byte shouldUpdateDisplay = 0;

  /*
   * Switches to move cursor on oled screen
   */
  if (switchStates[0] == HIGH && switchStates_prev[0] == LOW)
  {
    cursorPos--;
    display_timestamp = 0; // effectively make display update immediately
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }
  if (switchStates[1] == HIGH && switchStates_prev[1] == LOW)
  {
    cursorPos++;
    display_timestamp = 0; // effectively make display update immediately
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }


  /*
   * Switches to change character
   */
  char charChange = 0;
  if (switchStates[2] == HIGH && switchStates_prev[2] == LOW)
  {
    charChange = -1;
  }
  if (switchStates[3] == HIGH && switchStates_prev[3] == LOW)
  {
    charChange = 1;
  }

  if (charChange != 0 && outputState[cursorPos & 0x7] == RELEASED)
  {
    byte newKey = outputChannels.key[cursorPos & 0x7] + charChange;
    if (newKey == 44)
    {
      newKey = 0;
    }
    else if (newKey > 44) // newKey is an unsigned varible, negative values become large
    {
      newKey = 43;
    }
    outputChannels.key[cursorPos & 0x7] = newKey;
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }


  /*
   * Switches to change period
   */
  if (switchStates[4] == HIGH && switchStates_prev[4] == LOW)
  {
    change_period(&(outputChannels.period[cursorPos & 0x7]), -1);
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }
  if (switchStates[5] == HIGH && switchStates_prev[5] == LOW)
  {
    change_period(&(outputChannels.period[cursorPos & 0x7]), 1);
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }

  /*
   * Switches to change reps
   */
  if (switchStates[6] == HIGH && switchStates_prev[6] == LOW)
  {
    outputChannels.repeats[cursorPos & 0x7] += -1;
    if (outputChannels.repeats[cursorPos & 0x7] > 99)
    {
      outputChannels.repeats[cursorPos & 0x7] = 99;
    }
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }
  if (switchStates[7] == HIGH && switchStates_prev[7] == LOW)
  {
    outputChannels.repeats[cursorPos & 0x7] += 1;
    if (outputChannels.repeats[cursorPos & 0x7] < 1)
    {
      outputChannels.repeats[cursorPos & 0x7] = 1;
    }
    anyConfChange = 1;
    shouldUpdateDisplay = 1;
  }


  /*
   * Switches to send hold or periodic press
   */
  if (switchStates[8] == HIGH && switchStates_prev[8] == LOW)
  {
    if (outputState[cursorPos & 0x7] == RELEASED)
    {
      if (outputChannels.key[cursorPos & 0x7] != 39
          || outputChannels.key[cursorPos & 0x7] != 40)
      {
        outputState[cursorPos & 0x7] = SHALL_HOLD;
        shouldUpdateDisplay = 1;
      }
    }
    else if (outputState[cursorPos & 0x7] == HOLDING)
    {
      if (outputChannels.key[cursorPos & 0x7] != 39
          || outputChannels.key[cursorPos & 0x7] != 40)
      {
        outputState[cursorPos & 0x7] = SHALL_RELEASE;
        shouldUpdateDisplay = 1;
      }
    }
  }


  if (switchStates[9] == HIGH && switchStates_prev[9] == LOW)
  {
    if (outputState[cursorPos & 0x7] == RELEASED)
    {
      outputState[cursorPos & 0x7] = PRESSING;
      output_channel_timestamps[cursorPos & 0x7] = 0;
      shouldUpdateDisplay = 1;
    }
    else if (outputState[cursorPos & 0x7] == PRESSING)
    {
      outputState[cursorPos & 0x7] = RELEASED;
      output_channel_timestamps[cursorPos & 0x7] = 0;
      shouldUpdateDisplay = 1;
    }
  }


  if (switchStates[10] == HIGH && switchStates_prev[10] == LOW)
  {
    if (outputState[cursorPos & 0x7] == RELEASED)
    {
      outputState[cursorPos & 0x7] = PRESSING_N;
      output_repeats_left[cursorPos & 0x7] = outputChannels.repeats[cursorPos & 0x7];
      output_channel_timestamps[cursorPos & 0x7] = 0;
      shouldUpdateDisplay = 1;
    }
    else if (outputState[cursorPos & 0x7] == PRESSING_N)
    {
      outputState[cursorPos & 0x7] = RELEASED;
      output_repeats_left[cursorPos & 0x7] = 0;
      output_channel_timestamps[cursorPos & 0x7] = 0;
      shouldUpdateDisplay = 1;
    }
  }


  if (anyConfChange != 0)
  {
    saveOutputConf();
  }

  if (shouldUpdateDisplay != 0)
  {
    display_timestamp = 0; // effectively make display update immediately
    oled_off = 0;
    last_input_timestamp = millis();
  }
}















void handle_key_input()
{
  for (int i = 0; i < OUTPUT_CHANNEL_COUNT; i++)
  {


    switch (outputState[i])
    {
    case PRESSING:
      {
        // check timestamp and send a press if over time
        unsigned long timestamp = millis();
        if ((timestamp - output_channel_timestamps[i]) > outputChannels.period[i])
        {
          output_channel_timestamps[i] = timestamp;
          enum OutputKey key = (enum OutputKey)outputChannels.key[i];

          // handle keyboard and mouse pressing differently
          if (key < 36)
          {
            Keyboard.print(enumOutputKeytoChar(key));
          }
          else
          {
            // handle mouse button presses
            if (outputChannels.key[i] == key_lmb)
            {
              Mouse.click(MOUSE_LEFT);
            }
            else if (outputChannels.key[i] == key_rmb)
            {
              Mouse.click(MOUSE_RIGHT);
            }
            else if (outputChannels.key[i] == key_mmb)
            {
              Mouse.click(MOUSE_MIDDLE);
            }

            // TODO Handle scrool wheel scrolling
          }
        }
      }
      break;
    case PRESSING_N:
      {
        // check timestamp and send a press if over time
        unsigned long timestamp = millis();
        if ((timestamp - output_channel_timestamps[i]) > outputChannels.period[i])
        {
          output_channel_timestamps[i] = timestamp;
          enum OutputKey key = (enum OutputKey)outputChannels.key[i];

          // handle keyboard and mouse pressing differently
          if (key < 36)
          {
            Keyboard.print(enumOutputKeytoChar(key));
          }
          else
          {
            // handle mouse button presses
            if (outputChannels.key[i] == key_lmb)
            {
              Mouse.click(MOUSE_LEFT);
            }
            else if (outputChannels.key[i] == key_rmb)
            {
              Mouse.click(MOUSE_RIGHT);
            }
            else if (outputChannels.key[i] == key_mmb)
            {
              Mouse.click(MOUSE_MIDDLE);
            }
          }


          output_repeats_left[i]--;
          if (output_repeats_left[i] == 0)
          {
            outputState[i] = RELEASED;
          }
        }
      }
      break;
    case SHALL_HOLD:
      // handle key and mouse buttons differently
      if (outputChannels.key[i] < 36)
      {
        Keyboard.press(enumOutputKeytoChar(outputChannels.key[i]));
      }
      else
      {
        // handle mouse button presses
        if (outputChannels.key[i] == key_lmb)
        {
          Mouse.press(MOUSE_LEFT);
        }
        else if (outputChannels.key[i] == key_rmb)
        {
          Mouse.press(MOUSE_RIGHT);
        }
        else if (outputChannels.key[i] == key_mmb)
        {
          Mouse.press(MOUSE_MIDDLE);
        }
        else if (outputChannels.key[i] == key_ctrl)
        {
          Keyboard.press(KEY_LEFT_CTRL);
        }
        else if (outputChannels.key[i] == key_shift)
        {
          Keyboard.press(KEY_LEFT_SHIFT);
        }
        else if (outputChannels.key[i] == key_alt)
        {
          Keyboard.press(KEY_LEFT_ALT);
        }
      }
      outputState[i] = HOLDING;
      break;
    case SHALL_RELEASE:
      // handle key and mouse buttons differently
      if (outputChannels.key[i] < 36)
      {
        Keyboard.release(enumOutputKeytoChar(outputChannels.key[i]));
      }
      else
      {
        // handle mouse button presses
        if (outputChannels.key[i] == key_lmb)
        {
          Mouse.release(MOUSE_LEFT);
        }
        else if (outputChannels.key[i] == key_rmb)
        {
          Mouse.release(MOUSE_RIGHT);
        }
        else if (outputChannels.key[i] == key_mmb)
        {
          Mouse.release(MOUSE_MIDDLE);
        }
        else if (outputChannels.key[i] == key_ctrl)
        {
          Keyboard.release(KEY_LEFT_CTRL);
        }
        else if (outputChannels.key[i] == key_shift)
        {
          Keyboard.release(KEY_LEFT_SHIFT);
        }
        else if (outputChannels.key[i] == key_alt)
        {
          Keyboard.release(KEY_LEFT_ALT);
        }
      }
      outputState[i] = RELEASED;
      break;
    default:
      break;
    }
  }
}








void conf_to_display()
{
  unsigned long timestamp_new = millis();

  if (oled_off)
  {
    delay(10);
  }
  else
  {
    if (timestamp_new - last_input_timestamp > 60000)
    {
      // If no input for longer than 60 seconds, turn off display
      oled_off = true;
      clearDisplay_I2C();
    }
    // refresh screen every second (normally)
    if ((timestamp_new - display_timestamp) > 400)
    {
      for (int i = 0; i < OUTPUT_CHANNEL_COUNT; i++)
      {
        //const char test[] = "12345678";
        char channel_str[] = ">a 1234H13"; // 12 bytes, 8 spaces, 3 dashes, 1 NULL
        if (i == ((cursorPos) & 0x7))
        {
          channel_str[0] = '>';
        }
        else
        {
          channel_str[0] = ' ';
        }
        channel_str[1] = enumOutputKeytoChar(outputChannels.key[i]);
        channel_str[2] = ' ';

        // char index 3,4,5,6
        num2str4positive(outputChannels.period[i], &(channel_str[3]));

        if (outputState[i] == HOLDING)
        {
          channel_str[7] = 'H';
        }
        else if (outputState[i] == PRESSING)
        {
          channel_str[7] = 'P';
        }
        else if (outputState[i] == PRESSING_N)
        {
          channel_str[7] = 'N';
        }
        else
        {
          channel_str[7] = ' ';
        }

        num2str2positive(outputChannels.repeats[i], &(channel_str[8]));

        byte x = 16*(i & 4);
        byte y = i & 3;
        writeText_I2C(channel_str, x, y);
      }
      display_timestamp = timestamp_new;
    }
  }
}
