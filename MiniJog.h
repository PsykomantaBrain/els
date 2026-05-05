#pragma once
#include "CoupledRun.h"
#include "PCNT_handwheel.h"

class MiniJog
{
public:

	int Mode = 0; // 0 = off, 1 = 1x, 2 = 10x, 3 = 100x

	int layoutField = C_FIELD2;
	int layoutRowVal = 2;
	int layoutRowCapt = 3;
	
	int maxAccel = 100000;
	int maxSpeed = 42000;

	int target = 0; // target position for mini-jog mode, in motor steps from current position when mode is activated
	int tgtDRO = 0.0; // for display only, target position converted to µm for display
	PageValueInt pvMiniTgt = PageValueInt(4, &tgtDRO);

	CoupledRunF32 coupledRun;


	void layout(int field, int rowVal, int rowCap)
	{
		layoutField = field;
		layoutRowVal = rowVal;
		layoutRowCapt = rowCap;
	}

	void cycleMode()
	{
		setMode((Mode + 1) % 4);
	}

	void Stop()
	{
		setMode(0);
	}

	void setMode(int mode)
	{
		if (mode < 0 || mode > 3)
			return;		

		if (mode == 0)
		{
			if (coupledRun.isRunning())
			{
				btnStop.disarm();
				coupledRun.endRun();
				stepperStop();
			}
		}
		else
		{
			float step = 0.0f;
			switch (mode)
			{
			case 0:
				step = 0.0f;
			break;
			case 1:
				step = 10000.0f;
				break;
			case 2:
				step = 1000.0f;
			break;
			case 3:
				step = 100.0f;
			break;
			}

			if (step > 0.0f)
			{
				btnRun.disarm();
				btnStop.arm();

				stepper->setSpeedInHz(maxSpeed);
				stepper->setAcceleration(maxAccel);
				coupledRun.beginRun(hdwhlCount, stepper->getCurrentPosition(), step, hdwlPulsesPerRevolution);

				if (Mode == 0)
				{
					startRunTask();
				}
			}
		}

		Mode = mode;

		drawonce();
	}



	void drawonce()
	{
		lcd.setCursor(layoutField, layoutRowCapt);
		switch (Mode)
		{
		case 0:
			lcd.print(" JOG ");

			lcd.setCursor(layoutField, layoutRowVal);
			lcd.print("     "); // clear out the value row
			break;
		case 1:
			lcd.print("\003J10\002");
			break;
		case 2:
			lcd.print("\003J1\005\002");
			break;
		case 3:
			lcd.print("\003J.1\002");
			break;
		}

	}

	void drawloop()
	{
		switch (Mode)
		{
		case 0:
			break;
		case 1:
		case 2:
		case 3:
			pvMiniTgt.drawAt(lcd, layoutField, layoutRowVal); lcd.print("\009");
			break;
		}

	}

	private:


	float vel, acc;

	void coupledRunTask()
	{
		int hdwl = read_handwheel(); // up to date handwheel value

		target = coupledRun.getTargetMotorCount(hdwl);

		tgtDRO = ((target - coupledRun.m0)*1000) / (motorStepsPerRev*1000) * leadscrewPitchUM / 1000; // convert target to µm for display

		// measuring handwheel velocity for the commanded move speed, maybe it's also smoother like it was with the spindle		
		vel = coupledRun.updStepperSpeed(hdwl, micros());

		stepperMoveToTgt(target, vel, maxAccel);
	}

	void startRunTask()
	{
		vel = 0.0f;
		acc = 0.0f;

		// start the run task pinned to core 1
		xTaskCreatePinnedToCore(
			[](void* param)
			{
				MiniJog* mj = (MiniJog*)param;
				const TickType_t xDelay = pdMS_TO_TICKS(10);
				while (mj->coupledRun.isRunning())
				{
					mj->coupledRunTask();
					vTaskDelay(xDelay);
				}
				vTaskDelete(NULL);
			},
			"MiniJogCoupledRunTask",
			2048,
			(void*)this,
			1,
			NULL,
			1); // core 1

	}

};
MiniJog miniJog;