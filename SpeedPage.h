#pragma once



struct SpdPage : Page
{

	double cmdMMS = 0.0;
	int cmdRPM = 0;
	EditableValueDouble evMMScmd = EditableValueDouble(&cmdMMS, "\005/s", 0.1);
	EditableValueInt evRPMcmd = EditableValueInt(&cmdRPM, "RPM", 10);
	PageValueDouble pvMMSCmd = PageValueDouble(4, &cmdMMS);
	PageValueInt pvRPMCmd = PageValueInt(4, evRPMcmd.value);


	int motorPPSSet = 0;
	PageValueInt pvSetSpeed = PageValueInt(4, &motorPPSSet);

	int motorDirection = 1; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, " REVSTOP FWD" );


	// Conversion helpers — carriage moves leadscrewPitchUM (in micrometers) per motor revolution.
	double rpmFromMms(double mms)
	{
		if (leadscrewPitchUM == 0) return 0.0;
		return mms * 60.0 * 1000.0 / (double)leadscrewPitchUM;
	}
	double mmsFromRpm(double rpm)
	{
		return (double)rpm * (double)leadscrewPitchUM / (60.0 * 1000.0);
	}
	float ppsFromMms(double mms)
	{
		if (leadscrewPitchUM == 0) return 0.0f;
		return (float)(mms * 1000.0 * (double)motorStepsPerRev / (double)leadscrewPitchUM);
	}


	EditableValue* getEvAtField(int index) override
	{
		switch (index)
		{
		case 2:
			return &evMMScmd;
		case 3:
			return &evRPMcmd;
		default:
			return nullptr;
		}
	}

	void enterPage() override
	{
		Page::enterPage();

		cmdMMS = 0.0;
		evRPMcmd.setValue(0);

		motorDirection = 1; // stop
	}



	void drawOnce() override
	{
		// thrd page
		lcd.clear();
		// l0
		lcd.print(" DIR ");

		// l1
		lcd.setCursor(C_FIELD3, 0);
		lcd.print(" VSET");
		// 
		// l2
		

		// l3
		evMMScmd.drawCaption(lcd, C_FIELD2, 3);
		evRPMcmd.drawCaption(lcd, C_FIELD3, 3);
	}
	void drawLoop() override
	{
		pvDir.drawAt(lcd, C_FIELD0, 1);

		motorPPSSet = stepperGetCurrentPulseRate();
		pvSetSpeed.drawAt(lcd, C_FIELD3, 0);



		pvMMSCmd.drawAt(lcd, C_FIELD2, 2);
		pvRPMCmd.drawAt(lcd, C_FIELD3, 2);

		pvDRO.drawAt(lcd, C_FIELD0, 3);

	}



	void onRunPressed()
	{
		// start motor at current speed setting
		if (cmdMMS != 0.0)
		{
			motorPPSSet = stepperRunPPS(ppsFromMms(cmdMMS));

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
			int sf = selField; // remember selected field to restore after zeroing
			setEV(-1);

			cmdMMS = 0.0;
			evRPMcmd.setValue(0);

			if (sf > -1)
				setEV(sf);
		}


		if (btns & 0x0010) // DIR btn - flip directions
		{
			if (evEditing && !btnStop.IsArmed()) // not while running.
			{
				evEditing->commitEdit();
				evEditing->negate();
				evEditing->beginEdit(hdwhlCount);
				motorDirection = -1; // force value update below
			}
		}
		else // default behaviour for other buttons
		{
			Page::pageUpdate(btns);
		}

		// keep mm/s and RPM in sync based on which one was just edited
		if (evEditing == &evMMScmd)
		{
			evRPMcmd.setValue((int)round(rpmFromMms(cmdMMS)));
		}
		else if (evEditing == &evRPMcmd)
		{
			evMMScmd.setValue(mmsFromRpm(cmdRPM));
		}



		if (sign(cmdRPM) + 1 != motorDirection)
		{
			motorDirection = sign(cmdRPM) + 1;
		}
		// while running, allow realtime speed changes
		if (btnStop.IsArmed())
		{
			motorPPSSet = stepperRunPPS(ppsFromMms(cmdMMS));
		}
		else
		{
			// while not running, arm/disarm RUN button based whether speed command is non-zero
			bool nonZero = fabs(cmdMMS) >= 0.05;
			if (nonZero != btnRun.IsArmed())
			{
				if (nonZero)
					btnRun.arm();
				else
					btnRun.disarm();
			}

		}

	}
	void exitPage() override
	{

		cmdMMS = 0.0;
		evRPMcmd.setValue(0);
		stepperStop();

		btnRun.disarm();
		btnStop.disarm();

		Page::exitPage();
	}
};
SpdPage spdPage;
