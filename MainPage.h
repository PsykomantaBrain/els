#pragma once

// pages by index
#define PAGE_MAIN 0
#define PAGE_CONFIG 1
#define PAGE_THREADING 2
#define PAGE_SPEED 3
#define PAGE_JOG 4
#define PAGE_RETURN 5



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
		lcd.print("Spnd Hdwl");


		lcd.setCursor(C_FIELD2, 2);	lcd.print(" X\005:");
		pvDRO.drawAt(lcd, C_FIELD0, 3);

		lcd.setCursor(0, 3);
		lcd.print("      THR  SPD  JOG ");
	}

	void drawLoop() override
	{
		// show RPM value in its region
		pvSpndl.drawAt(lcd, C_FIELD0, 0);
		pvHdWhl.drawAt(lcd, C_FIELD1, 0);

		pvDRO.drawAt(lcd, C_FIELD3, 2);

		//pvBtns.drawAt(lcd, C_FIELD3, 2);

	}
	void pageUpdate(uint16_t btns) override
	{

		// handle inputs for main page if needed

		if (btns & 0x0001)
		{
			// nothing in this page. This is the RTN button in other pages.
			return;
		}
		if (btns & 0x0002)
		{
			// BTN2 pressed			
			goToPage(PAGE_THREADING);
			return;
		}
		if (btns & 0x0004)
		{
			// BTN3 pressed			
			goToPage(PAGE_SPEED);
			return;
		}
		if (btns & 0x0008)
		{
			// BTN4 pressed			
			goToPage(PAGE_JOG);
			return;
		}


		if (btns & 0x0010)
		{
			// BTN5 pressed			
		}
		if (btns & 0x0020)
		{
			// BTN6 pressed			
			return;
		}
		if (btns & 0x0040)
		{
			// BTN7 pressed		
			return;	
		}
		if (btns & 0x0080)
		{			
			goToPage(PAGE_CONFIG);
			return;
		}
	}
};
MainPage mainPage;