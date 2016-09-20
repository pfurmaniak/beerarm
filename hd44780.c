#include "hd44780.h"

#define LCD_I2C_ADDR		0x27 // 0010 0111

#define LCD_I2C_EN			0x04
#define LCD_I2C_RW			0x02
#define LCD_I2C_RS			0x01

#define LCD_I2C_DATA		LCD_I2C_RS
#define LCD_I2C_CMD			0x00

/* Commands */
#define LCD_CLEARDISPLAY	0x01
#define LCD_CURSORHOME		0x02
#define LCD_ENTRYMODESET	0x04
#define LCD_DISPLAYCONTROL	0x08
#define LCD_CURSORSHIFT		0x10
#define LCD_FUNCTIONSET		0x20
#define LCD_SETCGRAMADDR	0x40
#define LCD_SETDDRAMADDR	0x80

/* ENTRYMODESET flags */
#define LCD_INCREMENT		0x02
#define LCD_DECREMENT		0x00
#define LCD_SHIFT			0x01
#define LCD_NOSHIFT			0x00

/* DISPLAYCONTROL flags */
#define LCD_DISPLAYON		0x04
#define LCD_DISPLAYOFF		0x00
#define LCD_CURSORON		0x02
#define LCD_CURSOROFF		0x00
#define LCD_BLINKON			0x01
#define LCD_BLINKOFF		0x00

/* CURSORSHIFT flags */
#define LCD_DISPLAYMOVE		0x08
#define LCD_CURSORMOVE      0x00
#define LCD_MOVERIGHT       0x04
#define LCD_MOVELEFT        0x00

/* FUNCTIONSET flags */
#define LCD_8BITS			0x10
#define LCD_4BITS			0x00
#define LCD_2LINE			0x08
#define LCD_1LINE			0x00
#define LCD_5x10DOTS		0x04
#define LCD_5x8DOTS			0x00

/* BACKLIGHT flags */
#define LCD_BACKLIGHT		0x08
#define LCD_NOBACKLIGHT		0x00

/* Private global variables */
uint8_t _currentX = 0;
uint8_t _currentY = 0;
uint8_t _address = LCD_I2C_ADDR << 1;
uint8_t _backlight = LCD_BACKLIGHT;

/* Private functions */
void LCD_I2C_Init();
void LCD_I2C_Write(uint8_t address, uint8_t data);

void LCD_Send(uint8_t data, uint8_t mode);
void LCD_Send4Bits(uint8_t data);
void LCD_PulseEnable(uint8_t data);

void LCD_CursorSet(uint8_t x, uint8_t y);

void LCD_Init()
{
	TM_DELAY_Init();
	LCD_I2C_Init();

	Delayms(50);

	LCD_I2C_Write(_address, _backlight);
	Delayms(1000);

	// Put the LCD into 4-bit mode
	LCD_Send4Bits(0x03 << 4);
	Delay(4500);
	LCD_Send4Bits(0x03 << 4);
	Delay(4500);
	LCD_Send4Bits(0x03 << 4);
	Delay(4500);
	LCD_Send4Bits(0x02 << 4);

	LCD_Send(LCD_FUNCTIONSET | LCD_4BITS | LCD_2LINE | LCD_5x8DOTS, LCD_I2C_CMD);
	LCD_Send(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF, LCD_I2C_CMD);
	LCD_Send(LCD_CLEARDISPLAY, LCD_I2C_CMD);
	Delay(2000);
	LCD_Send(LCD_ENTRYMODESET | LCD_INCREMENT | LCD_NOSHIFT, LCD_I2C_CMD);
	LCD_Send(LCD_CURSORHOME, LCD_I2C_CMD);
	Delay(2000);
}
void LCD_Write(uint8_t x, uint8_t y, char* str)
{
	LCD_CursorSet(x, y);
	while (*str) {
		if (_currentX >= 16) {
			_currentX = 0;
			_currentY++;
			LCD_CursorSet(_currentX, _currentY);
		}
		if (*str == '\n') {
			_currentY++;
			LCD_CursorSet(_currentX, _currentY);
		} else if (*str == '\r') {
			LCD_CursorSet(0, _currentY);
		} else {
			LCD_Send(*str, LCD_I2C_DATA);
			_currentX++;
		}
		str++;
	}
}

void LCD_BacklightOn()
{
	_backlight = LCD_BACKLIGHT;
	LCD_I2C_Write(_address, _backlight);
}
void LCD_BacklightOff()
{
	_backlight = LCD_NOBACKLIGHT;
	LCD_I2C_Write(_address, _backlight);
}


/* Private functions */
void LCD_I2C_Init()
{
	// Variables
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	// Enable clocks
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	// SCL on PB6, SDA on PB7
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1); // SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA

	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C_InitStruct);
	I2C_Cmd(I2C1, ENABLE);
}
void LCD_I2C_Write(uint8_t address, uint8_t data)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, data);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_GenerateSTOP(I2C1, ENABLE);
}

void LCD_Send(uint8_t data, uint8_t mode)
{
	LCD_Send4Bits((data & 0xF0) | mode);
	LCD_Send4Bits(((data << 4) & 0xF0) | mode);
}
void LCD_Send4Bits(uint8_t data)
{
	LCD_I2C_Write(_address, data | _backlight);
	LCD_PulseEnable(data);
}
void LCD_PulseEnable(uint8_t data)
{
	LCD_I2C_Write(_address, data | LCD_I2C_EN | _backlight);
	Delay(1);
	LCD_I2C_Write(_address, (data & ~LCD_I2C_EN) | _backlight);
	Delay(50);
}

void LCD_CursorSet(uint8_t x, uint8_t y)
{
	uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};

	_currentX = x;
	_currentY = y;

	LCD_Send(LCD_SETDDRAMADDR | (x + row_offsets[y]), LCD_I2C_CMD);
}
