/*
 Name:		els.ino
 Created:	1/18/2026 2:01:02 PM
 Author:	HarvesteR
*/
#pragma once
#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Arduino.h>


#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

#define I2C_SCL SCL
#define I2C_SDA SDA


#define MOTOR_PIN_DIR 2
#define MOTOR_PIN_STEP 4
// spindle encoder pins
#define SPNDL_ROT_A 35
#define SPNDL_ROT_B 34

// USER INPUTS -------------------------------

// buttons
#define BTNZRO 32 // zeroing button

#define BTNEXE 14 // exec button
#define LEDEXE 12 // exec led


#define BTN1 27 // SK 1
#define BTN2 26 // SK 2
#define BTN3 25 // SK 3
#define BTN4 33 // SK 4

// rotary handwheel pins (RTR0)
#define RTR0_A 18
#define RTR0_B 19


// LCD layout consts -------------------------------
#define C_FIELD0 0 //1234
#define C_FIELD1 5 //6789
#define C_FIELD2 10//1234
#define C_FIELD3 15//6789



volatile uint32_t rot0_Tback;
volatile uint32_t rot0_Tfwd;
volatile int rot0_a = 0;
volatile int rot0_b = 0;

const uint32_t rotaryPulseTime = 20;




// device state variables  -------------------------------
//

int motorStepsPerRev = 400; // steps per revolution for the stepper motor

volatile int spndlCount = 0;
volatile int hdwhlCount = 0;


AccelStepper motor(AccelStepper::DRIVER, MOTOR_PIN_STEP, MOTOR_PIN_DIR);



//initialize the liquid crystal library
//the first parameter is the I2C address
//the second parameter is how many rows are on your screen
//the third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27, 20, 4);
byte cc0[8] = {
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
};
byte cc1[8] = {
	B00000,
	B00000,
	B00000,
	B00000,    
	B00000,
	B00000,
	B00000,
	B00000,
};
byte cc2[8] = {
	B10000,    
	B10000,
	B10000,
	B10000,
	B10000,    
	B10000,
	B10000,
	B10000,
};
byte cc3[8] = {
	B00001,
	B00001,
	B00001,
	B00001,
	B00001,
	B00001,    
	B00001,
	B00001,    
};
byte cc4[8] = {
	B10001,
	B10001,
	B10001,
	B10001,
	B10001,
	B10001,
	B10001,
	B10001,
};

struct PageValueRegion
{

private:
	char buffer[16];

public:
	uint8_t length;

	volatile int* linkedValue = nullptr;

	PageValueRegion(uint8_t l, volatile int* val)
	{
		length = l;
		linkedValue = val;
	}


	void drawAt(LiquidCrystal_I2C& lcd, uint8_t col, uint8_t row)
	{
		lcd.setCursor(col, row);
		// draw value with leading zeros to fill length

		snprintf(buffer, sizeof(buffer), "%0*d", length, *linkedValue);
		lcd.print(buffer);
	}
};
PageValueRegion pvHdWhl = PageValueRegion(4, &hdwhlCount);
PageValueRegion pvSpndl = PageValueRegion(4, &spndlCount);
PageValueRegion pvMoV = PageValueRegion(4, &motorStepsPerRev);

struct Page
{
	virtual ~Page() {}       

	virtual void enterPage() {}


	virtual void drawOnce() = 0;

	virtual void drawLoop() = 0;

	virtual void handleInputs(uint8_t btns)	{ }

	virtual void exitPage() {}

};
Page* currentPage = nullptr;


// fwd declares
void goToPage(int);


bool execArmed;
void armExec()
{
	digitalWrite(LEDEXE, HIGH);
	execArmed = true;
	Serial.println("Exec armed");
}
void disarmExec()
{
	digitalWrite(LEDEXE, LOW);
	execArmed = false;
	Serial.println("Exec disarmed");
}
bool handleExec()
{
	if (execArmed && digitalRead(BTNEXE) == LOW)
	{
		digitalWrite(LEDEXE, LOW);
		execArmed = false;

		Serial.println("Exec triggered");
		return true;
	}
	return false;
}




String LabelAct(String label, bool act)
{
	return act ? ("\003"+label+"\002") : (" "+label+" ");
}

struct MainPage : Page
{
	int pageDst = 0; // destination page for navigation (TMP, just for testing while there are only 2 buttons)

	void enterPage() override
	{
		// returning to main page always disarms.
		disarmExec();
	}

	void drawOnce() override
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("SPNDL ");
		

		lcd.setCursor(0, 3);

