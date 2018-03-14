

#include <Wire.h>
#include <LiquidCrystal.h>
#include "Sodaq_DS3231.h"
#include "EmonLib.h"         

#define SAMPLE_COUNT 5
#define VOLT_CAL1 147.7    //AC Voltage Calibration
#define CURRENT_CAL1 40.6 //CT Amp Calibration



//create 2 instances of the energy monitor lib
EnergyMonitor emon1;
//arrays to hold the sample data

double volts1[SAMPLE_COUNT];
double amps1[SAMPLE_COUNT];
double watts1[SAMPLE_COUNT];

const int currentPin1 = A2;
const int voltagePin1 = A3;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
//LCD Use Pins D8,D9,D4,D5,D6,D7 
//Clock Use Pins A4 And A5
//Rain Use Pin  A0
//All pins being used A0,A1,A2,A3,A4,A5,D2,D4,D5,D6,D7,D8,D9,D11,D12,D13
int Relay1 = 2;
int Relay2 = 11;
int Relay3 = 12;
int Relay4 = 13;
int batPen = A1;
int RainV = 0;
int rain = analogRead(A0);
double Volts1;
int readValue;
int readValue1A;
float Voltage;

//counter to keep track of the current sample location
int counter = 0;
bool relaysEnabled[] = { false, false, false, false };


//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
//DateTime dt(2018, 3, 11, 14, 59, 0, 2);//<---need for setting time
DateTime now;
double totalAmp;
double totalWatt;
void setup(void)
{
	Serial.begin(9600);

	Wire.begin();
	rtc.begin();
	// rtc.setDateTime(dt);//<---need for setting time

	emon1.voltage(voltagePin1, VOLT_CAL1, 1.7);  // Voltage: input pin, calibration, phase_shift
	emon1.current(currentPin1, CURRENT_CAL1);       // Current: input pin, calibration.

	lcd.begin(16, 2);

	pinMode(Relay1, OUTPUT);
	pinMode(Relay2, OUTPUT);
	pinMode(Relay3, OUTPUT);
	pinMode(Relay4, OUTPUT);
	pinMode(batPen, INPUT);
	pinMode(rain, INPUT);


}

uint32_t old_ts;
char weekDay[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void loop(void)
{

	now = rtc.now(); //get the current date-time
	uint32_t ts = now.getEpoch();

	if (old_ts == 0 || old_ts != ts) {
		old_ts = ts;
		Serial.print(now.hour(), DEC);
		Serial.print(':');
		Serial.print(now.minute(), DEC);
		Serial.print(':');
		Serial.print(now.second(), DEC);
		Serial.print(' ');
		Serial.print(weekDay[now.dayOfWeek()]);
		Serial.print(' ');
		Serial.print(now.month(), DEC);
		Serial.print('/');
		Serial.print(now.date(), DEC);
		Serial.print('/');
		Serial.print(now.year(), DEC);
		Serial.print(' ');
		Serial.println();



	}
	//reset the var that keeps track of the number of samples taken 
	//(loop back around to 0 on the array for our running total)
	if (counter >= SAMPLE_COUNT) {
		counter = 0;
	}

	//calculate the most recent readings
	emon1.calcVI(20, 2000);

	//save the voltage, current, watts to the array for later averaging

	amps1[counter] = emon1.Irms;
	volts1[counter] = emon1.Vrms;
	watts1[counter] = emon1.Vrms * emon1.Irms;

	counter++;

	//setup the vars to be averaged

	double wattAvg1 = 0;
	double voltAvg1 = 0;
	double ampAvg1 = 0;

	//add em up for averaging
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		wattAvg1 += watts1[i];
		voltAvg1 += volts1[i];
		ampAvg1 += amps1[i];

	}

	//get the final average by dividing by the # of samples
	wattAvg1 /= SAMPLE_COUNT * 4.85;//Adjust Watt Output
	ampAvg1 /= SAMPLE_COUNT;
	voltAvg1 /= SAMPLE_COUNT;

	//calculate the total amps and watts
	totalAmp = ampAvg1;
	totalWatt = wattAvg1 / 4.85;//Adjust Watt Output
								//send the power info to the ESP module through Serial1
	sendPowerInfo(voltAvg1, totalAmp, totalWatt);

}

void setStatusOfRelay(int relay, bool stat)
{
	Serial.print("Relay ");
	if (relay != 0)
	{
		Serial.print(relay);
	}
	else
	{
		Serial.print("ALL");
	}
	Serial.print((stat ? "ON" : "OFF"));
	Serial.print(" | Bat Volt ");
	Serial.print(Voltage);
	Serial.print(" | AC Volts ");
	Serial.print(Volts1);
	Serial.print(" | Amps ");
	Serial.print(totalAmp);
	Serial.print(" | Watts ");
	Serial.print(totalWatt);
	Serial.print(" | RAIN ");
	Serial.println(RainV);


	switch (relay)
	{
	case 0:
		digitalWrite(Relay1, (stat ? HIGH : LOW));
		digitalWrite(Relay2, (stat ? HIGH : LOW));
		digitalWrite(Relay3, (stat ? HIGH : LOW));
		digitalWrite(Relay4, (stat ? HIGH : LOW));
		break;
	case 1:
		digitalWrite(Relay1, (stat ? HIGH : LOW));
		break;
	case 2:
		digitalWrite(Relay2, (stat ? HIGH : LOW));
		break;
	case 3:
		digitalWrite(Relay3, (stat ? HIGH : LOW));
		break;
	case 4:
		digitalWrite(Relay4, (stat ? HIGH : LOW));
		break;
	default:
		break;
	}

	relaysEnabled[relay - 1] = stat;
}

