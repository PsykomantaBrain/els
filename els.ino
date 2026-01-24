/*
 Name:		els.ino
 Created:	1/18/2026 2:01:02 PM
 Author:	HarvesteR
*/
#pragma once

//#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Arduino.h>


#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))



// motor and spindle encoder pins
#define MOTOR_PIN_DIR 2
#define MOTOR_PIN_STEP 4
#define SPNDL_ROT_A 35
#define SPNDL_ROT_B 34

// USER INPUTS -------------------------------

// buttons
#define BTNZRO 32 // zeroing button

#define BTNRUN 14 // run button
#define LEDRUN 12 // run led
#define BTNSTP 23 // stop button
#define LEDSTP 13 // stop led



#define BTN1 27 // SK 1
#define BTN2 26 // SK 2
#define BTN3 25 // SK 3
#define BTN4 33 // SK 4

#define BTN5 16 // SK 5
#define BTN6 17 // SK 6
#define BTN7 39 // SK 7
#define BTN8 36 // SK 8

// rotary handwheel pins (RTR0)
#define RTR0_A 18
#define RTR0_B 19







// config params -------------------------------
// these need to get saved to eeprom and reloaded on startup 

int btnsState = 0; // current button states bitmask

int motorStepsPerRev = 400;    //  steps per revolution for the stepper motor
int spindlePulsesPerRev = 600; // pulses per revolution for the spindle encoder





// device state variables  -------------------------------
//


volatile int spndlCount = 0;
volatile int hdwhlCount = 0;

//AccelStepper motor(AccelStepper::DRIVER, MOTOR_PIN_STEP, MOTOR_PIN_DIR);

#include "LedcStepperCtrl.h"


//initialize the liquid crystal library
//the first parameter is the I2C address
//the second parameter is how many rows are on your screen
//the third parameter is how many columns are on your screen
#define I2C_SCL SCL
#define I2C_SDA SDA

LiquidCrystal_I2C lcd(0x27, 20, 4);
#include "LCDCustomChars.h"


#include "ArmingButton.h"
// arming buttons for STOP and RUN
ArmingButton btnStop(BTNSTP, LEDSTP);
ArmingButton btnRun(BTNRUN, LEDRUN);


#include "Page.h"


PageValueInt pvHdWhl = PageValueInt(4, &hdwhlCount);
PageValueInt pvSpndl = PageValueInt(4, &spndlCount);
PageValueInt pvMoV = PageValueInt(4, &motorStepsPerRev);

PageValueInt pvBtns = PageValueInt(4, &btnsState);

Page* currentPage = nullptr;


// fwd declares
void goToPage(int);



String LabelAct(String label, bool act)
{
	return act ? ("\003"+label+"\002") : (" "+label+" ");
}



#include "MainPage.h"
#include "ConfigPage.h"
#include "ThreadingPage.h"
#include "SpeedPage.h"



void setPage(Page* p)
{
	if (currentPage != nullptr)
		currentPage->exitPage();

#ifndef __INTELLISENSE__	
	currentPage = p; // intellisense keeps complaining about this line, but it compiles and works fine, so suppress the warning
#endif

	currentPage->enterPage();
	currentPage->drawOnce();
}
void goToPage(int iPage)
{
	switch (iPage)
	{
	default:
	case 0:
		setPage(&mainPage);
		break;
	case 1:
		setPage(&cfgPage);
		break;
	case 2:
		setPage(&threadingPage);
		break;
	case 3:
		setPage(&spdPage);
		break;

	}
	Serial.print("Page set to ");
	Serial.println(iPage);
}


// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================


#include "SerialComms.h"


