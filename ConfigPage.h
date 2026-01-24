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
		lcd.print("SPNDL ...  ...  ... ");
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
		// handle inputs for main page if needed
		if (btns & 0x0001)
		{
			goToPage(0);
			return;
		}
		if (btns & 0x0002) // MoV
		{
			if (selField != 1)
			{
				selField = 1;

				// set handwheel to configure the motor steps per rev
				hdw0 = motorStepsPerRev - hdwhlCount;
				v0 = motorStepsPerRev;
				drawOnce();
			}
			else
			{
				// commit the new value
				motorStepsPerRev = hdw0 + hdwhlCount;
			}
			return;
		}
		if (btns & 0x0004)
		{
			selField = 2;
			drawOnce();
			return;
		}
		if (btns & 0x0008)
		{
			selField = 3;
			drawOnce();
			return;
		}

		// handle handwheel changes

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

			int newVal = hdw0 + hdwhlCount;
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

