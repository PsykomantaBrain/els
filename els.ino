/*
 Name:		els.ino
 Created:	1/18/2026 2:01:02 PM
 Author:	HarvesteR
*/

// the setup function runs once when you press reset or power the board
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define D0 16

#define I2C_SCL 5
#define I2C_SDA 4

#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

//initialize the liquid crystal library
//the first parameter is the I2C address
//the second parameter is how many rows are on your screen
//the third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27, 20, 4);
byte cc0[8] = {
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
	B00100,
};
byte cc1[8] = {
	B00001,
	B00001,
	B00001,
	B00001,    
	B00001,
	B00001,
	B00001,
	B00001,
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
	B00011,
	B00011,
	B00011,
	B00011,
	B00011,
	B00011,    
	B00011,
	B00011,    
};
byte cc4[8] = {
	B11000,
	B11000,
	B11000,
	B11000,
	B11000,
	B11000,
	B11000,
	B11000,
};


uint8_t page = 0;




void setup() 
{
	// init usb serial for controlling
	Serial.begin(115200);
	
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
	lcd.print(" HRV ELS ");
	lcd.setCursor(0, 2);
	lcd.print("  \x08\x08\x03\x03\x01\x04\x02\x03\x03\x08\x08  ");


	delay(2000);
	setPage(0);

	Serial.println("ready");
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	// Handle serial data if there is anything in the buffer. 
	handleSerial();
	
	
	
}


void setPage(uint8_t newPage)
{
	page = newPage;
	drawPage(page);
}

void drawPage(uint8_t page)
{
	switch (page)
	{
		default:
		case 0:
			// home page
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print(" RPM ");
			lcd.print(0000);
			lcd.setCursor(0, 3);
			lcd.print(" CFG  THR  SPD  JOG ");
			break;

		case 1:
			// cfg page
			lcd.clear();

			lcd.setCursor(0, 2);
			lcd.print(parseChars("\x0001CFG\x0002 000  000      "));
			lcd.setCursor(0, 3);
			lcd.print(parseChars("\x0001CFG\x0002 CAL  LDS  MSR "));
			break;

		case 2:
			// thrd page
			lcd.clear();

			lcd.print(" PCH  LEN  ");
			lcd.setCursor(0, 1);
			lcd.print("  M3  042  ");
			lcd.setCursor(0, 2);


			lcd.setCursor(0, 3);
			lcd.print("\x0001THR\x0002 PDT  END  JOG ");
			break;
	}
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
			setPage(newPage);
			Serial.print("Page set to ");
			Serial.println(newPage);
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



