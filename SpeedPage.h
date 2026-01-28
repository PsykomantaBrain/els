#pragma once



struct SpdPage : Page
{

	int cmdPPS = 0;
	int cmdRPS = 0;
	int cmdRPM = 0;
	EditableValueInt evPPScmd = EditableValueInt(&cmdPPS, "PPS", 1, 10);
	EditableValueInt evRPScmd = EditableValueInt(&cmdRPS, "RPS", 2, 1);
	EditableValueInt evRPMcmd = EditableValueInt(&cmdRPM, "RPM", 3, 10);
	PageValueInt pvPPSCmd = PageValueInt(4, evPPScmd.value);
	PageValueInt pvRPSCmd = PageValueInt(4, evRPScmd.value);
	PageValueInt pvRPMCmd = PageValueInt(4, evRPMcmd.value);


	int motorPPSSet = 0;
	PageValueInt pvSetSpeed = PageValueInt(4, &motorPPSSet);

	int motorDirection = 1; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "REV STOP FWD" );


	EditableValueInt* getEvAtField(int index) override
	{
		switch (index)
		{
		case 1:
			return &evPPScmd;
		case 2:
			return &evRPScmd;
		case 3:
			return &evRPMcmd;
		default:
			return nullptr;
		}
	}

	void enterPage() override
	{		
		Page::enterPage();
						
		evPPScmd.setValue(0);
		evRPScmd.setValue(0);
		evRPMcmd.setValue(0);

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
		//lcd.setCursor(0, 3);
		//lcd.print("\003SPD\002 pps  rps  rpm ");

		evPPScmd.drawCaption(lcd, C_FIELD1, 3);
		evRPScmd.drawCaption(lcd, C_FIELD2, 3);
		evRPMcmd.drawCaption(lcd, C_FIELD3, 3);
	}
	void drawLoop() override
	{
		motorPPSSet = stepperGetCurrentPulseRate();
		pvSetSpeed.drawAt(lcd, C_FIELD0, 1);

		pvDir.drawAt(lcd, C_FIELD1, 1);


		pvPPSCmd.drawAt(lcd, C_FIELD1, 2);
		pvRPSCmd.drawAt(lcd, C_FIELD2, 2);
		pvRPMCmd.drawAt(lcd, C_FIELD3, 2);
	}



	void onRunPressed()
	{
		// start motor at current speed setting
		if (evPPScmd.getValue() != 0)
		{
			motorPPSSet = stepperRunPPS((float)*evPPScmd.value);

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

			evPPScmd.setValue(0);
			evRPScmd.setValue(0);
			evRPMcmd.setValue(0);

			if (sf > -1)
				setEV(sf); 
		}


		if (btns & 0x0020) // DIR btn - flip directions
		{
			if (evEditing && !btnStop.IsArmed()) // not while running. 
			{
				evEditing->commitEdit();
				evEditing->setValue(-evEditing->getValue());
				evEditing->beginEdit(hdwhlCount);
				motorDirection = -1; // force value update below
			}			
		}
		else // default behaviour for other buttons
		{
			Page::pageUpdate(btns);
		}

		// update related values (PPS/RPM/RPS)
		if (evEditing == &evPPScmd)
		{
			evRPScmd.setValue(evPPScmd.getValue() / motorStepsPerRev);
			evRPMcmd.setValue((evPPScmd.getValue() * 128 / motorStepsPerRev) * 60 / 128);
		}
		else if (evEditing == &evRPScmd)
		{
			evPPScmd.setValue(evRPScmd.getValue() * motorStepsPerRev);
			evRPMcmd.setValue(evRPScmd.getValue() * 60);
		}
		else if (evEditing == &evRPMcmd)
		{
			evRPScmd.setValue(evRPMcmd.getValue() / 60);
			evPPScmd.setValue((evRPMcmd.getValue() * 128 / 60) * motorStepsPerRev / 128);
		}



		if (sign(evPPScmd.getValue()) + 1 != motorDirection)
		{
			motorDirection = sign(evPPScmd.getValue()) + 1;
		}		
		// while running, allow realtime speed changes
		if (btnStop.IsArmed())
		{
			motorPPSSet = stepperRunPPS((float)evPPScmd.getValue());
		}
		else
		{
			// while not running, arm/disarm RUN button based whether speed command is non-zero
			if ((fabsf(evPPScmd.getValue()) >= 10) != btnRun.IsArmed())
			{
				if (fabsf(evPPScmd.getValue()) >= 10)
					btnRun.arm();
				else
					btnRun.disarm();
			}

		}
		
	}
	void exitPage() override
	{

		evPPScmd.setValue(0);
		evRPScmd.setValue(0);
		evRPMcmd.setValue(0);
		stepperStop();

		btnRun.disarm();
		btnStop.disarm();

		Page::exitPage();
	}
};
SpdPage spdPage;
