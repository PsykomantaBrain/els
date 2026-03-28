/*
 Name:		els.ino
 Created:	1/18/2026 2:01:02 PM
 Author:	HarvesteR
*/
#pragma once

#include <EEPROM.h>
#include <AVRStepperPins.h>
#include <FastAccelStepper.h>
#include <Log2Representation.h>
#include <Log2RepresentationConst.h>
#include <pico_pio.h>
#include <RampCalculator.h>
#include <RampControl.h>
#include <RampGenerator.h>
#include <StepperISR.h>


#include <Arduino.h>

#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))






// motor and spindle encoder pins
#define MOTOR_PIN_DIR 2
#define MOTOR_PIN_STEP 4
#define SPNDL_ROT_A 35
#define SPNDL_ROT_B 34

// USER INPUTS -------------------------------


// buttons
#define BTNZRO 25 // zeroing button OK
#define BTNRUN 26 // run button		OK
#define LEDRUN 27 // run led		OK
#define BTNSTP 12 // stop button	OK
#define LEDSTP 14 // stop led		OK



#define BTN1 23 // SK 1
#define BTN2 32 // SK 2 OK
#define BTN3 33 // SK 3 OK
#define BTN4 13 // SK 4

#define BTN5 17 // SK 5 OK
#define BTN6 16 // SK 6
#define BTN7 36 // SK 7
#define BTN8 39 // SK 8

// rotary handwheel pins (RTR0)
#define RTR0_A 18
#define RTR0_B 19


#define I2C_SCL SCL
#define I2C_SDA SDA



#include <Wire.h>
#include <LiquidCrystal_I2C.h>




// config params -------------------------------
// these need to get saved to eeprom and reloaded on startup 

int btnsRxState = 0; // button states sent over serial

int motorStepsPerRev = 400;    //  steps per revolution for the stepper motor
int spindlePulsesPerRev = 600; // pulses per revolution for the spindle encoder
int leadscrewPitchUM = 2000;      // lead screw pitch in �m/rev
int motorMaxAccel = 1000;      // max acceleration for the stepper motor in steps/s�


//#include "LedcStepperCtrl.h"
#include "FastAccelStepperCtrl.h"


// device state variables  -------------------------------
//


volatile int spndlCount = 0;
volatile int hdwhlCount = 0;



//initialize the liquid crystal library
//the first parameter is the I2C address
//the second parameter is how many rows are on your screen
//the third parameter is how many columns are on your screen


LiquidCrystal_I2C lcd(0x27, 20, 4);
#include "LCDCustomChars.h"


#include "ArmingButton.h"
// arming buttons for STOP and RUN
ArmingButton btnStop(BTNSTP, LEDSTP);
ArmingButton btnRun(BTNRUN, LEDRUN);


// fwd declares
void goToPage(int);

#include "Page.h"
Page* currentPage = nullptr;


int stepperCurrentPos;
PageValueInt pvMpos = PageValueInt(4, &stepperCurrentPos);

PageValueInt pvHdWhl = PageValueInt(4, &hdwhlCount);
PageValueInt pvSpndl = PageValueInt(4, &spndlCount);


// leadscrew DRO 
double leadscrewDatum; // zeroing datum for leadscrew readout
PageValueDouble pvDatum = PageValueDouble(5, &leadscrewDatum);
double leadscrewDRO; // calculated position of carriage from the stepper position and zeroing datum
PageValueDouble pvDRO = PageValueDouble(5, &leadscrewDRO);

void UpdateDRO(int btns)
{
	double stepLdAdv = leadscrewPitchUM * (1.0 / motorStepsPerRev);

	if ((btns & 0x0100) != 0) // Zero button
	{
		leadscrewDatum = ((double)stepperCurrentPos * stepLdAdv) / 1000.0; // in mm
	}

	leadscrewDRO = ((double)stepperCurrentPos * stepLdAdv) / 1000.0 - leadscrewDatum; // in mm
}



String LabelAct(String label, bool act)
{
	return act ? ("\003"+label+"\002") : (" "+label+" ");
}



#include "MainPage.h"
#include "ConfigPage.h"
#include "ThreadingPage.h"
#include "SpeedPage.h"
#include "JogPage.h"

