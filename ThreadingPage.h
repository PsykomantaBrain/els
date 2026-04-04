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

	int motorTarget = 0;
	PageValueInt pvMot = PageValueInt(4, &motorTarget);

	int runVel = 0; // for display only, calculated in the run task based on the spindle speed and pitch
	PageValueInt pvVel = PageValueInt(4, &runVel);


	CoupledRunF32 coupledRun;




	EditableValueInt* getEvAtField(int index) override
	{
		switch (index)
		{
		case 4:
			return &evPitch;
		case 5: 
			//is DIR button. No editable value, but toggle direction on press (while not running)

			if (!btnStop.IsArmed()) // not while running. 
			{
				motorDirection = motorDirection == 0 ? 2 : 0; // toggle between REV and FWD
				stepperDirection(motorDirection > 0); // set direction 

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
		// DIR btn on field 1
		evCplspd.drawCaption(lcd, C_FIELD2, 0);
		evCplacc.drawCaption(lcd, C_FIELD3, 0);

		

		
		lcd.setCursor(0, 3);
		lcd.print(" ...  Spn  Vel  MoT ");
	}
	void drawLoop() override
	{

		pvPitch.drawAt(lcd, C_FIELD0, 1);
		pvDir.drawAt(lcd, C_FIELD1, 1);
		pvCplspd.drawAt(lcd, C_FIELD2, 1);
		pvCplacc.drawAt(lcd, C_FIELD3, 1);


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

		
		// default behaviour for other buttons
		Page::pageUpdate(btns);

		
		
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
						digitalWrite(LEDRUN, 0);
						vTaskDelete(NULL);
						return;
					}

					digitalWrite(LEDRUN, (millis() / 250) % 2); // 2 Hz blink while waiting

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

				page->coupledRun.beginRun(spndlCount, stepper->getCurrentPosition(), (float)page->pitchUm);
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
		
		// measuring spindle velocity for the commanded move speed makes a HUGE difference in smoothness of the motion vs going to the target at a fixed speed.
		// I tried deriving acceleration as well, but that made the motor lag behind quite a bit. With acc set high enough, it moves smoothly enough and there's no perceivable latency.
		vel = coupledRun.updStepperSpeed(spndl, micros());
		runVel = (int)vel; // update display value
		

		stepperMoveToTgt(motorTarget, vel, cplAccel);
	}

};
ThreadingPage threadingPage;
