#ifndef RELAY_H
#define RELAY_H

#define RELAY_Clock	RCC_AHB1Periph_GPIOC
#define RELAY_Port	GPIOC
#define RELAY_Pin	GPIO_Pin_2

/*
 * Relay module used in this project is enabled by low-state.
 */
#define RelayOn()	GPIO_ResetBits(RELAY_Port, RELAY_Pin);
#define RelayOff()	GPIO_SetBits(RELAY_Port, RELAY_Pin);
void RelayInit()
{
	RCC_AHB1PeriphClockCmd(RELAY_Clock, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = RELAY_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(RELAY_Port, &GPIO_InitStruct);

	// Off by default
	RelayOff();
}

#endif
