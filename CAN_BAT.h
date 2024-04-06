/*
 * CAN_BAT.h
 *
 *  Created on: Jan 23, 2024
 *      Author: Jihkadre
 */

#pragma once


#include "CAN_BAT_config.h"
#include <string.h>
#include "stdint.h"
#include "stdio.h"

#if defined(USE_SSD1306)
#include "ssd1306/ssd1306.h"
#endif

typedef struct CANBAT_Rx_s{
	CAN_RxHeaderTypeDef RxHeader; 		// CAN Bus Receive Header
	CAN_FilterTypeDef CANFilter;		// CAN BUS FILTER
	uint8_t FrameIndex;
	uint8_t RxData[8];
	uint8_t RxBuffer[60];
}CANBAT_Rx_t;

typedef struct CANBAT_Tx_s{
	CAN_TxHeaderTypeDef TxHeader; 		// CAN Bus Transmit Header
	uint32_t TxMailbox;					// Tx MailBox
	uint8_t TxData[8];
	uint8_t TxBuffer[60];
}CANBAT_Tx_t;
/*
typedef struct CANBAT_msg_s{
	uint8_t FRAME_HEADER[4];
	uint8_t COMMAND[2];
	uint8_t LOAD_LENGTH[1];
	uint8_t LOAD_MARK[1];
	uint8_t LOAD[40];
	uint8_t CHECKOUT[2];
}CANBAT_msg_t;
*/

typedef struct CAN_BAT_s{
	CANBAT_Tx_t CANBAT_Tx;
	CANBAT_Rx_t CANBAT_Rx;
}CAN_BAT_t;

typedef struct D0_REALTIME_DATA_s{
	uint16_t bat_voltage;
	uint16_t bat_current;
	uint16_t bat_temp;
	uint16_t bat_rel_chargstate;
	uint16_t bat_abs_chargstate;
	uint16_t bat_state;			//STATE
	uint16_t bat_numbat;
	uint16_t bat_cell1;
	uint16_t bat_cell2;
	uint16_t bat_cell3;
	uint16_t bat_cell4;
	uint16_t bat_cell5;
	uint16_t bat_cell6;
	uint16_t bat_cell7;
	uint16_t bat_cell8;
	uint16_t bat_cell9;
	uint16_t bat_cell10;
	uint16_t bat_cell11;
	uint16_t bat_cell12;
	uint16_t bat_cell13;
	uint16_t bat_cell14;
}D0_REALTIME_DATA_t;

typedef struct D1_CAPACITY_DATA_s{
	uint16_t bat_rem_cap; 			//Remaining Capacity
	uint16_t bat_acc_cap;			//Actual Capacity
	uint16_t bat_designed_cap; 		//Designed Capacity
}D1_CAPACITY_DATA_t;

typedef struct D2_ENERGY_DATA_s{
	uint16_t bat_curr_pow;
	uint16_t bat_pow_marg;
}D2_ENERGY_DATA_t;

typedef struct D3_SAFETY_DATA_s{
	uint16_t bat_health;
	uint16_t bat_unb_vol;
	uint16_t bat_temp1;
	uint16_t bat_temp2;
	uint16_t bat_circul_times;
	uint16_t bat_overcharge_times;
	uint16_t bat_overdischarge_times;
	uint16_t bat_overtemp_times;
	uint16_t bat_overcurr_times;
}D3_SAFETY_DATA_t;

typedef struct D4_ATTRIBUTE_DATA_s{
	uint16_t bat_nom_volt;
	uint16_t bat_discharge_rate;
	uint16_t bat_fc_batcell;
	uint16_t bat_bat_safestorage;
}D4_ATTRIBUTE_DATA_t;

typedef struct P80_CAN_SETTINGS_s{
	uint8_t bat_can_rate_code;
}P80_CAN_SETTINGS_t;

typedef struct P81_SHA1_SETTINGS_s{
	uint8_t  bat_sha1_key[8];
}P81_SHA1_SETTINGS_t;

typedef struct P82_SHA1_MESSAGE_QUERY_s{
	uint8_t  bat_sha1_message[4];
}P82_SHA1_MESSAGE_QUERY_t;

typedef struct P83_BATTERY_ID_s{
	uint8_t  bat_ID[12];
}P83_BATTERY_ID_t;



void CAN_BAT_INIT( void );

void CAN_BAT_SEND(const uint8_t* aData, size_t aDataSize);
void CAN_BAT_READ( void );
void CAN_BAT_DECODE( void );

void CAN_BAT_REQUEST( void );

void CAN_BAT_ERROR( void );

#if defined(USE_SSD1306)
void CAN_BAT_OLED_INIT( void );
void CAN_BAT_OLED_SEND( void );
#endif

extern CAN_HandleTypeDef CANBUS_PORT;
#if defined(TIM_SEND)
extern TIM_HandleTypeDef TIM_PORT;
#endif

