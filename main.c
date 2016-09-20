/*
 * Beerarm - project enabling manual control over fridge's compressor.
 *
 * See documentation at: https://github.com/pfurmaniak/beerarm
 *
 * Author: Pawel Furmaniak <pawel.furmaniak@hotmail.com>
 * Date: 2016/08/18
 *
 * Default config:
 *   - Built-in button: PA0
 *   - LCD:				SCL - PB6, SDA - PB7
 *   - Relay:			PC0
 *   - OneWire:			PD0
 *
 */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"

/*
 * This program uses external, open-source library developed by Tilen Majerle
 *
 * Visit his website: http://stm32f4-discovery.net/
 *
 */
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_ds18b20.h"
#include "hd44780.h"
#include "button.h"
#include "relay.h"

#define EXPECTING_SENSORS	2

#define TEMP_MIN			6.0
#define TEMP_MAX			22.0
#define TEMP_STEP			0.5

float tempSet = TEMP_MIN;

void EXTI0_IRQHandler()
{
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {

		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
			while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0));

			if (tempSet + TEMP_STEP > TEMP_MAX) {
				tempSet = TEMP_MIN;
			} else {
				tempSet += TEMP_STEP;
			}
		}

		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

int main()
{
	SystemInit();
	ButtonInit();
	RelayInit();
	LCD_Init();

	TM_OneWire_t OneWire1;
	TM_DELAY_Init();
	TM_OneWire_Init(&OneWire1, GPIOD, GPIO_Pin_0);

	uint8_t devCount, dev, i;
	uint8_t ROM[EXPECTING_SENSORS][8];
	float temps[EXPECTING_SENSORS];
	char buf[16];

	// Check for present devices on the OneWire bus.
	devCount = 0;
	dev = TM_OneWire_First(&OneWire1);
	while (dev) {
		devCount++;

		TM_OneWire_GetFullROM(&OneWire1, ROM[devCount - 1]);
		dev = TM_OneWire_Next(&OneWire1);
	}

	if (devCount > 0) {
		sprintf(buf, "%d sensor/s found", devCount);
		LCD_Write(0, 0, buf);
	} else {
		LCD_Write(0, 0, "No sensors found");
		return 1;
	}
	Delayms(1000);

	for (i = 0; i < devCount; i++) {
		TM_DS18B20_SetResolution(&OneWire1, ROM[i], TM_DS18B20_Resolution_12bits);
	}

	while (1) {
		TM_DS18B20_StartAll(&OneWire1);
		while (!TM_DS18B20_AllDone(&OneWire1));

		float temp = 0.0;
		for (i = 0; i < devCount; i++) {
			if (TM_DS18B20_Read(&OneWire1, ROM[i], &temps[i])) {
				temp += temps[i];
			} else {
				LCD_Write(0, 0, "Reading error");
			}
		}
		temp /= devCount;
		if (temp < 10.0) {
			sprintf(buf, "Sensor:  %- .2f C", temp);
		} else {
			sprintf(buf, "Sensor: %- .2f C", temp);
		}
		LCD_Write(0, 0, buf);

		if (tempSet < 10.0) {
			sprintf(buf, "Set:     %- .2f C", tempSet);
		} else {
			sprintf(buf, "Set:    %- .2f C", tempSet);
		}
		LCD_Write(0, 1, buf);

		if (temp > tempSet + 1) {
			RelayOn();
		} else if (temp <= tempSet) {
			RelayOff();
		}
	}
}
