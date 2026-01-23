#pragma once

struct CfgPage : Page
{
	int selField = 0;

	void enterPage() override
	{
		selField = 0;
	}
	void drawOnce() override
	{
		lcd.clear();
		lcd.print("SPNDL ");
		lcd.setCursor(0, 2);
		
		lcd.setCursor(0, 3);
		lcd.print((String)" CFG "+LabelAct("MoV", selField == 1)+LabelAct("SpV", selField == 2)+LabelAct("HdW", selField == 3));
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
		if (btns & 0x0002)
		{
			selField = 1;
			drawOnce();
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

			btnRun.arm(nullptr);

			return;
		}

		if (btns & 0x0010)
		{
			hdwhlCount = 0;
		}
	}
};
CfgPage cfgPage;

