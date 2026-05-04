#pragma once

// Modal page that takes over after a threading pass hits the endstop.
// Prompts the user to retract the cross slide, then on ZERO press performs
// a backlash-compensated return to the run-start motor position.
//
// Backlash strategy: overshoot the start position by `backlashCompUM` in the
// direction OPPOSITE to the cut, then come back to the start position by
// travelling in the cut direction. This leaves the leadscrew nut engaged on
// the same face it would be on at the start of the next cut, so the next
// pass starts with no backlash to take up.

struct ReturnPage : Page
{
	// inputs populated by the threading page before goToPage(PAGE_RETURN)
	int m0 = 0;             // run-start motor position (target for the return)
	int endPos = 0;         // motor position when the endstop was reached
	int returnSpeed = 42000; // steps/sec for the return move
	int returnAcc   = 42000; // steps/sec² for the return move

	enum State : uint8_t
	{
		ST_PROMPT,            // waiting for user to retract and press ZERO
		ST_MOVING_OVERSHOOT,  // moving to the overshoot position past m0
		ST_MOVING_HOME,       // moving back to m0 in the cut direction
		ST_DONE               // arrived; ready to bounce back to threading page
	};
	State state = ST_PROMPT;

	int overshootPos = 0;       // motor target for the overshoot leg
	int cutDirSign = 0;         // +1 or -1; direction of cut travel (endPos - m0 sign)
	uint32_t doneAtMs = 0;      // millis() when we entered ST_DONE


	void enterPage() override
	{
		Page::enterPage();
		state = ST_PROMPT;

		// derive cut direction from the actual travel during the run
		int travel = endPos - m0;
		cutDirSign = (travel > 0) ? +1 : (travel < 0) ? -1 : +1; // default to +1 if zero

		// overshoot carriage return move by backlashCompUM 
		// homing then approaches m0 from the same direction as the cut, to take up the backlash.
		// baclashCompUM needs to be at least as large as the actual mechanical backlash in the leadscrew/nut to be effective.

		int backlashSteps = 0;
		if (leadscrewPitchUM > 0)
			backlashSteps = (int)((double)backlashCompUM * (double)motorStepsPerRev / (double)leadscrewPitchUM);
		overshootPos = m0 - backlashSteps * cutDirSign;

		// safety: keep operator buttons disarmed while in the modal
		btnRun.disarm();
		btnStop.disarm();
	}


	void drawOnce() override
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		//lcd.print(" END STOP REACHED   ");

		switch (state)
		{
		case ST_PROMPT:
			lcd.setCursor(0, 0);
			lcd.print(" RETRACT CROSS SLIDE");
			lcd.setCursor(0, 1);
			lcd.print("[ZERO]: Carriage Return");
			break;
		case ST_MOVING_OVERSHOOT:
			lcd.setCursor(0, 1);
			lcd.print(" RETURNING...       ");
			lcd.setCursor(0, 2);
			lcd.print(" (BACKLASH COMP)    ");
			break;
		case ST_MOVING_HOME:
			lcd.setCursor(0, 1);
			lcd.print(" RETURNING...       ");
			lcd.setCursor(0, 2);
			lcd.print(" HOMING             ");
			break;
		case ST_DONE:
			lcd.setCursor(0, 1);
			lcd.print(" HOMED              ");
			lcd.setCursor(0, 2);
			lcd.print("                    ");
			break;
		}
	}

	void drawLoop() override
	{
		// DRO on the bottom row so the operator can see actual carriage position throughout
		pvDRO.drawAt(lcd, C_FIELD0, 3);
	}


	void pageUpdate(uint16_t btns) override
	{
		// SK1 aborts the modal back to threading page (no return move performed)
		if (btns & 0x0001)
		{
			stepperStop();
			goToPage(PAGE_THREADING);
			return;
		}

		switch (state)
		{
		case ST_PROMPT:
			if (btns & 0x0100) // ZERO
			{
				startOvershoot();
			}
			break;

		case ST_MOVING_OVERSHOOT:
			// once the overshoot move has settled, kick off the home leg
			if (!stepper->isRunning() && stepper->getCurrentPosition() == overshootPos)
			{
				startHome();
			}
			break;

		case ST_MOVING_HOME:
			if (!stepper->isRunning() && stepper->getCurrentPosition() == m0)
			{
				state = ST_DONE;
				doneAtMs = millis();
				drawOnce();
			}
			break;

		case ST_DONE:
			// brief dwell so the user sees the "HOMED" confirmation, then back to threading
			if (millis() - doneAtMs > 600)
			{
				goToPage(PAGE_THREADING);
				return;
			}
			break;
		}
	}


	void startOvershoot()
	{
		stepper->setSpeedInHz((uint32_t)abs(returnSpeed));
		stepper->setAcceleration(min(returnAcc, motorMaxAccel));
		stepper->moveTo(overshootPos);
		state = ST_MOVING_OVERSHOOT;
		drawOnce();
	}

	void startHome()
	{
		stepper->setSpeedInHz((uint32_t)abs(returnSpeed));
		stepper->setAcceleration(min(returnAcc, motorMaxAccel));
		stepper->moveTo(m0);
		state = ST_MOVING_HOME;
		drawOnce();
	}


	void exitPage() override
	{
		Page::exitPage();
	}
};
ReturnPage returnPage;
