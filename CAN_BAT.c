/*
 * CAN_BAT.c
 *
 *  Created on: Jan 23, 2024
 *      Author: Jihkadre
 */

#include "CAN_BAT.h"

CAN_BAT_t					CANBAT;
D0_REALTIME_DATA_t 			REALTIME_DATA;
D1_CAPACITY_DATA_t 			CAPACITY_DATA;
D2_ENERGY_DATA_t			ENERGY_DATA;
D3_SAFETY_DATA_t			SAFETY_DATA;
D4_ATTRIBUTE_DATA_t 		ATTRIBUTE_DATA;
P80_CAN_SETTINGS_t			CAN_SETTINGS;
P81_SHA1_SETTINGS_t			SHA1_SETTINGS;
P82_SHA1_MESSAGE_QUERY_t	SHA1_MESSAGE;
P83_BATTERY_ID_t			BATTERY_ID;

#define frame_length_d2					0x11 //17
uint8_t frame_can_Tx_d2[frame_length_d2] = {0x5A, 0x46, 0x4B, 0x4A,0x02, 0x00,0x04,0xBB,0x00, 0x00, 0x00, 0x00,0xFF, 0xFF,0x45, 0x4E, 0x44};

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcanx)
{
	CAN_BAT_READ();
	CAN_BAT_DECODE();
}

#if defined(TIM_SEND)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM7){
		CAN_BAT_REQUEST();
	}
}
#endif

