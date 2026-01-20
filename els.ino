/*
 Name:		els.ino
 Created:	1/18/2026 2:01:02 PM
 Author:	HarvesteR
*/
#pragma once
#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

#define I2C_SCL 5
#define I2C_SDA 4

#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define SDD2 9 // not sure about this one
#define SDD3 10

#define MOTOR_PIN_DIR D3
#define MOTOR_PIN_STEP D4

// USER INPUTS -------------------------------

// buttons
#define BTN0 D7
#define BTN1 SDD3

// rotary handwheel pins (RTR0)
#define RTR0_A D5
#define RTR0_B D6

volatile uint32_t rot0_Tback;
volatile uint32_t rot0_Tfwd;
volatile int rot0_a = 0;
volatile int rot0_b = 0;

const uint32_t rotaryPulseTime = 20;




// device state variables  -------------------------------
//

int motorStepsPerRev = 400; // steps per revolution for the stepper motor

volatile int spindlerpm = 0;
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
PageValueRegion pvRPM = PageValueRegion(4, &spindlerpm);




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


// fwd declare page switch
void goToPage(int);



struct MainPage : Page
{
	int pageDst = 0; // destination page for navigation (TMP, just for testing while there are only 2 buttons)

	void drawOnce() override
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(" RPM ");
		

		lcd.setCursor(0, 3);

		switch (pageDst)
		{
			default:
			case 0:
				lcd.print(" CFG  THR  SPD  JOG ");
				break;
			case 1:
				lcd.print("\003CFG\002 THR  SPD  JOG ");
				break;
			case 2:
				lcd.print(" CFG \003THR\002 SPD  JOG ");
				break;
			case 3:
				lcd.print(" CFG  THR \003SPD\002 JOG ");
				break;
			case 4:
				lcd.print(" CFG  THR  SPD \003JOG\002");
			break;
		}
	}

	void drawLoop() override
	{
		// show RPM value in its region
		pvRPM.drawAt(lcd, 0,1);
	}
	void handleInputs(uint8_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x01)
		{
			// BTN0 pressed
			pageDst = (pageDst + 1) % 4;	
			drawOnce();
		}
		if (btns & 0x02)
		{
			// BTN1 pressed			
			goToPage(pageDst);

		}
	}
};
MainPage mainPage;


struct CfgPage : Page
{
	void drawOnce() override
	{
		lcd.clear();
		lcd.setCursor(0, 2);
		lcd.print("           000      ");
		lcd.setCursor(0, 3);
		lcd.print("\003SET\002 HWL  LDS  MSR ");
	}
	void drawLoop() override
	{
		pvHdWhl.drawAt(lcd, 6, 2);
		pvRPM.drawAt(lcd, 0, 3);
	}
	void handleInputs(uint8_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x01)
			goToPage(0);
	}
};
CfgPage cfgPage;

struct ThreadingPage : Page
{

	void drawOnce() override
	{
		// thrd page
		lcd.clear();

		lcd.print(" PCH  LEN  ");
		lcd.setCursor(0, 1);
		lcd.print("  M3  042  ");
		lcd.setCursor(0, 2);


		lcd.setCursor(0, 3);
		lcd.print("\003THR\002 PDT  END  JOG ");
	}
	void drawLoop() override
	{
		pvHdWhl.drawAt(lcd, 0, 3);
	}
	void handleInputs(uint8_t btns) override
	{
		// handle inputs for main page if needed
		if (btns & 0x01)
			goToPage(0);
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
		lcd.print("RPM");
		// l1
		pvRPM.drawAt(lcd, 0, 1);
		// l2

		// l3
		lcd.setCursor(0, 3);
		lcd.print("\003SPD\002 FRT  ZRO  JOG ");
	}
	void drawLoop() override
	{
		pvRPM.drawAt(lcd, 0, 1);
		pvTgtSpeed.drawAt(lcd, 6, 2);
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
			motorSpeed = 0;
			hdWhl0 = hdwhlCount;
		}
		else
		{
			int newspd = hdwhlCount - hdWhl0;

			if (motorSpeed != newspd)
			{
				if (newspd == 0)
					motor.stop();
				else								
					motor.setSpeed(newspd);
			
				motorSpeed = newspd;
			}
		}

		if (motorSpeed != 0)
			motor.runSpeed();

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










void setup() 
{
	// init usb serial for controlling
	Serial.begin(115200);
	
	motor.setMinPulseWidth(60); // set step pulse width to 600us
	motor.setPinsInverted(false, true, false);
	motor.setMaxSpeed(4000); // set max speed
	motor.setAcceleration(100); // set acceleration
	motor.stop();

	// set up 4x20 LCD display via SPI on the default pins
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
	pinMode(BTN0, INPUT_PULLUP);
	pinMode(BTN1, INPUT_PULLUP);

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
	


	// update fake RPM value
	spindlerpm = (spindlerpm + 3) % 10000; // dummy update
	currentPage->drawLoop();

	// handle input
	// grab button states here and pass

	uint8_t stBtns = 0;
	if (digitalRead(BTN0) == LOW) stBtns |= 0x01;
	if (digitalRead(BTN1) == LOW) stBtns |= 0x02;

	currentPage->handleInputs(stBtns);

	if (stBtns != 0)
		delay(150); // simple debounce

		
}



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



void IRAM_ATTR interrupt_ROT0()
{
	//encoderRead(RTR0_A, RTR0_B, &rot0_a, &rot0_b, &rot0_Tfwd, &rot0_Tback);

	// SAP impl, this interrupts on A rising edge, so state of B determines direction
	uint32_t mils = millis();	
	if (digitalRead(RTR0_B))
	{
		hdwhlCount++;
	}
	else
	{
		hdwhlCount--;		
	}
}

void encoderRead(int pinA, int pinB, volatile int* a0, volatile int* b0, volatile uint32_t* rot_Tfwd, volatile uint32_t* rot_Tback)
{
	uint32_t mils = millis();

	int a = digitalRead(pinA);
	int b = digitalRead(pinB);

	if (a != *a0)
	{
		*a0 = a;
		if (b != *b0)
		{
			*b0 = b;

			if (a == b)
			{
				if (rot0_Tback + rotaryPulseTime < mils)
				{
					*rot_Tfwd = mils;
					hdwhlCount++;
				}
			}
			else
			{
				if (rot0_Tfwd + rotaryPulseTime < mils)
				{
					*rot_Tback = mils;
					hdwhlCount--;
				}
			}
		}
	}
}
