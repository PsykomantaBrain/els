#pragma once
#include <Arduino.h>


// LCD layout consts -------------------------------
#define C_FIELD0 0 //1234
#define C_FIELD1 5 //6789
#define C_FIELD2 10//1234
#define C_FIELD3 15//6789


struct EditableValueInt
{
	String caption;
	int vIndex = -1;

	int* value = 0;
	int step = 1;

	int v0, hdwl0;
	
	bool editing;

	EditableValueInt(int* linkedValue, String cap, int vIndex, int step = 1)
	{
		caption = cap;
		this->vIndex = vIndex;

		this->value = linkedValue;
		this->v0 = *linkedValue;

		this->step = step;
	}

	void beginEdit(int hdwl)
	{
		if (editing)
			return;

		hdwl0 = hdwl;
		v0 = *value;
		editing = true;
	}

	void updateEdit(int hdwl)
	{
		if (!editing)
			return;

		*value = v0 + ((hdwl - hdwl0) * step);
	}

	void commitEdit()
	{
		if (!editing)
			return;
		editing = false;
	}
	void cancelEdit()
	{
		if (!editing)
			return;

		*value = v0;
		editing = false;
	}

	void setValue(int v)
	{
		*value = v;
	}
	int getValue()
	{
		return *value;
	}


	void drawCaption(LiquidCrystal_I2C& lcd, uint8_t col, uint8_t row)
	{
		lcd.setCursor(col, row);
		if (editing)
			lcd.print("\003" + caption + "\002");
		else
			lcd.print(" " + caption + " ");
	}
};



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
	int selField = 0;
	int hdw0, v0;
	
	
	virtual ~Page() {}



	virtual void enterPage()
	{
		selField = -1;
		evEditing = nullptr;
	}


	virtual void drawOnce() = 0;

	virtual void drawLoop() = 0;

	virtual void pageUpdate(uint16_t btns) 
	{
		if (evEditing != nullptr)
			evEditing->updateEdit(hdwhlCount);
	}


	virtual void exitPage() 
	{
		setEV(-1);
	}



	virtual EditableValueInt* getEvAtField(int index)
	{
		return nullptr;
	}


	EditableValueInt* evEditing = nullptr;
	void setEV(int sel)
	{
		if (evEditing != nullptr)
		{
			if (selField == sel)
			{
				evEditing->commitEdit();
				sel = -1;
			}
			else
			{
				evEditing->cancelEdit();
			}
		}


		evEditing = getEvAtField(sel);
		selField = sel;

		if (evEditing != nullptr)
		{
			evEditing->beginEdit(hdwhlCount);
		}

		drawOnce();
	}
};

