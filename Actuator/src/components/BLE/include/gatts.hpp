#ifndef BLE_INCLUDE_GUARD_
#define BLE_INCLUDE_GUARD_
/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
namespace BLE {

/* Attributes State Machine */
enum {
  IDX_SVC,
  IDX_CHAR_A,
  IDX_CHAR_VAL_A,
  IDX_CHAR_CFG_A,

  IDX_CHAR_B,
  IDX_CHAR_VAL_B,

  IDX_CHAR_C,
  IDX_CHAR_VAL_C,

  HRS_IDX_NB,
};
enum Command {
  CMD_SET_SPEED,
  CMD_CHANGE_SYSTEM_MODE,
  CMD_MOVE_ACTUATOR,
  CMD_PHONE_JOYX,
  CMD_PHONE_JOYY,
  CMD_DRIVE_SPEED,
};
enum Actuator_moving_dir {
  AMD_left,
  AMD_right,
  AMD_stop,
};

enum Speed_setting {
  speed_low,
  speed_medium,
  speed_high,
};
extern enum Speed_setting speedSettings;
extern enum Actuator_moving_dir actMovDir;
void BLEtaskFunction(void *pvParameter);
}; // namespace BLE
#endif
