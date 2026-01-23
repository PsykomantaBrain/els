#pragma once

struct ArmingButton
{


private:
	uint8_t pinBtn;
	uint8_t pinLed;

	bool armed = 0;
	
	// Callback function to be called on press
	void (*onPress)() = nullptr;

public:

	//ctor
	ArmingButton(uint8_t btnPin, uint8_t ledPin) : pinBtn(btnPin), pinLed(ledPin)
	{
		pinMode(pinBtn, INPUT_PULLUP);
		pinMode(pinLed, OUTPUT);
		digitalWrite(pinLed, LOW);
	}
	~ArmingButton() {}

	void arm(void (*callback)())	
	{
		armed = true;
		onPress = callback;
		digitalWrite(pinLed, HIGH);
	}
	void disarm()
	{
		armed = false;
		onPress = nullptr;
		digitalWrite(pinLed, LOW);
	}

	bool inputUpdate()
	{
		if (armed && digitalRead(pinBtn) == LOW)
		{
			armed = false; // disarm after press
			digitalWrite(pinLed, LOW);
			
			if (onPress != nullptr)
			{
				onPress();
			}
			return true;
		}
		return false;
	}

};