bool isRelayActivated(int relay)
{
	return relaysEnabled[relay - 1];
}
//send the power info to the ESP module through Serial1 (comma separated and starting with *)
void sendPowerInfo(double Volts1g, double Amps, double Watts) {


	readValue1A = analogRead(rain);
	readValue = analogRead(batPen);
	Voltage = (205. / 1023.)*readValue;//DC Voltage Calibration(--->>>205.<<<---/1023.)
	RainV = readValue1A;
	Volts1 = Volts1g;


	//Serial.print("                         ");
	//Serial.print(" | AC Volts ");
	//Serial.println(Volts1);



	if (RainV >= 800)
	{
		if ((now.hour() >= 20 && now.minute() >= 30) or (Voltage < 4))//OFF 20=8pm winter hours
																	  //if((now.hour() >= 20) or (Voltage < 46.8))//OFF 20=8pm winter hours
		{
			{
				setStatusOfRelay(0, false);
			}
		}
		else if (now.hour() >= 6 && now.minute() >= 30)//ON 6=6am winter hours
			if (now.hour() >= 9 && now.minute() >= 30)//OFF 9=9am winter hours
				if (now.hour() >= 15 && now.minute() >= 30)//ON 15=3pm winter hours
				{
					{
						if ((Amps < 8) && (Voltage < 47))

						{
							setStatusOfRelay(1, true);
						}
						else
						{
							setStatusOfRelay(1, false);
						}

						if ((Amps < 7) && (Voltage < 47.5))
						{
							setStatusOfRelay(2, true);
						}
						else
						{
							setStatusOfRelay(2, false);
						}

						if ((Amps < 6) && (Voltage < 48))
						{
							setStatusOfRelay(3, true);
						}
						else
						{
							setStatusOfRelay(3, false);
						}

						if ((Amps < 5) && (Voltage < 48.5))
						{
							setStatusOfRelay(4, true);
						}
						else
						{
							setStatusOfRelay(4, false);
						}
					}
				}

		lcd.begin(16, 2);              // start the library
		lcd.setCursor(0, 0);
		lcd.print(' ');
		lcd.print("POWER WALL"); // print a simple message
		lcd.print(' ');
		lcd.print(weekDay[now.dayOfWeek()]); // print a simple message
		lcd.setCursor(0, 1);
		lcd.print(now.hour(), DEC);
		lcd.print(":");
		lcd.print(now.minute(), DEC);
		//lcd.print(":");
		//lcd.print(now.second(), DEC);
		lcd.print(' ');
		lcd.print(now.month(), DEC);
		lcd.print('/');
		lcd.print(now.date(), DEC);
		lcd.print('/');
		lcd.print(now.year(), DEC);
		delay(2000);

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(' ');
		lcd.print("Bat Volt"); // print a simple message
		lcd.print(' ');
		lcd.print(Voltage);
		lcd.setCursor(0, 1);
		lcd.print("InverterAmp");
		lcd.print(' ');
		lcd.print(Amps);
		delay(2000);

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(' ');
		lcd.print(' ');
		lcd.print("Watts"); // print a simple message
		lcd.print(' ');
		lcd.print(Watts);
		lcd.setCursor(0, 1);
		lcd.print("AC Volts");
		lcd.print(' ');
		lcd.print(Volts1);
		lcd.print(' ');

		delay(2000);

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Relay ");
		lcd.setCursor(6, 0);
		lcd.print("1");
		lcd.setCursor(11, 0);
		lcd.print("2");

		lcd.setCursor(0, 1);
		lcd.print("Relay");
		lcd.setCursor(6, 1);
		lcd.print("3");
		lcd.setCursor(11, 1);
		lcd.print("4");

		if (isRelayActivated(1)) {
			lcd.setCursor(8, 0);
			lcd.print("ON");
		}
		else {
			lcd.setCursor(8, 0);
			lcd.print("OFF");
		}
		if (isRelayActivated(2)) {
			lcd.setCursor(13, 0);
			lcd.print("ON");
		}
		else {
			lcd.setCursor(13, 0);
			lcd.print("OFF");
		}
		if (isRelayActivated(3)) {
			lcd.setCursor(8, 1);
			lcd.print("ON");
		}
		else {
			lcd.setCursor(8, 1);
			lcd.print("OFF");
		}
		if (isRelayActivated(4)) {
			lcd.setCursor(13, 1);
			lcd.print("ON");
		}
		else {
			lcd.setCursor(13, 1);
			lcd.print("OFF");
		}
		delay(4000);
		//delay(60000); 
	}
}
