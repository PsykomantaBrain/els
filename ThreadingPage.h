#pragma once

struct ThreadingPage : Page
{
	int selField = 0;


	void drawOnce() override
	{
		// thrd page
		lcd.clear();
		lcd.print((String)"     " + LabelAct("PCH", selField == 1) + LabelAct("   ", selField == -1) + LabelAct("   ", selField == -1));		
		lcd.setCursor(0, 1);
		lcd.print("  M3  042  ...  ... ");
		lcd.setCursor(0, 2);

		lcd.print(" ...  ...  ...  ... ");
		lcd.setCursor(0, 3);
		lcd.print((String)" THR " + LabelAct("PDT", selField == 1) + LabelAct("END", selField == 2) + LabelAct("RDY", selField == 3));

		//lcd.print("\003THR\002 PDT  END  JOG ");
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 3);
	}
	void pageUpdate(uint16_t btns) override
	{
		if (btns & 0x0001) // RTN
		{
			goToPage(0);
			return;
		}
		if (btns & 0x0002) // PCH
		{
			selField = 1;
			drawOnce();
			return;
		}
		if (btns & 0x0004) // END
		{
			selField = 2;
			drawOnce();
			return;
		}
		if (btns & 0x0008) // RDY
		{
			selField = 3;
			drawOnce();

			
			return;
		}


	}
};
ThreadingPage threadingPage;

