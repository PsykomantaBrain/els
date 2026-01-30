#pragma once
#include "CoupledRun.h"

struct ThreadingPage : Page
{
	
	int pitchUm = 1000;	
	EditableValueInt evPitch = EditableValueInt(&pitchUm, "PCH", 5, 25);
	PageValueInt pvPitch = PageValueInt(4, evPitch.value);

	int cplSpeed = 10000;
	EditableValueInt evCplspd = EditableValueInt(&cplSpeed, "SPD", 7, 250);
	PageValueInt pvCplspd = PageValueInt(4, evCplspd.value);

	int motorDirection = 2; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "L///STOPR\\\\\\");

	int motorTarget = 0;
	PageValueInt pvMot = PageValueInt(4, &motorTarget);

	CoupledRunF32 coupledRun;

	EditableValueInt* getEvAtField(int index) override
	{
		switch (index)
		{

		case 5:
			return &evPitch;
		case 7:
			return &evCplspd;
			
		default:
			return nullptr;
		}
	}


	void drawOnce() override
	{
		// thrd page
		lcd.clear();

		// l0
		lcd.print(" ...  pch  DIR  SPD ");
		
		evPitch.drawCaption(lcd, C_FIELD1, 0);
		// DIR btn on field 2
		evCplspd.drawCaption(lcd, C_FIELD3, 0);

		

		
		lcd.setCursor(0, 3);
		lcd.print(" ...  Spn  ...  TGT ");
	}
	void drawLoop() override
	{
		pvPitch.drawAt(lcd, C_FIELD1, 1);
		pvDir.drawAt(lcd, C_FIELD2, 1);
		pvCplspd.drawAt(lcd, C_FIELD3, 1);

		pvMot.drawAt(lcd, C_FIELD3, 2);

		pvSpndl.drawAt(lcd, C_FIELD0, 3);

	}

	void enterPage() override
	{
		Page::enterPage();
		
		btnRun.arm();
		motorTarget = stepper->getCurrentPosition();
	}



	void onRunPressed()
	{
		// start coupled run with current settings
		if (evPitch.getValue() != 0)
		{
			// disarm run button
			btnRun.disarm();
			btnStop.arm();

			coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)pitchUm);			
			
			
		}
	}
	void onStopPressed()
	{		
		coupledRun.endRun();


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
				stepperDirection(motorDirection > 0); // set direction 

				delay(200); // debounce
			}
		}
		else
		{
			// default behaviour for other buttons
			Page::pageUpdate(btns);
		}

		
		
		if (coupledRun.isRunning())
		{
			if ((float)pitchUm != coupledRun.K)
			{
				// restart with new pitch (keeping current position as m0, so pitch change affects speed only, instead of causing the motor to jump)
				coupledRun.endRun();
				coupledRun.K = (float)pitchUm;
				coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)pitchUm);
			}


			motorTarget = coupledRun.getTargetMotorCount(spndlCount);
			stepperMoveToTgt(motorTarget, cplSpeed);
		}
	}

	void exitPage() override
	{
		coupledRun.endRun();
		stepperStop();

		btnRun.disarm();
		btnStop.disarm();

		Page::exitPage();
	}
};
ThreadingPage threadingPage;

