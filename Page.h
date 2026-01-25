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
		
		snprintf(buffer, length+1, "%s", linkedValue->c_str());
		lcd.print(buffer);
	}
};


struct PageValueEnum
{
private:
	uint8_t len;
	char buffer[16];

public:
	volatile int* linkedValue = nullptr;
	char* enumLabels;

	PageValueEnum(uint8_t labelLen, volatile int* val, char* labels)
	{
		linkedValue = val;
		enumLabels = labels;
		len = labelLen;

	}

	void drawAt(LiquidCrystal_I2C& lcd, uint8_t col, uint8_t row)
	{
		lcd.setCursor(col, row);
		
		// print len chars from enumLabels at index linkedValue*len
		
		snprintf(buffer, len+1, "%s", &enumLabels[(*linkedValue) * len]);

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


struct EditableValueInt
{

	volatile int* linkedValue = nullptr;
	int step = 1;

	int v0, hdwl0;
	int vDisplay;

	bool editing;

	EditableValueInt(volatile int* val, int s)
	{
		linkedValue = val;
		step = s;

		v0 = *linkedValue;
		vDisplay = v0;
	}

	void beginEdit(int hdwl)
	{
		hdwl0 = hdwl;
		v0 = *linkedValue;
		vDisplay = v0;
		editing = true;
	}
	
	void updateEdit(int hdwl)
	{
		vDisplay = v0 + ((hdwl - hdwl0)*step);
	}

	void commitEdit()
	{
		*linkedValue = vDisplay;
		editing = false;
	}
	void cancelEdit()
	{
		*linkedValue = v0;
		editing = false;
	}


};