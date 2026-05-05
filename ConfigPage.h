#pragma once
#include <EEPROM.h> 


void saveConfigToEEPROM()
{
	int bWritten = 0;
	bWritten += EEPROM.writeInt(0, motorStepsPerRev);
	bWritten += EEPROM.writeInt(4, spindlePulsesPerRev);
	bWritten += EEPROM.writeInt(8, leadscrewPitchUM);
	bWritten += EEPROM.writeInt(12, motorMaxAccel);
	bWritten += EEPROM.writeInt(16, backlashCompUM);
	bWritten += EEPROM.writeInt(20, hdwlPulsesPerRevolution);
	EEPROM.commit();


	lcd.setCursor(0, 3);
	lcd.print("Config Saved   w    ");
	lcd.setCursor(15, 3);
	lcd.print(bWritten);

	Serial.println((String)"Config Saved, bytes written: " + bWritten);
	delay(1000);
}
void loadConfigFromEEPROM()
{
	motorStepsPerRev = EEPROM.readInt(0);
	spindlePulsesPerRev = EEPROM.readInt(4);
	leadscrewPitchUM = EEPROM.readInt(8);
	motorMaxAccel = EEPROM.readInt(12);
	backlashCompUM = EEPROM.readInt(16);
	hdwlPulsesPerRevolution = EEPROM.readInt(20);

	Serial.println((String)"Config Loaded from EEPROM");
}

struct CfgPage : Page
{
	EditableValueInt evMoV = EditableValueInt(&motorStepsPerRev, "MoV", 5);
	EditableValueInt evSpV = EditableValueInt(&spindlePulsesPerRev, "SpV", 5);
	EditableValueInt evLsP = EditableValueInt(&leadscrewPitchUM, "LsP", 1);
	EditableValueInt evAcc = EditableValueInt(&motorMaxAccel, "Acc", 100);
	EditableValueInt evBla = EditableValueInt(&backlashCompUM, "BLS", 1);
	EditableValueInt evHwV = EditableValueInt(&hdwlPulsesPerRevolution, "HwV", 1);



	PageValueInt pvMoV = PageValueInt(4, evMoV.value);
	PageValueInt pvSpV = PageValueInt(4, evSpV.value);
	PageValueInt pvLsP = PageValueInt(4, evLsP.value);
	PageValueInt pvAcc = PageValueInt(4, evAcc.value);
	PageValueInt pvBla = PageValueInt(4, evBla.value);
	PageValueInt pvHwV = PageValueInt(4, evHwV.value);


	EditableValue* getEvAtField(int index) override
	{
		switch (index)
		{
		case 1:
			return &evMoV;
		case 2:
			return &evSpV;
		case 3:
			return &evLsP;
		case 5:
			return &evHwV;
		case 6:
			return &evBla;
		default:
			return nullptr;
		}
	}

	void drawOnce() override
	{
		lcd.clear();
		lcd.setCursor(C_FIELD0, 0);
		lcd.print("STR");

		//evAcc.drawCaption(lcd, C_FIELD1, 0);
		evHwV.drawCaption(lcd, C_FIELD1, 0);
		evBla.drawCaption(lcd, C_FIELD2, 0);

		evMoV.drawCaption(lcd, C_FIELD1, 3);
		evSpV.drawCaption(lcd, C_FIELD2, 3);
		evLsP.drawCaption(lcd, C_FIELD3, 3);
	}
	void drawLoop() override
	{
		//pvAcc.drawAt(lcd, C_FIELD1, 1);
		pvHwV.drawAt(lcd, C_FIELD1, 1);
		pvBla.drawAt(lcd, C_FIELD2, 1);


		pvMoV.drawAt(lcd, C_FIELD1, 2);
		pvSpV.drawAt(lcd, C_FIELD2, 2);
		pvLsP.drawAt(lcd, C_FIELD3, 2);


		pvDRO.drawAt(lcd, C_FIELD0, 3);
	}


	void pageUpdate(uint16_t btns) override
	{
		if (btns & 0x0010) // SK4 - save settings to eeprom
		{
			saveConfigToEEPROM();
			drawOnce();
			return;
		}


		Page::pageUpdate(btns);

	}



};
CfgPage cfgPage;


