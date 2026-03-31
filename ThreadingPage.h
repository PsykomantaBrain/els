#pragma once
#include "CoupledRun.h"
#include "PCNT_spindle.h"



struct ThreadingPage : Page
{
	
	int pitchUm = 1000;	
	EditableValueInt evPitch = EditableValueInt(&pitchUm, "PCH", 25);
	PageValueInt pvPitch = PageValueInt(4, evPitch.value);


	int cplAccel = 100000;
	EditableValueInt evCplacc = EditableValueInt(&cplAccel, "ACC", 250);
	PageValueInt pvCplacc = PageValueInt(4, evCplacc.value);

	int cplSpeed = 10000;
	EditableValueInt evCplspd = EditableValueInt(&cplSpeed, "SPD", 250);
	PageValueInt pvCplspd = PageValueInt(4, evCplspd.value);

	int motorDirection = 2; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "L///STOPR<<<");


	// location (motor position) of endstop (to automatically disengage the run)
	// this is relative to the 0 position where the run started (there's no closed loop position input here... yet)
	// so it's more of a 'stop-after-this-much-travel' limit
	int endStop = 0;
	EditableValueInt evEndstop = EditableValueInt(&endStop, "END", 10);
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

	int runVel = 0; // for display only, calculated in the run task based on the spindle speed and pitch
	PageValueInt pvVel = PageValueInt(4, &runVel);


	CoupledRunF32 coupledRun;


	int runTriggerState = 0; // 'thread dial indicator' state. 0 = idle. 1 = waiting for spindle to move arming zone, 2 = waiting for threshold crossing to start. 3 = running
	// negative states are the same but for running in reverse.
	bool waitForRunStart()
	{
		// wait for spindle to reach the next index position (or a multiple of it, based on runQ0) before starting the run, to ensure consistent thread start points

		// we don't actually have to store a new run start position each time. We can just wait for the spindle to reach a fixed index point for all runs.
		// the only caveat is that the logic is different based on whether we're running in forward or reverse.

		// to avoid false starts, this needs to be a two-state check. First we check if we are in the right side of the triggerable range (eg s < halfrev) 
		// then we wait for the spindle to move into the other half (eg s >= halfrev) before we trigger the start. This way we avoid triggering if we start the run while we're already in the trigger zone.

		if (runTriggerState == 0)
			return false;

		int spndl = read_spindle() % spindlePulsesPerRev;
		switch (runTriggerState)
		{
		case 1:
			if (spndl < spindlePulsesPerRev / 2)
				runTriggerState = 2;
			break;
		case 2:
			if (spndl >= spindlePulsesPerRev / 2)
			{
				runTriggerState = 3;
				digitalWrite(LEDRUN, 0);
				return true;
			}
			break;
		case -1:
			if (spndl >= spindlePulsesPerRev / 2)
				runTriggerState = -2;
			break;
		case -2:
			if (spndl < spindlePulsesPerRev / 2)
			{
				runTriggerState = -3;
				digitalWrite(LEDRUN, 0);
				return true;
			}
			break;
		}

		// for states |1| and |2|, blink the run LED to indicate waiting for the trigger 
		if (runTriggerState == 1 || runTriggerState == -1)
		{
			digitalWrite(LEDRUN, (millis() / 250) % 2); // blink at 2Hz
		}
		if (runTriggerState == 2 || runTriggerState == -2)
		{
			digitalWrite(LEDRUN, (millis() / 125) % 2); // blink at 4Hz
		}

		return false;
	}



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
		lcd.print(" ...  Spn  Vel  TGT ");
	}
	void drawLoop() override
	{

		pvCplacc.drawAt(lcd, C_FIELD0, 1);
		pvPitch.drawAt(lcd, C_FIELD1, 1);
		pvDir.drawAt(lcd, C_FIELD2, 1);
		pvCplspd.drawAt(lcd, C_FIELD3, 1);


		pvDRO.drawAt(lcd, C_FIELD0, 3);
		pvSpndl.drawAt(lcd, C_FIELD1, 2);
		pvVel.drawAt(lcd, C_FIELD2, 2);
		pvMot.drawAt(lcd, C_FIELD3, 2);


	}

	void enterPage() override
	{
		Page::enterPage();
		
		btnRun.arm();
		motorTarget = stepper->getCurrentPosition();
		runVel = 0;
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
			startRunTask();
		}
	}
	void onStopPressed()
	{		
		coupledRun.endRun();
		runTriggerState = 0;

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


		}
	}

	void exitPage() override
	{
		runTriggerState = 0;

		coupledRun.endRun();
		stepperStop();

		btnRun.disarm();
		btnStop.disarm();

		Page::exitPage();
	}


	float vel, acc;
	void startRunTask()
	{
		vel = 0.0f;
		acc = 0.0f;

		runTriggerState = motorDirection == 0 ? -1 : 1; // set initial trigger state based on direction (waiting for spindle to move into the opposite half of the index pulse)

		// start the run task pinned to core 1
		xTaskCreatePinnedToCore(
			[](void* param) 
			{
				ThreadingPage* page = (ThreadingPage*)param;
				const TickType_t xDelay = pdMS_TO_TICKS(10);

				Serial.println("Coupled run task started... waiting for spindle indexing");
				while (!page->waitForRunStart())
				{
					if (page->runTriggerState == 0) // if the run was stopped while waiting for the trigger, exit the task
					{
						Serial.println("Coupled run cancelled.");
						digitalWrite(LEDRUN, 0); // stop blinking

						vTaskDelete(NULL);
						return;
					}

					vTaskDelay(xDelay);
				}

				digitalWrite(LEDRUN, 0); // stop blinking

				page->coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)page->pitchUm);
				Serial.println("Running.");

				while (page->coupledRun.isRunning())
				{					
					page->coupledRunTask();
					vTaskDelay(xDelay);
				}
				vTaskDelete(NULL);
			},
			"CoupledRunTask",
			2048,
			(void*)this,
			1,
			NULL,
			1); // core 1

	}

	void coupledRunTask()
	{
		int spndl = read_spindle(); // up to date spindle value

		motorTarget = coupledRun.getTargetMotorCount(spndl);
		
		// measuring spindle velocity for the commanded move speed makes a HUGE difference in smoothness of the motion vs going to the target at a fixed speed.
		// I tried deriving acceleration as well, but that made the motor lag behind quite a bit. With acc set high enough, it moves smoothly enough and there's no perceivable latency.
		vel = coupledRun.updStepperSpeed(spndl, micros());
		runVel = (int)vel; // update display value
		

		stepperMoveToTgt(motorTarget, vel, cplAccel);
	}

};
ThreadingPage threadingPage;
