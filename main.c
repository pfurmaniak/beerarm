/*
 * Beerarm - project enabling manual control over fridge's compressor.
 *
 * See documentation at: https://github.com/pfurmaniak/beerarm
 *
 * Author: Pawel Furmaniak <pawel.furmaniak@hotmail.com>
 * Date: 2016/08/18
 *
 */

#include "stm32f4xx.h"
#include "tm_stm32f4_ds18b20.h"
#include "tm_stm32f4_hd44780.h"

//#include <stdio.h>

#define EXPECTING_SENSORS	2

int main(void) {
	SystemInit();

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Relay module is enabled by low-state, so enable high-state on PB0
	GPIO_SetBits(GPIOB, GPIO_Pin_0);

	char buf[16];
	uint8_t devices, count, i, j, ac_enabled;
    uint8_t device[EXPECTING_SENSORS][8];
    float temps[EXPECTING_SENSORS];

    TM_OneWire_t OneWire1;
    TM_DELAY_Init();
    TM_OneWire_Init(&OneWire1, GPIOD, GPIO_Pin_0);
    TM_HD44780_Init(16, 2);

    // Check for present devices
    count = 0;
    devices = TM_OneWire_First(&OneWire1);
    while (devices) {
    	count++;

    	TM_OneWire_GetFullROM(&OneWire1, device[count - 1]);

    	devices = TM_OneWire_Next(&OneWire1);
    }

    if (count > 0) {
    	sprintf(buf, "Devices found: %d", count);
    	TM_HD44780_Puts(0, 0, buf);
    } else {
    	TM_HD44780_Puts(0, 0, "No devices found");
    }
    Delayms(3000);

    for (i = 0; i < count; i++) {
    	TM_DS18B20_SetResolution(&OneWire1, device[i], TM_DS18B20_Resolution_12bits);
    }

    while (1) {
    	TM_DS18B20_StartAll(&OneWire1);

    	while (!TM_DS18B20_AllDone(&OneWire1));

    	TM_HD44780_Clear();
    	sprintf(buf, "                ");
    	for (i = 0; i < count; i++) {
    		if (TM_DS18B20_Read(&OneWire1, device[i], &temps[i])) {
    			sprintf(buf, "Temp %d: % 2.2f C", i + 1, temps[i]);
    			TM_HD44780_Puts(0, i, buf);
    		} else {
    			TM_HD44780_Puts(0, i, "Reading error");
    		}
    	}

    	// Enable PB0 if temp1 is over 26C
    	if (temps[0] > 26 && last_temp)


    	if (temps[0] > 26) {
    		GPIO_ResetBits(GPIOB, GPIO_Pin_0);
    	} else {
    		GPIO_SetBits(GPIOB, GPIO_Pin_0);
    	}

    	Delayms(1000);
    }
}