void CAN_BAT_INIT( void ){

	CANBAT.CANBAT_Rx.CANFilter.FilterActivation = CAN_FILTER_ENABLE;    // Enable the filter
	CANBAT.CANBAT_Rx.CANFilter.FilterBank = 0;                          // Choose a filter bank (0 to 27 on some STM32 models)
	CANBAT.CANBAT_Rx.CANFilter.FilterFIFOAssignment = CAN_RX_FIFO0;     // Assign the filter to RX FIFO 0
	CANBAT.CANBAT_Rx.CANFilter.FilterIdHigh = 0x00;				 	    // Set the filter ID
	CANBAT.CANBAT_Rx.CANFilter.FilterIdLow = 0x00;
	CANBAT.CANBAT_Rx.CANFilter.FilterMaskIdHigh = 0x00;   			    // Set the filter mask
	CANBAT.CANBAT_Rx.CANFilter.FilterMaskIdLow =  0x00;
	CANBAT.CANBAT_Rx.CANFilter.FilterMode = CAN_FILTERMODE_IDMASK;      // Use ID masking
	CANBAT.CANBAT_Rx.CANFilter.FilterScale = CAN_FILTERSCALE_32BIT;     // Use 32-bit identifier
	CANBAT.CANBAT_Rx.CANFilter.SlaveStartFilterBank = 0;                // First filter bank for slave nodes

	//Filter configuration
	if(HAL_CAN_ConfigFilter(&CANBUS_PORT, &CANBAT.CANBAT_Rx.CANFilter) != HAL_OK)
		CAN_BAT_ERROR();

	// Start CAN bus
	if(HAL_CAN_Start(&CANBUS_PORT) != HAL_OK)
		CAN_BAT_ERROR();

	// Enable interrupt
	if(HAL_CAN_ActivateNotification(&CANBUS_PORT, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
		CAN_BAT_ERROR();

	CANBAT.CANBAT_Tx.TxHeader.StdId = 0x00;
	CANBAT.CANBAT_Tx.TxHeader.ExtId = SELF_ID;
	CANBAT.CANBAT_Tx.TxHeader.IDE = CAN_ID_EXT;//CAN_ID_STD CAN_ID_EXT
	CANBAT.CANBAT_Tx.TxHeader.RTR = CAN_RTR_DATA;
	CANBAT.CANBAT_Tx.TxHeader.DLC = 8; 			//Number of bytes
	CANBAT.CANBAT_Tx.TxHeader.TransmitGlobalTime = DISABLE;

#if defined(TIM_SEND)
	HAL_TIM_Base_Start_IT(&htim7);
#endif
}


void CAN_BAT_READ( void ){
	if (HAL_CAN_GetRxMessage(&CANBUS_PORT, CAN_RX_FIFO0, &CANBAT.CANBAT_Rx.RxHeader, CANBAT.CANBAT_Rx.RxData) != HAL_OK) {
		CAN_BAT_ERROR();
	}
	LED_GREEN
	memcpy(CANBAT.CANBAT_Rx.RxBuffer+CANBAT.CANBAT_Rx.FrameIndex,
			CANBAT.CANBAT_Rx.RxData,
			CANBAT.CANBAT_Rx.RxHeader.DLC);
	CANBAT.CANBAT_Rx.FrameIndex+=CANBAT.CANBAT_Rx.RxHeader.DLC;

}

inline void CAN_BAT_DECODE( void ){
    for (size_t i = 0; i < CANBAT.CANBAT_Rx.FrameIndex; i++) {
    	if (CANBAT.CANBAT_Rx.RxBuffer[i] == 0x45 &&
    		CANBAT.CANBAT_Rx.RxBuffer[i + 1] == 0x4E &&
			CANBAT.CANBAT_Rx.RxBuffer[i + 2] == 0x44) {
        	if (CANBAT.CANBAT_Rx.RxBuffer[0] == 0x5A &&
        		CANBAT.CANBAT_Rx.RxBuffer[1] == 0x46 &&
				CANBAT.CANBAT_Rx.RxBuffer[2] == 0x4B &&
				CANBAT.CANBAT_Rx.RxBuffer[3] == 0x4A) {
        		switch(CANBAT.CANBAT_Rx.RxBuffer[4]){
					case 0x00:
						REALTIME_DATA.bat_voltage       =((uint16_t)(CANBAT.CANBAT_Rx.RxBuffer[8 ] << 8) | (uint8_t)CANBAT.CANBAT_Rx.RxBuffer[9 ]);
						REALTIME_DATA.bat_current       =(CANBAT.CANBAT_Rx.RxBuffer[10] << 8) | CANBAT.CANBAT_Rx.RxBuffer[11];
						REALTIME_DATA.bat_temp		  	=(CANBAT.CANBAT_Rx.RxBuffer[12] << 8) | CANBAT.CANBAT_Rx.RxBuffer[13];
						REALTIME_DATA.bat_rel_chargstate=(CANBAT.CANBAT_Rx.RxBuffer[14] << 8) | CANBAT.CANBAT_Rx.RxBuffer[15];
						REALTIME_DATA.bat_abs_chargstate=(CANBAT.CANBAT_Rx.RxBuffer[16] << 8) | CANBAT.CANBAT_Rx.RxBuffer[17];
						REALTIME_DATA.bat_state         =(CANBAT.CANBAT_Rx.RxBuffer[18] << 8) | CANBAT.CANBAT_Rx.RxBuffer[19];
						REALTIME_DATA.bat_numbat    	=(CANBAT.CANBAT_Rx.RxBuffer[20] << 8) | CANBAT.CANBAT_Rx.RxBuffer[21];
						REALTIME_DATA.bat_cell1         =(CANBAT.CANBAT_Rx.RxBuffer[22] << 8) | CANBAT.CANBAT_Rx.RxBuffer[23];
						REALTIME_DATA.bat_cell2         =(CANBAT.CANBAT_Rx.RxBuffer[24] << 8) | CANBAT.CANBAT_Rx.RxBuffer[25];
						REALTIME_DATA.bat_cell3         =(CANBAT.CANBAT_Rx.RxBuffer[26] << 8) | CANBAT.CANBAT_Rx.RxBuffer[27];
						REALTIME_DATA.bat_cell4         =(CANBAT.CANBAT_Rx.RxBuffer[28] << 8) | CANBAT.CANBAT_Rx.RxBuffer[29];
						REALTIME_DATA.bat_cell5         =(CANBAT.CANBAT_Rx.RxBuffer[30] << 8) | CANBAT.CANBAT_Rx.RxBuffer[31];
						REALTIME_DATA.bat_cell6         =(CANBAT.CANBAT_Rx.RxBuffer[32] << 8) | CANBAT.CANBAT_Rx.RxBuffer[33];
						REALTIME_DATA.bat_cell7         =(CANBAT.CANBAT_Rx.RxBuffer[34] << 8) | CANBAT.CANBAT_Rx.RxBuffer[35];
						REALTIME_DATA.bat_cell8         =(CANBAT.CANBAT_Rx.RxBuffer[36] << 8) | CANBAT.CANBAT_Rx.RxBuffer[37];
						REALTIME_DATA.bat_cell9         =(CANBAT.CANBAT_Rx.RxBuffer[38] << 8) | CANBAT.CANBAT_Rx.RxBuffer[39];
						REALTIME_DATA.bat_cell10        =(CANBAT.CANBAT_Rx.RxBuffer[40] << 8) | CANBAT.CANBAT_Rx.RxBuffer[41];
						REALTIME_DATA.bat_cell11        =(CANBAT.CANBAT_Rx.RxBuffer[42] << 8) | CANBAT.CANBAT_Rx.RxBuffer[43];
						REALTIME_DATA.bat_cell12        =(CANBAT.CANBAT_Rx.RxBuffer[44] << 8) | CANBAT.CANBAT_Rx.RxBuffer[45];
						REALTIME_DATA.bat_cell13        =(CANBAT.CANBAT_Rx.RxBuffer[46] << 8) | CANBAT.CANBAT_Rx.RxBuffer[47];
						REALTIME_DATA.bat_cell14        =(CANBAT.CANBAT_Rx.RxBuffer[48] << 8) | CANBAT.CANBAT_Rx.RxBuffer[49];
						break;
					case 0x01:
						CAPACITY_DATA.bat_rem_cap       =((CANBAT.CANBAT_Rx.RxBuffer[8 ] << 8) | CANBAT.CANBAT_Rx.RxBuffer[9 ]) * 100;
						CAPACITY_DATA.bat_acc_cap	    =((CANBAT.CANBAT_Rx.RxBuffer[10] << 8) | CANBAT.CANBAT_Rx.RxBuffer[11]) * 100;
						CAPACITY_DATA.bat_designed_cap  =((CANBAT.CANBAT_Rx.RxBuffer[12] << 8) | CANBAT.CANBAT_Rx.RxBuffer[13]) * 100;
						break;
					case 0x02:
						ENERGY_DATA.bat_curr_pow      =(CANBAT.CANBAT_Rx.RxBuffer[8 ] << 8) | CANBAT.CANBAT_Rx.RxBuffer[9 ];
						ENERGY_DATA.bat_pow_marg	  =(CANBAT.CANBAT_Rx.RxBuffer[10] << 8) | CANBAT.CANBAT_Rx.RxBuffer[11];
						break;
					case 0x03:
						SAFETY_DATA.bat_health              =(CANBAT.CANBAT_Rx.RxBuffer[8 ] << 8) | CANBAT.CANBAT_Rx.RxBuffer[9 ];
						SAFETY_DATA.bat_unb_vol	            =(CANBAT.CANBAT_Rx.RxBuffer[10] << 8) | CANBAT.CANBAT_Rx.RxBuffer[11];
						SAFETY_DATA.bat_temp1		        =((CANBAT.CANBAT_Rx.RxBuffer[12] << 8) | CANBAT.CANBAT_Rx.RxBuffer[13])/10;
						SAFETY_DATA.bat_temp2               =((CANBAT.CANBAT_Rx.RxBuffer[14] << 8) | CANBAT.CANBAT_Rx.RxBuffer[15])/10;
						SAFETY_DATA.bat_circul_times        =(CANBAT.CANBAT_Rx.RxBuffer[16] << 8) | CANBAT.CANBAT_Rx.RxBuffer[17];
						SAFETY_DATA.bat_overcharge_times    =(CANBAT.CANBAT_Rx.RxBuffer[18] << 8) | CANBAT.CANBAT_Rx.RxBuffer[19];
						SAFETY_DATA.bat_overdischarge_times =(CANBAT.CANBAT_Rx.RxBuffer[20] << 8) | CANBAT.CANBAT_Rx.RxBuffer[21];
						SAFETY_DATA.bat_overtemp_times      =(CANBAT.CANBAT_Rx.RxBuffer[22] << 8) | CANBAT.CANBAT_Rx.RxBuffer[23];
						SAFETY_DATA.bat_overcurr_times      =(CANBAT.CANBAT_Rx.RxBuffer[24] << 8) | CANBAT.CANBAT_Rx.RxBuffer[25];
						break;
					case 0x04:
						ATTRIBUTE_DATA.bat_nom_volt            =(CANBAT.CANBAT_Rx.RxBuffer[8 ] << 8) | CANBAT.CANBAT_Rx.RxBuffer[9 ];
						ATTRIBUTE_DATA.bat_discharge_rate      =(CANBAT.CANBAT_Rx.RxBuffer[10] << 8) | CANBAT.CANBAT_Rx.RxBuffer[11];
						ATTRIBUTE_DATA.bat_fc_batcell          =(CANBAT.CANBAT_Rx.RxBuffer[12] << 8) | CANBAT.CANBAT_Rx.RxBuffer[13];
						ATTRIBUTE_DATA.bat_bat_safestorage     =(CANBAT.CANBAT_Rx.RxBuffer[14] << 8) | CANBAT.CANBAT_Rx.RxBuffer[15];
						break;
					case 0x80:
						CAN_SETTINGS.bat_can_rate_code = (CANBAT.CANBAT_Rx.RxBuffer[8 ]);
						break;
					case 0x81:
						memcpy(SHA1_SETTINGS.bat_sha1_key,CANBAT.CANBAT_Rx.RxBuffer+8,8);
						break;
					case 0x82:
						memcpy(SHA1_MESSAGE.bat_sha1_message,CANBAT.CANBAT_Rx.RxBuffer+8,4);
						break;
					case 0x83:
						memcpy(BATTERY_ID.bat_ID,CANBAT.CANBAT_Rx.RxBuffer+8,12);
						break;
        		}

        	}
        	memset(CANBAT.CANBAT_Rx.RxBuffer,0x00,60);
        	CANBAT.CANBAT_Rx.FrameIndex = 0;
    	}
    }
}

void CAN_BAT_SEND(const uint8_t* aData, size_t aDataSize) {
    for (size_t i = 0; i < aDataSize; i += 8) {
    	LED_BLUE

        int data_length = (aDataSize - i) < 8 ? (aDataSize - i) : 8;
        CANBAT.CANBAT_Tx.TxHeader.DLC = data_length;

        if (HAL_CAN_AddTxMessage(&CANBUS_PORT, &CANBAT.CANBAT_Tx.TxHeader, &aData[i],  &CANBAT.CANBAT_Tx.TxMailbox) != HAL_OK) {
        	CAN_BAT_ERROR();
        }
    }
}

void CAN_BAT_REQUEST( void ){
	CAN_BAT_SEND(frame_can_Tx_d2,frame_length_d2);

}

inline void CAN_BAT_ERROR( void ){
	LED_RED(1)
}

#if defined(USE_SSD1306)
void CAN_BAT_OLED_INIT( void ){
	ssd1306_Init();
	ssd1306_DrawBitmap(0,0,airaks_logo,128,64,White);
	ssd1306_UpdateScreen();
	HAL_Delay(2000);
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
}
