#pragma once



struct SpdPage : Page
{

	int motorSpeed = 0;
	int hdWhl0;

	PageValueRegion pvTgtSpeed = PageValueRegion(4, &motorSpeed);

	void enterPage() override
	{
		hdWhl0 = hdwhlCount;
		motorSpeed = 0;

		motor.stop();
		motor.setAcceleration(50);

		// arm run button
		btnRun.arm(); 

	}
	
	void onRunPressed()
	{
		// start motor at current speed setting
		if (motorSpeed != 0)
		{
			motor.setSpeed(motorSpeed);
			btnStop.arm();
		}
	}
	void onStopPressed()
	{
		// stop motor
		motor.stop();
		motorSpeed = 0;
		btnRun.disarm();
	}

	void drawOnce() override
	{
		// thrd page
		lcd.clear();		
		// l0
		lcd.print("SPNDL ...  ...  ... ");
		
		// l1
		pvSpndl.drawAt(lcd, C_FIELD0, 1);
		// l2

		// l3
		lcd.setCursor(0, 3);
		lcd.print("\003SPD\002 PSR  adv  ... ");
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 1);
		pvTgtSpeed.drawAt(lcd, C_FIELD1, 2);
	}

	void pageUpdate(uint16_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x0001)
		{
			goToPage(0);
			return;
		}
		if (btns & 0x0004)
		{
			motorSpeed = 0;
			hdWhl0 = hdwhlCount;
			motor.stop();
		}
		if (btns & 0x0008)
		{
			
			return;			
		}


		
		int spdSetting = hdwhlCount - hdWhl0;
		if (motorSpeed != spdSetting)
		{
			if (spdSetting == 0)
				motor.stop();
			else
				motor.setSpeed(spdSetting);

			motorSpeed = spdSetting;
		}
		if (motorSpeed != 0)
		{
			motor.runSpeed();
		}

	}
	void exitPage() override
	{
		motor.stop();
		motorSpeed = 0;

		btnRun.disarm();
		btnStop.disarm();
	}
};
SpdPage spdPage;
