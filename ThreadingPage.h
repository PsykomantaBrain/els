#pragma once
#include "CoupledRun.h"
#include "PCNT_spindle.h"



struct ThreadingPage : Page
{
	
	int pitchUm = 1000;	
	EditableValueInt evPitch = EditableValueInt(&pitchUm, "PCH", 25);
	PageValueInt pvPitch = PageValueInt(4, evPitch.value);


	int cplAccel = 420000;
	EditableValueInt evCplacc = EditableValueInt(&cplAccel, "ACC", 250);
	PageValueInt pvCplacc = PageValueInt(4, evCplacc.value);

	int cplSpeed = 42000;
	EditableValueInt evCplspd = EditableValueInt(&cplSpeed, "SPD", 250);
	PageValueInt pvCplspd = PageValueInt(4, evCplspd.value);

	int motorDirection = 2; // 0=REV, 1 = STP, 2=FWD
	PageValueEnum pvDir = PageValueEnum(4, &motorDirection, "L\006\006\006STOPR\007\007\007");


	// Endstop: maximum carriage travel from the run-start position, in micrometers.
	// When the threading run reaches this distance, it stops automatically and the
	// ReturnPage modal takes over for backlash-compensated return-to-zero.
	int endStop = 0;                    // microns; positive distance from m0
	double endStopMM = 0.0;             // mm, derived for display
	EditableValueInt evEndstop = EditableValueInt(&endStop, "END", 100); // step 100 �m = 0.1 mm
	PageValueDouble pvEndstop = PageValueDouble(4, &endStopMM);

	// flag set by the run task (core 1) when the endstop is reached;
	// pageUpdate (core 0) consumes it and switches to ReturnPage.
	volatile bool endstopReached = false;

	// later, a 'return to zero' function can be added here to jog back to the stored zero point automatically	

	int motorTarget = 0;
	PageValueInt pvMot = PageValueInt(4, &motorTarget);

	int runVel = 0; // for display only, calculated in the run task based on the spindle speed and pitch
	PageValueInt pvVel = PageValueInt(4, &runVel);


	CoupledRunF32 coupledRun;


	void enterPage() override
	{
		Page::enterPage();

		btnRun.arm();
		motorTarget = stepper->getCurrentPosition();
		runVel = 0;
		endstopReached = false;

		miniJog.layout(C_FIELD2, 2, 3);
	}



	bool StopMiniJogIfEnabled()
	{
		if (miniJog.Mode != 0 || miniJog.coupledRun.isRunning())
		{
			miniJog.setMode(0);
			return true;
		}
		return false;
	}

	EditableValue* getEvAtField(int index) override
	{
		if (index != 2)
		{
			// if we're trying to edit any field other than pitch, make sure minijog is off so we don't have conflicting controls
			if (StopMiniJogIfEnabled())
			{				
				btnRun.arm(); // rearm if minijog was controlling the run
			}
		}

		switch (index)
		{
		case 2:
			if (!coupledRun.running && spndl_index_task_handle == nullptr)
			{
				miniJog.cycleMode();		

				if (miniJog.Mode == 0)
					btnRun.arm(); // rearm if minijog cycles back to off.
			}
			return nullptr;
		case 3:
			return &evEndstop;
		case 4:
			return &evPitch;
		case 5:
			//is DIR button. No editable value, but toggle direction on press (while not running)

			if (!btnStop.IsArmed()) // not while running.
			{
				motorDirection = motorDirection == 0 ? 2 : 0; // toggle between REV and FWD
				delay(200); // debounce
			}

			return nullptr;
		case 6:
			return &evCplspd;
		case 7:
			return &evCplacc;

		default:
			return nullptr;
		}
	}


	void drawOnce() override
	{
		// thrd page
		lcd.clear();

		// l0
		lcd.print(" PCH  DIR  SPD  ACC ");

		evPitch.drawCaption(lcd, C_FIELD0, 0);
		// DIR btn on 6
		evCplspd.drawCaption(lcd, C_FIELD2, 0);
		evCplacc.drawCaption(lcd, C_FIELD3, 0);


		lcd.setCursor(C_FIELD0, 2);	lcd.print("\010\010");

		lcd.setCursor(0, 3);

		// END field on field 3 (SK3 selects it)
		lcd.print("                end ");
		evEndstop.drawCaption(lcd, C_FIELD3, 3);


		miniJog.drawonce();

	}
	void drawLoop() override
	{

		pvPitch.drawAt(lcd, C_FIELD0, 1);
		pvDir.drawAt(lcd, C_FIELD1, 1);
		pvCplspd.drawAt(lcd, C_FIELD2, 1);
		pvCplacc.drawAt(lcd, C_FIELD3, 1);


		// keep endStopMM in sync with endStop microns for the display
		endStopMM = (double)endStop / 1000.0;
		pvEndstop.drawAt(lcd, C_FIELD3, 2);


		pvDRO.drawAt(lcd, C_FIELD0, 3);

		//pvSpndl.drawAt(lcd, C_FIELD1, 2);
		//pvVel.drawAt(lcd, C_FIELD2, 2);
		//pvMot.drawAt(lcd, C_FIELD3, 2);

		miniJog.drawloop();
	}



