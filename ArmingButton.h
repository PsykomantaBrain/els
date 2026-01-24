#pragma once

struct ArmingButton
{


private:
	uint8_t pinBtn;
	uint8_t pinLed;

	bool armed = 0;
	

public:

	bool IsArmed()
	{
		return armed;
	}

	//ctor
	ArmingButton(uint8_t btnPin, uint8_t ledPin) : pinBtn(btnPin), pinLed(ledPin)
	{
		pinMode(pinBtn, INPUT_PULLUP);
		pinMode(pinLed, OUTPUT);
		digitalWrite(pinLed, LOW);
	}
	~ArmingButton() {}

	void arm()
	{
		armed = true;		
		digitalWrite(pinLed, HIGH);
	}
	void disarm()
	{
		armed = false;		
		digitalWrite(pinLed, LOW);
	}

	bool inputUpdate()
	{
		if (armed && digitalRead(pinBtn) == LOW)
		{	
			return true;
		}
		return false;
	}

};
