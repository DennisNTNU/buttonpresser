#ifndef OLED_UI_H
#define OLED_UI_H

#include <Arduino.h>

#define OUTPUT_CHANNEL_COUNT 8
#define SWITCH_COUNT 11

// press actions
#define RELEASED 0
#define SHALL_HOLD 1
#define HOLDING 2
#define SHALL_RELEASE 3
#define PRESSING 4
#define PRESSING_N 5

enum OutputKey {
  key_a, // 0
  key_b, // 1
  key_c, // 2
  key_d, // 3
  key_e, // 4
  key_f, // 5
  key_g, // 6
  key_h, // 7
  key_i, // 8
  key_j, // 9
  key_k, // 10
  key_l, // 11
  key_m, // 12
  key_n, // 13
  key_o, // 14
  key_p, // 15
  key_q, // 16
  key_r, // 17
  key_s, // 18
  key_t, // 19
  key_u, // 20
  key_v, // 21
  key_w, // 22
  key_x, // 23
  key_y, // 24
  key_z, // 25
  key_0, // 26
  key_1, // 27
  key_2, // 28
  key_3, // 29
  key_4, // 30
  key_5, // 31
  key_6, // 32
  key_7, // 33
  key_8, // 34
  key_9, // 35
  key_lmb, // 36
  key_rmb, // 37
  key_mmb, // 38
  key_scrlup, // 39
  key_scrldwn, // 40
  key_ctrl, // 41
  key_shift, // 42
  key_alt // 43
};

struct __attribute__((packed)) OutputChannelConf
{
  //enum OutputKey output_keys[8];

  // stores the currently configured key enum for each channel
  byte key[8]; // 8 Bytes
  // stores the currently configured period for each output channel
  int period[8]; // 16 Bytes (sizeof(int) = 2 Bytes)
  // stores the repeat count
  byte repeats[8]; // 8 Bytes
};

// read the OutputChannelConf sctruct from EEPROM
void loadOutputConf();
// write the OutputChannelConf sctruct to EEPROM
void saveOutputConf();



// read the OutputChannelConf sctruct from EEPROM
// and reset press states
void initOutputConf();


// this funtion updates a global switch state array that
// mirrors the state of the physical buttons
void read_switches();
//void handle_switch_input();
void update_conf();
void handle_key_input();
void conf_to_display();










/*
#define KEY_INPUT_COUNT 4

struct __attribute__((packed)) input_configuration
{
  char keyboard_config[KEY_INPUT_COUNT];
  byte cursor_pos;
  int period_RMB;
  int period_LMB;
  int period_MMB;
  int period_scrl_up;
  int period_scrl_dn;
  
  //.switches = {'w', 'w', 'w', 'w', 0},
};

void display_current_config();


 // display current config
 // display user input
struct switch_chars_state
{
  unsigned long timestamp;
  unsigned long timestamp_s_btn;
  unsigned long timestamp_c_btn;
  
  byte switch_pins[SWTCH_COUNT];
  char switches[SWTCH_COUNT+1];
  
  byte active_switch;

  byte active_switch_print_last;
  byte switch_change_btn;
  byte char_change_btn_up;
  byte char_change_btn_down;
};

void handle_switch_btn_input();
void display_switch_chars();
*/
#endif
