#pragma once
//byte cc0[8] = { // cc0 doesn't work
//	B00000,
//	B00000,
//	B00000,
//	B00000,
//	B00000,
//	B00000,
//	B00000,
//	B00000,
//};
byte cc1[8] = {
	B00000,
	B00110,
	B00110,
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
// cc5: stacked "mm" — unit glyph for millimeters.
// Two 3-row lowercase m's, one on top of the other, with a 1-row gap.
byte cc5[8] = {
	B11011,
	B10101,
	B10101,
	B00000,
	B11011,
	B10101,
	B10101,
	B00000,
};
// cc6: double forward-slash — left-handed thread indicator.
// Two 2-pixel-thick diagonals running bottom-left to top-right, stacked vertically.
byte cc6[8] = {
	B00011,
	B00011,
	B00110,
	B00110,
	B01100,
	B01100,
	B11000,
	B11000,
};
// cc7: double back-slash — right-handed thread indicator.
// Mirror of cc6 — diagonals running top-left to bottom-right.
byte cc7[8] = {
	B11000,
	B11000,
	B01100,
	B01100,
	B00110,
	B00110,
	B00011,
	B00011,
};
// cc8: single small m
byte cc8[8] = {
	B00000,
	B00000,
	B00000,
	B00000,
	B11011,
	B10101,
	B10101,
	B00000,
};
// cc9: single small micron
byte cc9[8] = {
	B00000,
	B00000,
	B00000,
	B00000,
	B01010,
	B01010,
	B01101,
	B00000,
};

void addLCDCustomChars(LiquidCrystal_I2C& lcd)
{
	//lcd.createChar(0, cc0); // cc0 seems to not work.
	lcd.createChar(1, cc1);
	lcd.createChar(2, cc2);
	lcd.createChar(3, cc3);
	lcd.createChar(4, cc4);
	lcd.createChar(5, cc5);
	lcd.createChar(6, cc6);
	lcd.createChar(7, cc7);
	lcd.createChar(8, cc8);
	lcd.createChar(9, cc9);


}

void testLCDCustomChars(LiquidCrystal_I2C& lcd)
{
	lcd.print("\001\002\003\004\005\006\007    ");
}