		lcd.print(" CFG  THR  SPD  JOG ");		
	}

	void drawLoop() override
	{
		// show RPM value in its region
		pvSpndl.drawAt(lcd, C_FIELD0, 1);

	}
	void handleInputs(uint8_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x01)
		{
			// BTN1 pressed
			goToPage(1);
		}
		if (btns & 0x02)
		{
			// BTN2 pressed			
			goToPage(2);
		}
		if (btns & 0x04)
		{
			// BTN3 pressed			
			goToPage(3);
		}
		if (btns & 0x08)
		{
			// BTN4 pressed			
			armExec();
		}

	}
};
MainPage mainPage;


struct CfgPage : Page
{
	int selField = 0;

	void enterPage() override
	{
		selField = 0;
	}
	void drawOnce() override
	{
		lcd.clear();
		lcd.print("SPNDL ");
		lcd.setCursor(0, 2);
		
		lcd.setCursor(0, 3);
		lcd.print((String)"     "+LabelAct("MoV", selField == 1)+LabelAct("SpV", selField == 2)+LabelAct("HdW", selField == 3));
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 3);
		pvMoV.drawAt(lcd, C_FIELD1, 2);
		pvHdWhl.drawAt(lcd, C_FIELD3, 2);
	}
	void handleInputs(uint8_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x01)
		{
			goToPage(0);
			return;
		}
		if (btns & 0x02)
		{
			selField = 1;
			drawOnce();
			return;
		}
		if (btns & 0x04)
		{
			selField = 2;
			drawOnce();
			return;
		}
		if (btns & 0x08)
		{
			selField = 3;
			drawOnce();

			armExec();
			return;
		}

		if (btns & 0x10)
		{
			hdwhlCount = 0;
		}
	}
};
CfgPage cfgPage;

struct ThreadingPage : Page
{
	int selField = 0;


	void drawOnce() override
	{
		// thrd page
		lcd.clear();

		lcd.print(" PCH  LEN  ");
		lcd.setCursor(0, 1);
		lcd.print("  M3  042  ");
		lcd.setCursor(0, 2);


		lcd.setCursor(0, 3);
		lcd.print((String)"     " + LabelAct("PDT", selField == 1) + LabelAct("END", selField == 2) + LabelAct("JOG", selField == 3));

		//lcd.print("\003THR\002 PDT  END  JOG ");
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 3);
	}
	void handleInputs(uint8_t btns) override
	{
		if (btns & 0x01)
		{
			goToPage(0);
			return;
		}
		if (btns & 0x02)
		{
			selField = 1;
			drawOnce();
			return;
		}
		if (btns & 0x04)
		{
			selField = 2;
			drawOnce();
			return;
		}
		if (btns & 0x08)
		{
			selField = 3;
			drawOnce();

			armExec();
			return;
		}


	}
};
ThreadingPage threadingPage;


struct SpdPage : Page
{

	int motorSpeed = 0;
	int hdWhl0;

	PageValueRegion pvTgtSpeed = PageValueRegion(4, &motorSpeed);

	void enterPage() override
	{
		hdWhl0 = hdwhlCount;
		motorSpeed = 0;

		motor.stop();
		motor.setAcceleration(50);

	}
	void drawOnce() override
	{
		// thrd page
		lcd.clear();
		// l0
		lcd.print("SPNDL");
		// l1
		pvSpndl.drawAt(lcd, C_FIELD0, 1);
		// l2

		// l3
		lcd.setCursor(0, 3);
		lcd.print("\003SPD\002 FRT  ZRO  RUN ");
	}
	void drawLoop() override
	{
		pvSpndl.drawAt(lcd, C_FIELD0, 1);
		pvTgtSpeed.drawAt(lcd, C_FIELD1, 2);
	}
	void handleInputs(uint8_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x01)
		{
			goToPage(0);
			return;
		}		
		if (btns & 0x04)
		{
			motorSpeed = 0;
			hdWhl0 = hdwhlCount;
			motor.stop();
		}
		if (btns & 0x08)
		{
			armExec();
		}

		if (btns & 0x10)
		{
			motorSpeed = 0;
			hdWhl0 = hdwhlCount;
			motor.stop();
		}
		else
		{
			int spdSetting = hdwhlCount - hdWhl0;
						
			if (motorSpeed != spdSetting)
			{
				if (spdSetting == 0)
					motor.stop();
				else if (execArmed)
					motor.setSpeed(spdSetting);

				motorSpeed = spdSetting;
			}

			if (execArmed && motorSpeed != 0)
			{
				motor.runSpeed();
			}
		
		}

	}
	void exitPage() override
	{
		motor.stop();
		motorSpeed = 0;
	}
};
SpdPage spdPage;











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