void setup() 
{
	// init usb serial for controlling
	Serial.begin(115200);
	//motor.setMinPulseWidth(60); // set step pulse width to 600us
	//motor.setPinsInverted(false, true, false);
	//motor.setMaxSpeed(4000); // set max speed
	//motor.setAcceleration(100); // set acceleration
	//motor.stop();

	// just for now, read spindle encoder with interrupts.
	// later the idea is to use PCNT hardware peripheral for this
	pinMode(SPNDL_ROT_A, INPUT_PULLUP);
	pinMode(SPNDL_ROT_B, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(SPNDL_ROT_A), interrupt_SPNDL, RISING);



	// set up 4x20 LCD display via SPI on the default pins
	pinMode(I2C_SDA, OUTPUT);
	pinMode(I2C_SCL, OUTPUT);
	Wire.begin(I2C_SDA, I2C_SCL);
	lcd.init();	
	lcd.backlight();
	lcd.setBacklight(64);	
	delay(1000);	
	addLCDCustomChars(lcd);
	
	
	
	lcd.setCursor(0, 1);
	lcd.print(" HRVToolworks ELS");
	lcd.setCursor(0, 2);
	lcd.print(" \001\002\003\004\005");


	delay(1000);

	
	if (stepperSetup())
		lcd.print("OK");
	else
		lcd.print("STEPPER FAIL");

	delay(500);

	// buttons
	pinMode(BTNZRO, INPUT_PULLUP);

	pinMode(BTN1, INPUT_PULLUP);
	pinMode(BTN2, INPUT_PULLUP);
	pinMode(BTN3, INPUT_PULLUP);
	pinMode(BTN4, INPUT_PULLUP);
	pinMode(BTN5, INPUT_PULLUP);
	pinMode(BTN6, INPUT_PULLUP);
	pinMode(BTN7, INPUT);
	pinMode(BTN8, INPUT);




	// handwheel rotary inputs 
	pinMode(RTR0_A, INPUT_PULLUP);
	pinMode(RTR0_B, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(RTR0_A), interrupt_ROT0, RISING);


	delay(500);
	setPage(&mainPage);

	Serial.println("ready");
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	// Handle serial data if there is anything in the buffer. 
	handleSerial();
	

	
	currentPage->drawLoop();

	// handle input
	
	

	// grab button states here and pass
	uint16_t skBtns = 0;

	// handle arming buttons first, nothing else if they are pressed
	if (btnStop.inputUpdate())
	{
		Serial.println("STOP triggered");	
		skBtns |= 0x2000; 
	}
	else if (btnRun.inputUpdate())
	{
		Serial.println("RUN triggered");
		skBtns |= 0x1000; 
	}
	else
	{
		if (digitalRead(BTNZRO) == LOW) skBtns |= 0x0100; // ZERO button

		if (digitalRead(BTN1) == LOW) skBtns |= 0x0001; // SK1	
		if (digitalRead(BTN2) == LOW) skBtns |= 0x0002; // SK2
		if (digitalRead(BTN3) == LOW) skBtns |= 0x0004; // SK3
		if (digitalRead(BTN4) == LOW) skBtns |= 0x0008; // SK4
		if (digitalRead(BTN5) == LOW) skBtns |= 0x0010; // SK5
		if (digitalRead(BTN6) == LOW) skBtns |= 0x0020; // SK6
		if (digitalRead(BTN7) == LOW) skBtns |= 0x0040; // SK7
		if (digitalRead(BTN8) == LOW) skBtns |= 0x0080; // SK8
	}

	btnsState = skBtns; 
	currentPage->pageUpdate(skBtns);

	if (skBtns != 0)
		delay(150); // simple debounce

		
}


// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================







void IRAM_ATTR interrupt_ROT0()
{	
	// SAP impl, this interrupts on A rising edge, so state of B determines direction	
	if (digitalRead(RTR0_B))
	{
		hdwhlCount++;
	}
	else
	{
		hdwhlCount--;		
	}
}


void IRAM_ATTR interrupt_SPNDL()
{
	// SAP impl, this interrupts on A rising edge, so state of B determines direction	
	if (digitalRead(SPNDL_ROT_B))
	{
		spndlCount++;
	}
	else
	{
		spndlCount--;
	}
}