	void onRunPressed()
	{
		if (StopMiniJogIfEnabled())
		{
			btnRun.arm(); // rearm if minijog was controlling the run, but don't start a new run below
			miniJog.drawonce();
			return;
		}

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
		if (miniJog.coupledRun.running)
			miniJog.setMode(0);


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

		if (btns & 0x0100) // ZERO
		{
			if (evEditing == &evEndstop)
			{
				evEditing->commitEdit();
				evEditing->zeroValue();
				evEditing->beginEdit(hdwhlCount);
			}
		}

		// endstop reached during a coupled run — hand off to ReturnPage
		if (endstopReached)
		{
			endstopReached = false;
			stepperStop();
			btnStop.disarm();
			btnRun.disarm();

			// snapshot the run-start motor position so ReturnPage can return there with backlash comp
			returnPage.m0 = coupledRun.m0;
			returnPage.endPos = stepper->getCurrentPosition();
			
			goToPage(PAGE_RETURN);
			return;
		}


		// default behaviour for other buttons
		Page::pageUpdate(btns);



		if (coupledRun.isRunning())
		{
			if (coupledRun.K != (float)pitchUm / (float)leadscrewPitchUM)
			{
				// restart with new pitch (keeping current position as m0, so pitch change affects speed only, instead of causing the motor to jump)
				stepper->setSpeedInHz(cplSpeed);
				stepper->setAcceleration(cplAccel);
				coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)pitchUm, spindlePulsesPerRev);
			}


		}
	}

	void exitPage() override
	{
		StopMiniJogIfEnabled();

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

		// start the run task pinned to core 1
		xTaskCreatePinnedToCore(
			[](void* param)
			{
				ThreadingPage* page = (ThreadingPage*)param;

				Serial.println("Coupled run task started... waiting for spindle index pulse");

				// Register this task so the PCNT overflow ISR (= index pulse) wakes it.
				// Must be set before the first ulTaskNotifyTake call.
				spndl_index_task_handle = xTaskGetCurrentTaskHandle();

				bool triggered = false;
				while (!triggered)
				{
					if (!btnStop.IsArmed())
					{
						spndl_index_task_handle = nullptr;
						Serial.println("Coupled run cancelled.");
						//digitalWrite(LEDRUN, 0);
						vTaskDelete(NULL);
						return;
					}

					digitalWrite(LEDRUN, (millis() / 250) % 2); // 2 Hz blink while waiting
					page->StopMiniJogIfEnabled(); // make sure minijog is off while this is going

					if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)) > 0)
					{						
						// Stop may have raced with the index pulse
						if (!btnStop.IsArmed())
						{
							spndl_index_task_handle = nullptr;
							digitalWrite(LEDRUN, 0);
							vTaskDelete(NULL);
							return;
						}
						triggered = true;
					}
				}


				spndl_index_task_handle = nullptr;
				digitalWrite(LEDRUN, 0);

				page->coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)page->pitchUm, spindlePulsesPerRev);
				Serial.println("Running.");

				const TickType_t xDelay = pdMS_TO_TICKS(10);
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

		// endstop check: stop the run when the carriage has actually travelled |endStop|
		// microns from m0. Use the stepper's real position (not motorTarget) — motorTarget
		// is the projected position from the spindle and runs ahead of the motor by however
		// much the motor lags under acceleration limits, which would make the run stop far
		// short of the configured distance.
		if (endStop > 0 && leadscrewPitchUM > 0)
		{
			int actualSteps = abs(stepper->getCurrentPosition() - coupledRun.m0);
			int endStopSteps = (int)((double)endStop * (double)motorStepsPerRev / (double)leadscrewPitchUM);
			if (actualSteps >= endStopSteps)
			{
				endstopReached = true;
				coupledRun.endRun();   // breaks the run-task loop
				return;                // pageUpdate (core 0) handles the rest
			}
		}

		// measuring spindle velocity for the commanded move speed makes a HUGE difference in smoothness of the motion vs going to the target at a fixed speed.
		// I tried deriving acceleration as well, but that made the motor lag behind quite a bit. With acc set high enough, it moves smoothly enough and there's no perceivable latency.
		vel = coupledRun.updStepperSpeed(spndl, micros());
		runVel = (int)vel; // update display value


		stepperMoveToTgt(motorTarget, vel, cplAccel);
	}

};
ThreadingPage threadingPage;
