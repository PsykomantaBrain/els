#pragma once


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
		if (input.startsWith("/btn"))
		{
			char btn = input.charAt(5);

			switch (btn)
			{
				case '1':
					btnsRxState |= 0x0001;
					break;
				case '2':
					btnsRxState |= 0x0002;
					break;
				case '3':
					btnsRxState |= 0x0004;
					break;
				case '4':
					btnsRxState |= 0x0008;
					break;
				case '5':
					btnsRxState |= 0x0010;
					break;
				case '6':
					btnsRxState |= 0x0020;
					break;
				case '7':
					btnsRxState |= 0x0040;
					break;
				case '8':
					btnsRxState |= 0x0080;
					break;


				case 'Z':
				case 'z':
				case '0':
					btnsRxState |= 0x0100;
					break;

				case 'R':
				case 'r':
					btnsRxState |= 0x1000;
					break;

				case 'S':
				case 's':
				case ' ':
					btnsRxState |= 0x2000;
					break;


				default:
				btnsRxState = 0;
				break;
			}
		}

		if (input.startsWith("/hwl"))
		{
			int mov = input.substring(5).toInt();
			mov_handwheel(mov);
		}
	}
}

