#pragma once  
#include <Arduino.h>  // Include the Arduino core library for ESP32 to define ledcSetup and related functions  

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper* stepper = nullptr;

float stepperGetCurrentPulseRate()
{
	return stepper->getCurrentSpeedInMilliHz() / 1000.0f;
}

bool stepperSetup(uint8_t resolution = 8)
{
	bool ok = true;

	pinMode(MOTOR_PIN_DIR, OUTPUT);
	pinMode(MOTOR_PIN_STEP, OUTPUT);

	engine.init();
	stepper = engine.stepperConnectToPin(MOTOR_PIN_STEP);
	if (stepper == nullptr) 
	{
		return false;
	}
	else 
	{
		stepper->setDirectionPin(MOTOR_PIN_DIR);				
		stepper->setAcceleration(5000);
		stepper->setAutoEnable(true);
		
	}

	return ok;
}

void stepperDirection(bool dir)
{
	digitalWrite(MOTOR_PIN_DIR, dir ? HIGH : LOW);
}

bool stepperStop()
{
	stepper->stopMove();
	return true;
}

// stepsPerSecond = 0 disables pulses  
uint32_t stepperRunPPS(float stepsPerSecond)
{
	stepperDirection(stepsPerSecond >= 0.0f); // set direction based on sign of speed command
	stepsPerSecond = fabsf(stepsPerSecond);


	if (stepsPerSecond == 0.0f)
	{
		stepper->stopMove();
		return 0;
	}

	bool ok = stepper->setSpeedInHz((uint32_t)stepsPerSecond) == 0;
	ok &= stepper->setAcceleration(motorMaxAccel) == 0;
	if (!ok)
	{
		return 0;
	}
	else
	{	
		if (stepsPerSecond > 0.0f)
		{
			stepper->runForward();
		}
		else
		{
			stepper->runBackward();
		}
		
		return (uint32_t)stepsPerSecond;
	}
}

// set stepper speed in revs per second (from known steps per rev)
uint32_t stepperRunRPS(float revsPerSecond, int stepsPerRev)
{
	float stepsPerSecond = revsPerSecond * (float)stepsPerRev;
	return stepperRunPPS(stepsPerSecond);
}

// set stepper speed in RPM (from known steps per rev)
uint32_t stepperRunRPM(float rpm, int stepsPerRev)
{
	float revsPerSecond = rpm / 60.0f;
	return stepperRunRPS(revsPerSecond, stepsPerRev);
}


bool stepperMoveToTgt(int tgt, uint32_t speedHz)
{ 
	bool ok = stepper->setSpeedInHz(speedHz) == 0;
	ok &= stepper->setAcceleration(motorMaxAccel) == 0;
		
	if (ok)
	{
		MoveResultCode mrc = stepper->moveTo(tgt);
		if (mrc == MoveResultCode::OK)
			return true;
		else
		{
			// print error to serial
			Serial.print("stepperMoveToTgt failed: ");
			Serial.println(toString(mrc));
			return false;
		}
	}
}