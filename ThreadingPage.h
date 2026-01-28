#pragma once
#include <CoupledRun.h>

struct ThreadingPage : Page
{
	
	int pitchUm = 1000;	
	EditableValueInt evPitch = EditableValueInt(&pitchUm, "PCH", 5, 25);
	PageValueInt pvPitch = PageValueInt(4, evPitch.value);


	int motorDirection = 2; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "R\\\\\\STOPL///");

	int motorTarget;
	PageValueInt pvMot = PageValueInt(4, &motorTarget);

	CoupledRunF32* coupledRun = nullptr;

	EditableValueInt* getEvAtField(int index) override
	{
		switch (index)
		{

		case 5:
			return &evPitch;
		default:
			return nullptr;
		}
	}


	void drawOnce() override
	{
		// thrd page
		lcd.clear();

		// l0
		lcd.print(" ...  pch  DIR  ... ");
		// lcd.print((String)"     " + LabelAct("PCH", selField == -1) + LabelAct("   ", selField == -1) + LabelAct("   ", selField == -1));		
		evPitch.drawCaption(lcd, C_FIELD1, 0);


		

		
		//lcd.print((String)" THR " + LabelAct("PDT", selField == 1) + LabelAct("END", selField == 2) + LabelAct("RDY", selField == 3));

		lcd.setCursor(0, 3);
		lcd.print(" ...  Spn  ...  TGT ");
	}
	void drawLoop() override
	{
		pvPitch.drawAt(lcd, C_FIELD1, 1);
		pvDir.drawAt(lcd, C_FIELD2, 1);

		pvMot.drawAt(lcd, C_FIELD3, 2);

		pvSpndl.drawAt(lcd, C_FIELD0, 3);

	}

	void enterPage() override
	{
		Page::enterPage();
		
		btnRun.arm();

	}



	void onRunPressed()
	{
		// start coupled run with current settings
		if (evPitch.getValue() != 0)
		{
			// disarm run button
			btnRun.disarm();
			btnStop.arm();

			coupledRun = new CoupledRunF32(hdwhlCount, stepper->getCurrentPosition(), (float)pitchUm / 1000.0f);
		}
	}
	void onStopPressed()
	{		
		delete coupledRun;
		coupledRun = nullptr;

		// stop motor (only disarm if actually stopped)
		if (stepperStop())
		{
			btnStop.disarm();
			btnRun.arm();
		}

	}
	void pageUpdate(uint16_t btns) override
	{
		if (btns & 0x2000) // STOP
		{
			onStopPressed();
			return;
		}

		if (btnStop.IsArmed())
			return;

		if (btns & 0x1000) // RUN
		{
			onRunPressed();
			return;
		}

		
		if (btns & 0x0040) // DIR btn - flip directions
		{
			if (!btnStop.IsArmed()) // not while running. 
			{
				motorDirection = motorDirection == 0 ? 2 : 0; // toggle between REV and FWD
				delay(200); // debounce
			}
		}
		else
		{
			// default behaviour for other buttons
			Page::pageUpdate(btns);
		}

		if (coupledRun)
		{
			int targetMotorPos = coupledRun->getTargetMotorCount(hdwhlCount);
			stepper->moveTo(targetMotorPos);
		}
	}

	void exitPage() override
	{
		stepperStop();

		btnRun.disarm();
		btnStop.disarm();

		Page::exitPage();
	}
};
ThreadingPage threadingPage;

