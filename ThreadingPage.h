#pragma once
#include "CoupledRun.h"

struct ThreadingPage : Page
{
	
	int pitchUm = 1000;	
	EditableValueInt evPitch = EditableValueInt(&pitchUm, "PCH", 5, 25);
	PageValueInt pvPitch = PageValueInt(4, evPitch.value);


	int cplAccel = 10000;
	EditableValueInt evCplacc = EditableValueInt(&cplAccel, "ACC", 7, 250);
	PageValueInt pvCplacc = PageValueInt(4, evCplacc.value);

	int cplSpeed = 10000;
	EditableValueInt evCplspd = EditableValueInt(&cplSpeed, "SPD", 7, 250);
	PageValueInt pvCplspd = PageValueInt(4, evCplspd.value);

	int motorDirection = 2; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "L///STOPR<<<");


	// location (motor position) of endstop (to automatically disengage the run)
	// this is relative to the 0 position where the run started (there's no closed loop position input here... yet)
	// so it's more of a 'stop-after-this-much-travel' limit
	int endStop = 0;
	EditableValueInt evEndstop = EditableValueInt(&endStop, "END", 7, 10);
	PageValueInt pvEndstop = PageValueInt(4, evEndstop.value);

	// later, a 'return to zero' function can be added here to jog back to the stored zero point automatically
	
	// note that both EndStop and Home positions (which require disengaging) add the need for a synced cut-in feature.
	// if we disengage the run, it's the same as disengaging the halfnut. We lose our position along the thread.
	// we can add a synced cut-in that waits for the spindle to reach the next index pulse (or multiple of its pulseCount) before re-engaging the run.

	// alternatively, the lathe spindle does have a magnet on it for the builtin tach. We can add a hall sensor to read that and get an index event with an interrupt on the rising edge of that.
	// assuming we even have an input pin remaining (we don't, unless we want to mess around with D15)

	// Without an interrupt, what we can do is begin the run with a starting spindle position ahead of the current one.
	// copilot: eg, if we're at spindle encoder count 1234 when we start the run, we can set s0 to 1234 + spindlePulsesPerRev - (1234 % spindlePulsesPerRev)
	// We then set up the run logic to stay at motor zero until the current spindle pos moves past s0 (given the set direction)
	// It's a bit annoying because it's a bunch of stateful logic to add, but it would work.

	int motorTarget = 0;
	PageValueInt pvMot = PageValueInt(4, &motorTarget);

	CoupledRunF32 coupledRun;

	EditableValueInt* getEvAtField(int index) override
	{
		switch (index)
		{
		case 4:
			return &evCplacc;
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
		lcd.print(" ACC  pch  DIR  SPD ");
		
		evCplacc.drawCaption(lcd, C_FIELD0, 0);
		evPitch.drawCaption(lcd, C_FIELD1, 0);

		// DIR btn on field 2
		evCplspd.drawCaption(lcd, C_FIELD3, 0);

		

		
		lcd.setCursor(0, 3);
		lcd.print(" ...  Spn  ...  TGT ");
	}
	void drawLoop() override
	{

		pvCplacc.drawAt(lcd, C_FIELD0, 1);
		pvPitch.drawAt(lcd, C_FIELD1, 1);
		pvDir.drawAt(lcd, C_FIELD2, 1);
		pvCplspd.drawAt(lcd, C_FIELD3, 1);


		pvSpndl.drawAt(lcd, C_FIELD1, 2);
		pvMot.drawAt(lcd, C_FIELD3, 2);

		pvMpos.drawAt(lcd, C_FIELD0, 3);

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

			stepper->setSpeedInHz(cplSpeed);
			stepper->setAcceleration(cplAccel);
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
			if (coupledRun.K != (float)pitchUm / (float)leadscrewPitchUM)
			{
				// restart with new pitch (keeping current position as m0, so pitch change affects speed only, instead of causing the motor to jump)				
				stepper->setSpeedInHz(cplSpeed);
				stepper->setAcceleration(cplAccel);
				coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)pitchUm);
			}


			motorTarget = coupledRun.getTargetMotorCount(spndlCount);
			stepperMoveToTgt(motorTarget, cplSpeed, cplAccel);
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

