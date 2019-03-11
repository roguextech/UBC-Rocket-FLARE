/*Cameras Header*/
#ifndef CAMERAS_H
#define CAMERAS_H

/*Includes------------------------------------------------------------*/
#include "sensors.h"

/*Constants------------------------------------------------------------*/
#define SerialCamera        Serial3
#define CameraBaud          115200

/*Variables------------------------------------------------------------*/

/*CAMERA initialization commands*/ //crc is zero initially
// uint8_t power_command[] = {0xCC,0x01,0x01,0x0};
// uint8_t startRecord_command[] = {0xCC,0x01,0x03,0x0};
// uint8_t stopRecord_command[] = {0xCC,0x01,0x04,0x0};
// const uint8_t checkStatus_command[] = {0xCC,0x11,0x0}; not sure yet

/*Functions------------------------------------------------------------*/
void power_cameras(); //a toggle switch
void start_record();
void stop_record();
//void check_status(); //crc stuff
// uint8_t crc_high_first(uint8_t *ptr, unsigned char);
uint8_t crc_calculator(uint8_t *command, uint8_t len);
uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
#endif