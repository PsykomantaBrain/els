#pragma once

// pages by index
#define PAGE_MAIN 0
#define PAGE_CONFIG 1
#define PAGE_THREADING 2
#define PAGE_SPEED 3


struct MainPage : Page
{
	void enterPage() override
	{
		// returning to main page always disarms RUN. (and probably should trigger STOP?)
		btnRun.disarm();
	}

	void drawOnce() override
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(" ...  ...  ...  CFG ");

		lcd.setCursor(0, 1);
		lcd.print("SPNDL"); pvSpndl.drawAt(lcd, C_FIELD1, 1);

		lcd.setCursor(0, 2);
		lcd.print("HdWhl"); pvHdWhl.drawAt(lcd, C_FIELD1, 2);

		lcd.setCursor(0, 3);
		lcd.print("      THR  SPD  ... ");
	}

	void drawLoop() override
	{
		// show RPM value in its region
		pvSpndl.drawAt(lcd, C_FIELD1, 1);
		pvHdWhl.drawAt(lcd, C_FIELD1, 2);

	}
	void pageUpdate(uint16_t btns) override
	{

		// handle inputs for main page if needed

		if (btns & 0x0001)
		{
			// nothing in this page. This is the RTN button in other pages.
		}
		if (btns & 0x0002)
		{
			// BTN2 pressed			
			goToPage(2);
		}
		if (btns & 0x0004)
		{
			// BTN3 pressed			
			goToPage(3);
		}
		if (btns & 0x0008)
		{
			// BTN4 pressed			
	
		}

		if (btns & 0x0010)
		{
			// BTN5 pressed			
		}
		if (btns & 0x0020)
		{
			// BTN6 pressed			
		}
		if (btns & 0x0040)
		{
			// BTN7 pressed			
		}
		if (btns & 0x0080)
		{			
			goToPage(PAGE_CONFIG);
		}
	}
};
MainPage mainPage;