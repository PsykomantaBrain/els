#pragma once
#include <Arduino.h>


// LCD layout consts -------------------------------
#define C_FIELD0 0 //1234
#define C_FIELD1 5 //6789
#define C_FIELD2 10//1234
#define C_FIELD3 15//6789



struct PageValueRegion
{

private:
	char buffer[16];

public:
	uint8_t length;

	volatile int* linkedValue = nullptr;

	PageValueRegion(uint8_t l, volatile int* val)
	{
		length = l;
		linkedValue = val;
	}


	void drawAt(LiquidCrystal_I2C& lcd, uint8_t col, uint8_t row)
	{
		lcd.setCursor(col, row);
		// draw value with leading zeros to fill length

		snprintf(buffer, sizeof(buffer), "%0*d", length, *linkedValue);
		lcd.print(buffer);
	}
};


struct Page
{
	virtual ~Page() {}

	virtual void enterPage() {}


	virtual void drawOnce() = 0;

	virtual void drawLoop() = 0;

	virtual void pageUpdate(uint16_t btns) {}

	virtual void exitPage() {}

};