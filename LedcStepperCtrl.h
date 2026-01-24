#pragma once  
#include <Arduino.h>  // Include the Arduino core library for ESP32 to define ledcSetup and related functions  



float stepperGetCurrentPulseRate()
{
	// get current frequency on channel (Arduino ESP32 core)  
	return (float)ledcReadFreq(MOTOR_PIN_STEP);
}

bool stepperSetup()  
{  
	bool ok = true;

	pinMode(MOTOR_PIN_DIR, OUTPUT);
	pinMode(MOTOR_PIN_STEP, OUTPUT);

	// configure LEDC for stepper control
	ok &= ledcAttach(MOTOR_PIN_STEP, 1000, 12);              // 1kHz initial freq, 14-bit resolution 
	ok &= ledcWrite(MOTOR_PIN_STEP, 0);                   // start with 0 duty cycle (no pulses)

	return ok;
}  

void stepperDirection(bool dir)
{
	digitalWrite(MOTOR_PIN_DIR, dir ? HIGH : LOW);
}

bool stepperStop()
{
	return ledcWrite(MOTOR_PIN_STEP, 0);                 // duty 0 => no pulses  
}

// stepsPerSecond = 0 disables pulses  
uint32_t stepperRunPPS(float stepsPerSecond)  
{  
	stepperDirection(stepsPerSecond >= 0.0f); // set direction based on sign of speed command
	stepsPerSecond = fabsf(stepsPerSecond);

	// it seems that frequencies below 10Hz are not supported and will crash the thing.
   if (stepsPerSecond < 10.0f) {  
	   return ledcWrite(MOTOR_PIN_STEP, 0);                 // duty 0 => no pulses  	     
   }  

   // LEDC frequency is integer Hz. Round and clamp.  
   uint32_t f = (uint32_t)(stepsPerSecond + 0.5f);  
   if (f < 1) f = 1;  


   return ledcWriteTone(MOTOR_PIN_STEP, f);               // sets freq on channel (Arduino ESP32 core)  
   // no need to set duty, writeTone sets it to 50% by default
   
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