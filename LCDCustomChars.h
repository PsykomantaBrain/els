#pragma once
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

void addLCDCustomChars(LiquidCrystal_I2C& lcd)
{
	lcd.createChar(0, cc0);
	lcd.createChar(1, cc1);
	lcd.createChar(2, cc2);
	lcd.createChar(3, cc3);
	lcd.createChar(4, cc4);
}