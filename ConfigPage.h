#pragma once

struct CfgPage : Page
{

	EditableValueInt evMoV = EditableValueInt(&motorStepsPerRev, "MoV", 1, 100);
	EditableValueInt evSpV = EditableValueInt(&spindlePulsesPerRev, "SpV", 2, 100);
	EditableValueInt evLsP = EditableValueInt(&leadscrewPitchUM, "LsP", 3, 100);


	PageValueInt pvMoV = PageValueInt(4, evMoV.value);
	PageValueInt pvSpV = PageValueInt(4, evSpV.value);
	PageValueInt pvLsP = PageValueInt(4, evLsP.value);


	EditableValueInt* getEvAtField(int index) override
	{		
		switch (index)
		{
		case 1:
			return &evMoV;		
		case 2:
			return &evSpV;
		case 3:
			return &evLsP;
		default:
			return nullptr;
		}
	}

	void drawOnce() override
	{
		lcd.clear();
		lcd.print("SPNDL  ...  ...  ... ");


		lcd.setCursor(0, 1);
		lcd.print(" ...  ...  ...  ... ");
		lcd.setCursor(0, 2);
		lcd.print(" ...  ...  ...  ... ");
		lcd.setCursor(0, 3);
		lcd.print(" RTN  mov  spv  lds ");

		evMoV.drawCaption(lcd, C_FIELD1, 3);
		evSpV.drawCaption(lcd, C_FIELD2, 3);
		evLsP.drawCaption(lcd, C_FIELD3, 3);

		//lcd.print((String)" CFG "+LabelAct("MoV", selField == 1)+LabelAct("SpV", selField == 2)+LabelAct("HdW", selField == -1));
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 3);

		pvMoV.drawAt(lcd, C_FIELD1, 2);
		pvSpV.drawAt(lcd, C_FIELD2, 2);
		pvLsP.drawAt(lcd, C_FIELD3, 2);
	}


	void pageUpdate(uint16_t btns) override
	{

		if (btns != 0 && btns < 0x0100) // handle buttons 1-8 first
		{
			switch (btns)
			{
				default:
					setEV(-1);
				break;

				case 0x0001: // SK1 RTN				
					goToPage(0);

				return;

				case 0x0002: // SK2 MoV
					setEV(1);
				break;

				case 0x0004: // SK3 SpV
					setEV(2);
				break;

				case 0x0008: // SK4 Lds
					setEV(3);
				break;

				// case 0x0010: // SK4
				// case 0x0020: // SK5
				// case 0x0040: // SK6
				// case 0x0080: // SK7
			}
			drawOnce();
			return;
		}

		Page::pageUpdate(btns);

	}

};
CfgPage cfgPage;

