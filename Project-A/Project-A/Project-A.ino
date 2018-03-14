/*
 Name:		Project_A.ino
 Created:	3/13/2018 6:29:22 PM
 Author:	Brandon Smith, and Ray Smith
*/

#include "lib\threads\Thread.h"
#include "lib\lcd\LiquidCrystal.h"
#include <Wire\src\Wire.h>

Thread* lcdThread;
LiquidCrystal lcd;

int lcdCounter = -1;

int batteryVoltage = 0;
int batteryAC = 0;

// the setup function runs once when you press reset or power the board
void setup() 
{
	Serial.begin(9600);
	// Initialize instance data
	lcdThread = new Thread();
	lcd = LiquidCrystal(12, 11, 5, 4, 3, 2);
	lcd.begin(16, 2);

	// Setup lcdThread
	lcdThread->setInterval(5000);
	lcdThread->enabled = true;
	lcdThread->onRun(onLCDUpdate);
	if (lcdThread->shouldRun())
	{
		lcdThread->run();
	}
	else
	{
		Serial.println("[Chip Thread | Critical Error]: Failed to open LCD Thread.");
	}
}

// the loop function runs over and over again until power down or reset
void loop() 
{
  
}


void onLCDUpdate()
{
	if (lcdCounter == -1)
	{
		lcd.setCursor(0, 0);
		lcd.println("		Board Initializing...");
		delay(500);
		lcd.clear();

		lcdCounter++;
		onLCDUpdate();
		return;
	}

	if (lcdCounter == 0)
	{

	}

	lcdCounter++;
	if (lcdCounter >= 5)
	{
		lcdCounter = 0;
	}
}