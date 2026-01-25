#pragma once



struct SpdPage : Page
{

	int motorPPSCmd = 0;
	int motorPPSSet = 0;
	int motorDirection = 1; // 0=REV, 1 = STP, 2=REV
	int hdWhl0;

	PageValueInt pvCmdSpeed = PageValueInt(4, &motorPPSCmd);
	PageValueInt pvSetSpeed = PageValueInt(4, &motorPPSSet);

	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "REV STOP FWD" );

	void enterPage() override
	{
		hdWhl0 = hdwhlCount;

		motorPPSCmd = 0;		
		motorDirection = 1; // stop


	}
	


	void drawOnce() override
	{
		// thrd page
		lcd.clear();		
		// l0
		lcd.print("VSET DIR  ...  ... ");
		
		// l1
		pvSpndl.drawAt(lcd, C_FIELD0, 1);
		// l2

		// l3
		lcd.setCursor(0, 3);
		lcd.print("\003SPD\002 PPS  rps  rpm ");
	}
	void drawLoop() override
	{
		motorPPSSet = stepperGetCurrentPulseRate();

		pvSetSpeed.drawAt(lcd, C_FIELD0, 1);
		pvDir.drawAt(lcd, C_FIELD1, 1);


		pvCmdSpeed.drawAt(lcd, C_FIELD1, 2);
	}



	void onRunPressed()
	{
		// start motor at current speed setting
		if (motorPPSCmd != 0)
		{
			motorPPSSet = stepperRunPPS((float)motorPPSCmd);

			// disarm run button
			btnRun.disarm();
			btnStop.arm();
		}
	}
	void onStopPressed()
	{		
		// stop motor (only disarm if actually stopped)
		if (stepperStop())
			btnStop.disarm();

	}

	void pageUpdate(uint16_t btns) override
	{		
		if ((btns & 0x0001) && !btnStop.IsArmed())
		{
			goToPage(0);
			return;
		}
		if (btns & 0x1000) // RUN
		{
			onRunPressed();
			return;
		}
		if (btns & 0x2000) // STOP
		{
			onStopPressed();
			return;
		}

		if (btns & 0x0100) // ZERO
		{
			hdWhl0 = hdwhlCount;
			motorPPSCmd = 0;
		}
				
		// update speed command from handwheel
		motorPPSCmd = (hdwhlCount - hdWhl0) * 10;		



		if (sign(motorPPSCmd) != (sign(motorDirection) + 1))
		{
			motorDirection = sign(motorPPSCmd) + 1;
		}		
		// while running, allow realtime speed changes
		if (btnStop.IsArmed())
		{
			motorPPSSet = stepperRunPPS((float)motorPPSCmd);
		}
		else
		{
			// while not running, arm/disarm RUN button based whether speed command is non-zero
			if ((fabsf(motorPPSCmd) >= 10) != btnRun.IsArmed())
			{
				if (fabsf(motorPPSCmd) >= 10)
					btnRun.arm();
				else
					btnRun.disarm();
			}

		}
		
	}
	void exitPage() override
	{
		stepperStop();

		btnRun.disarm();
		btnStop.disarm();
	}
};
SpdPage spdPage;