void setup() 
{
	// init usb serial for controlling
	Serial.begin(115200);

	pinMode(MOTOR_PIN_DIR,  OUTPUT);
	pinMode(MOTOR_PIN_STEP, OUTPUT);
	motor.setMinPulseWidth(60); // set step pulse width to 600us
	motor.setPinsInverted(false, true, false);
	motor.setMaxSpeed(4000); // set max speed
	motor.setAcceleration(100); // set acceleration
	motor.stop();

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
	
	delay(2000);
	
	lcd.createChar(0, cc0);    
	lcd.createChar(1, cc1);   
	lcd.createChar(2, cc2);   
	lcd.createChar(3, cc3);   
	lcd.createChar(4, cc4);   
	
	
	
	lcd.setCursor(0, 1);
	lcd.print(" HRVToolworks ELS");
	lcd.setCursor(0, 2);
	lcd.print("  \000\001\002\003\004\005");


	delay(1500);

	// buttons
	pinMode(BTNZRO, INPUT_PULLUP);
	
	pinMode(BTNEXE, INPUT_PULLUP); 
	pinMode(LEDEXE, OUTPUT);



	pinMode(BTN1, INPUT_PULLUP);
	pinMode(BTN2, INPUT_PULLUP);
	pinMode(BTN3, INPUT_PULLUP);
	pinMode(BTN4, INPUT_PULLUP);


	// rotary inputs 
	pinMode(RTR0_A, INPUT_PULLUP);
	pinMode(RTR0_B, INPUT_PULLUP);


	rot0_Tback = 0;
	rot0_Tfwd = 0;
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
	uint8_t stBtns = 0;
	if (digitalRead(BTNZRO) == LOW) stBtns |= 0x10; // zero button		

	if (digitalRead(BTN1) == LOW) stBtns |= 0x01; // SK1	
	if (digitalRead(BTN2) == LOW) stBtns |= 0x02; // SK2
	if (digitalRead(BTN3) == LOW) stBtns |= 0x04; // SK3
	if (digitalRead(BTN4) == LOW) stBtns |= 0x08; // SK4
	

	currentPage->handleInputs(stBtns);

	if (handleExec())
	{
		// handle exec button action (maybe arming the exec takes a callback to execute on click?)
	}

	if (stBtns != 0)
		delay(150); // simple debounce

		
}


// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================
// ==========================================================================================================================



void handleSerial()
{
	if (Serial.available())
	{
		String input = Serial.readStringUntil('\n');

		// commands all begin with fwd slash. Next 3 chars are the command, then a space, then space separated parameters


		if (input.startsWith("/bri"))
		{
			int bri = input.substring(5).toInt();
			if (bri < 0) bri = 0;
			if (bri > 255) bri = 255;
			lcd.setBacklight(bri);
			Serial.print("Backlight set to ");
			Serial.println(bri);
		}
		if (input.startsWith("/cls"))
		{
			lcd.clear();
			Serial.println("LCD Cleared");
		}
		if (input.startsWith("/pos"))
		{
			int space1 = input.indexOf(' ', 5);
			int col = input.substring(5, space1).toInt();
			int row = input.substring(space1 + 1).toInt();

			lcd.setCursor(col, row);
			Serial.print("Cursor set to ");
			Serial.print(col);
			Serial.print(", ");
			Serial.println(row);
		}
		if (input.startsWith("/wrt"))
		{
			String toPrint = input.substring(5);

			// parse custom chars (0-7) 
			toPrint = parseChars(toPrint);

			lcd.print(toPrint);
			Serial.println(toPrint);
		}
		if (input.startsWith("/wcc"))
		{
			int cc = input.substring(5).toInt();
			lcd.write(byte(cc));
		}

		if (input.startsWith("/pag"))
		{
			int newPage = input.substring(5).toInt();
			goToPage(newPage);			
		}
	}
}
String parseChars(String inStr)
{
	String outStr = "";
	for (unsigned int i = 0; i < inStr.length(); i++)
	{
		if (inStr.charAt(i) == '\\' && i + 1 < inStr.length())
		{
			char nextChar = inStr.charAt(i + 1);
			if (nextChar >= '0' && nextChar <= '7')
			{
				outStr += byte(nextChar - '0');
				i++; // skip next char
			}
			else
			{
				outStr += inStr.charAt(i);
			}
		}
		else
		{
			outStr += inStr.charAt(i);
		}
	}
	return outStr;
}






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
