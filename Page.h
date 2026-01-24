#pragma once
#include <Arduino.h>


// LCD layout consts -------------------------------
#define C_FIELD0 0 //1234
#define C_FIELD1 5 //6789
#define C_FIELD2 10//1234
#define C_FIELD3 15//6789



struct PageValueInt
{

private:
	char buffer[16];

public:
	uint8_t length;

	volatile int* linkedValue = nullptr;

	PageValueInt(uint8_t l, volatile int* val)
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

struct PageValueStr
{

private:
	char buffer[16];

public:
	uint8_t length;
	String* linkedValue = nullptr;

	PageValueStr(uint8_t l, String* val)
	{
		length = l;
		linkedValue = val;
	}


	void drawAt(LiquidCrystal_I2C& lcd, uint8_t col, uint8_t row)
	{
		lcd.setCursor(col, row);
		
		snprintf(buffer, sizeof(buffer), "%-*s", length, linkedValue->c_str());
		lcd.print(buffer);
	}
};


struct PageValueEnum
{
private:
	char buffer[16];
public:
	uint8_t length;
	volatile int* linkedValue = nullptr;
	const String* enumLabels = nullptr;

	PageValueEnum(uint8_t l, volatile int* val, const String* labels)
	{
		length = l;
		linkedValue = val;
		enumLabels = labels;
	}

	void drawAt(LiquidCrystal_I2C& lcd, uint8_t col, uint8_t row)
	{
		lcd.setCursor(col, row);
		// draw value with leading zeros to fill length
		const String label = enumLabels[*linkedValue];
		snprintf(buffer, sizeof(buffer), "%-*s", length, label);
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