// TO DO: add jog page (digital carriage handwheel mode - ie, coupled run to handwheel instead of spindle)



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
	case 4:
		setPage(&jogPage);
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

#include "PCNT_handwheel.h"
#include "PCNT_spindle.h"

void setup() 
{
	// init usb serial for controlling
	Serial.begin(115200);

	// zero button needs to be set early because we're reading it to decide whether to load config from eeprom
	pinMode(BTNZRO, INPUT_PULLUP);

	delay(200);

	// load config from eeprom (if not holding zero button)
	EEPROM.begin(512);
	if (digitalRead(BTNZRO) == HIGH)
		loadConfigFromEEPROM();
	else
		Serial.println("Skipping config load from EEPROM (ZERO button held)");

	// just for now, read spindle encoder with interrupts.
	// later the idea is to use PCNT hardware peripheral for this
	//pinMode(SPNDL_ROT_A, INPUT);
	//pinMode(SPNDL_ROT_B, INPUT);
	//attachInterrupt(digitalPinToInterrupt(SPNDL_ROT_A), interrupt_SPNDL, RISING);



	// set up 4x20 LCD display via SPI on the default pins
	pinMode(I2C_SDA, OUTPUT);
	pinMode(I2C_SCL, OUTPUT);
	Wire.begin(I2C_SDA, I2C_SCL);
	lcd.init();	
	lcd.backlight();
	lcd.setBacklight(64);	
	delay(800);	
	addLCDCustomChars(lcd);		


	lcd.setCursor(0, 1);
	lcd.print(" HRVToolworks ELS");
	lcd.setCursor(0, 2);
	testLCDCustomChars(lcd);
	

	delay(1000);


	const uint8_t ledcRes = 16; // ledc resolution bits	
	if (stepperSetup(ledcRes))
	{
		lcd.print("OK");
		Serial.println((String)"Stepper Config Ok");
	}
	else
	{
		lcd.print(" STPPR FAIL");
		Serial.println((String)"Stepper Config FAILED");
	}

	delay(500);

	// buttons
	pinMode(BTN1, INPUT_PULLUP);
	pinMode(BTN2, INPUT_PULLUP);
	pinMode(BTN3, INPUT_PULLUP);
	pinMode(BTN4, INPUT_PULLUP);
	pinMode(BTN5, INPUT_PULLUP);
	pinMode(BTN6, INPUT_PULLUP);
	pinMode(BTN7, INPUT);
	pinMode(BTN8, INPUT);




	// handwheel rotary inputs 
	//pinMode(RTR0_A, INPUT_PULLUP);
	//pinMode(RTR0_B, INPUT_PULLUP);
	//attachInterrupt(digitalPinToInterrupt(RTR0_A), interrupt_ROT0, RISING);

	// read handwheel with PCNT hardware peripheral 
	setup_handwheel_pcnt();

	// spindle encoder with PCNT hardware peripheral
	setup_spindle_pcnt();

	

	delay(500);
	setPage(&mainPage);

	Serial.println("ready");
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	btnsRxState = 0;

	// Handle serial data if there is anything in the buffer. 
	handleSerial();
	
	// update global state variables (that need updating)
	stepperCurrentPos = stepper->getCurrentPosition();
	spndlCount = read_spindle();
	hdwhlCount = read_handwheel();
		

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
	skBtns |= btnsRxState;
		
	UpdateDRO(skBtns);
	currentPage->drawLoop();

	currentPage->pageUpdate(skBtns);

	if (skBtns != 0)
		delay(150); // simple debounce

		
}


// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================







//void IRAM_ATTR interrupt_ROT0()
//{	
//	// SAP impl, this interrupts on A rising edge, so state of B determines direction	
//	if (digitalRead(RTR0_B))
//	{
//		hdwhlCount++;
//	}
//	else
//	{
//		hdwhlCount--;		
//	}
//}


//void IRAM_ATTR interrupt_SPNDL()
//{
//	// SAP impl, this interrupts on A rising edge, so state of B determines direction	
//	if (digitalRead(SPNDL_ROT_B))
//	{
//		spndlCount++;
//	}
//	else
//	{
//		spndlCount--;
//	}
//}
