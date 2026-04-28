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
		lcd.print(" THR  SPD  JOG  CFG ");
				


		lcd.setCursor(C_FIELD0, 2);	lcd.print("\010\010");
		//lcd.setCursor(C_FIELD2, 2);	lcd.print("Spnd Hdwl");
		
		
	}

	void drawLoop() override
	{
		pvDRO.drawAt(lcd, C_FIELD0, 3);

		// show RPM value in its region ?
		//pvSpndl.drawAt(lcd, C_FIELD2, 3);
		//pvHdWhl.drawAt(lcd, C_FIELD3, 3);

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
			return;
		}
		if (btns & 0x0004)
		{
			// BTN3 pressed		
			return;	
		}
		if (btns & 0x0008)
		{
			// BTN4 pressed			
			return;
		}


		if (btns & 0x0010)
		{
			// BTN5 pressed			
			goToPage(PAGE_THREADING);
			return;
		}
		if (btns & 0x0020)
		{
			// BTN6 pressed			
			goToPage(PAGE_SPEED);
			return;			
		}
		if (btns & 0x0040)
		{
			// BTN7 pressed		
			goToPage(PAGE_JOG);
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