#pragma once

struct CfgPage : Page
{
	int selField = 0;
	int hdw0, v0;

	
	void enterPage() override
	{
		selField = 0;
	}
	void drawOnce() override
	{
		lcd.clear();
		lcd.print((String)"SPNDL "+ LabelAct("LSP", selField == 5) +" ...  ... ");
		lcd.setCursor(0, 1);
		lcd.print(" ...  ...  ...  ... ");
		lcd.setCursor(0, 2);
		lcd.print(" ...  ...  ...  ... ");
		lcd.setCursor(0, 3);
		lcd.print((String)" CFG "+LabelAct("MoV", selField == 1)+LabelAct("SpV", selField == 2)+LabelAct("HdW", selField == -1));
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 3);
		pvMoV.drawAt(lcd, C_FIELD1, 2);
		pvHdWhl.drawAt(lcd, C_FIELD3, 2);
	}
	void pageUpdate(uint16_t btns) override
	{
		if (btns != 0 && btns < 0x0100) // handle buttons 1-8 first
		{
			switch (btns)
			{
				default:
					if (selField != -1)
					{
						selField = -1;
						drawOnce();
						return;
					}
				break;

			case 0x0001: // SK1 RTN
				goToPage(0);
				return;

			case 0x0002: // SK2 MoV
				if (selField != 1)
				{
					selField = 1;
					// set handwheel to configure the motor steps per rev
					hdw0 = motorStepsPerRev - hdwhlCount;
					v0 = motorStepsPerRev;
				}
				else
				{
					selField = -1;
				}
				break;

			case 0x0004: // SK3 HdW
				selField = 2;				
				break;

			case 0x0008: // SK4 SpV
				selField = 3;
				break;

			// case 0x0010: // SK4

			case 0x0020: // SK5 SpV
				selField = 5;
				break;
			}
			// case 0x0040: // SK6
			// case 0x0080: // SK7

			drawOnce();
			return;
		}



		if (selField == 1)
		{
			if (btns & 0x0100) // ZERO
			{
				hdw0 = hdwhlCount;
				motorStepsPerRev = v0;
				selField = 0;

				drawOnce();
				drawLoop();
				delay(500);
				return;
			}

			int newVal = (hdw0 + hdwhlCount) * 10; // 10-step increments (could be 100 for this)
			if (newVal != motorStepsPerRev)
			{
				motorStepsPerRev = newVal;				
			}
		}
	}
	void exitPage() override
	{		
	}
};
CfgPage cfgPage;

