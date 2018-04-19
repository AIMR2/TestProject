#include <Thread.h>
#include <ThreadController.h>

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "Sodaq_DS3231.h"
#include "EmonLib.h"


#pragma region Default Admin Configuration
#define DC_VOLT1 228.0
#define DC_VOLT2 228.0
#define VOLT_CAL1 220.0    //voltage calibration
#define VOLT_CAL2 220.0    //voltage calibration
#define CURRENT_CAL1 43.6 //sensor 1 calibration,CT1,I Amps
#define CURRENT_CAL2 42.6 //sensor 2 calibration,CT2,I Amps

#define CURRENT_CAL3 43.6 //sensor 3 calibration,CT3,H Amps
#define CURRENT_CAL4 42.6 //sensor 4 calibration,CT4,H Amps
#pragma endregion

#pragma region Developer Configuration
String _VERSION = "2.0";
int pageSwitchSeconds = 3;
#define UpdateDate false
#define IsDebugMode false

#define SAMPLE1_COUNT 5
#define SAMPLE_COUNT 5

#define LOWEST_DC_VOLT1 100
#define LOWEST_DC_VOLT2 100

#define LOWEST_VOLTAGE1 100
#define HIGHEST_VOLTAGE1 250
#define LOWEST_VOLTAGE2 100
#define HIGHEST_VOLTAGE2 250

#define LOWEST_AMPS1 0
#define HIGHEST_AMPS1 50
#define LOWEST_AMPS2 0
#define HIGHEST_AMPS2 50
#define LOWEST_AMPS3 0
#define HIGHEST_AMPS3 50
#define LOWEST_AMPS4 0
#define HIGHEST_AMPS4 50

#define LOWEST_VOLTS1 0
#define HIGHEST_VOLTS1 100
#define LOWEST_VOLTS2 0
#define HIGHEST_VOLTS2 100
#define LOWEST_VOLTS3 0
#define HIGHEST_VOLTS3 100
#define LOWEST_VOLTS4 0
#define HIGHEST_VOLTS4 100

#define LOWEST_CURRENT1 0
#define HIGHEST_CURRENT1 100
#define LOWEST_CURRENT2 0
#define HIGHEST_CURRENT2 100
#define LOWEST_CURRENT3 0
#define HIGHEST_CURRENT3 100
#define LOWEST_CURRENT4 0
#define HIGHEST_CURRENT4 100

#define LOWEST_START_TIME 0
#define HIGHEST_START_TIME 24
#define LOWEST_STOP_TIME 0
#define HIGHEST_STOP_TIME 24

#pragma endregion

#pragma region Threading Instance Data

// ThreadController that will controll all threads
ThreadController controll = ThreadController();

// Controls the LCD Thread
Thread* lcdThread = new Thread();
Thread* lcdRenderThread = new Thread();
Thread* lcdPagerThread = new Thread();
Thread* calculationThread = new Thread();
#pragma endregion

#pragma region PIN Instance Data

const int currentPin1 = A15;//A2
const int currentPin2 = A14;//A1
const int voltagePin2 = A8;//A3
const int voltagePin1 = A7;//A0
const int currentPin3 = A13;//A8
const int currentPin4 = A12;//A9

int Relay1 = 35;//22
int Relay2 = 37;//24
int Relay3 = 39;//26
int Relay4 = 41;//28
int batPen1 = A10;
int batPen2 = A11;
float rainValue = A6;//A7
float CloudsValue = A9;//A6
					   //CLOCK A5,A4
					   //23,25,27,29,31 buttons           
#pragma endregion

#pragma region Other Instance Data

float Voltage1;
float Voltage2;
double Volts1;
double Volts2;
int readValueB1;
int readValueB2;
int readValueC;
int readValueR;
int RainV = 0;
int CloudsV = 0;

double Amps = 0;
//double VoltsL1;
//double VoltsL2;// double Amps, double Watts, double Watts2
double totalAmp;
double totalAmp2;
double totalWatt1;
double totalWatt2;
// counter to keep track of the current sample location
int counter = 0;
int counter1 = 0;
bool relaysEnabled[] = { false, false, false, false };

#pragma endregion

#pragma region DateTime Instance Data
// year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
// writing any non-existent time-data may interfere with normal operation of the RTC.
// Take care of week-day also.

#if UpdateDate 
DateTime dt(2018, 3, 29, 8, 39, 0, 4);
#endif

DateTime* now;

//uint32_t old_ts;
char weekDay[][4] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

#pragma endregion
//The Ethernet shield allows you to connect a WizNet Ethernet controller to the Arduino or Genuino boards via the SPI bus.
//It uses pins 10, 11, 12, and 13 for the SPI connection to the WizNet. Later models of the Ethernet shield also have an SD 
//Card on board. Digital pin 4 is used to control the slave select pin on the SD card. 
#pragma region Ethernet Instance Data
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
	0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//IPAddress ip(192, 168, 0, 30);
IPAddress ip(76, 80, 34, 235);
//76.80.34.235/
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
#pragma endregion

#pragma region LCD Instance Data

// Initiailze the LCD.
//LiquidCrystal lcd(36, 34, 44, 42, 40, 38);
LiquidCrystal lcd(8, 9, 2, 5, 6, 7);
//rs = 36, en = 34, d4 = 44, d5 = 42, d6 = 40, d7 = 38;
//rs = 8, en = 9, d4 = 2, d5 = 5, d6 = 6, d7 = 7;
//LiquidCrystal lcd(8,9,4,5,6,7);
const int numOfInputs = 5;

const int inputPins[numOfInputs] = { 23,25,27,29,31 };//(23,greenB),(25,yellowF),(27,pink-) (29,blue+) (31jumper)(white is ground common)
int inputState[numOfInputs];
int lastInputState[numOfInputs] = { LOW,LOW,LOW,LOW };
bool inputFlags[numOfInputs] = { LOW,LOW,LOW,LOW };
long lastDebounceTime[numOfInputs] = { 0,0,0,0 };
long debounceDelay = 0;

// LCD Menu Logic
const int numOfScreens = 27;
int currentScreen = 0;
String screens[numOfScreens][2] = {
	{ "Bat1 Volts",     "VDC" },
{ "Bat2 Volts",     "VDC" },
{ "CT1 Calibrate",    "Amps" },
{ "CT2 Calibrate",    "Amps" },
{ "CT3 Calibrate",    "Amps" },
{ "CT4 Calibrate",    "Amps" },
{ "L1 AC Calibrate",  "VAC" },
{ "L2 AC Calibrate",  "VAC" },
{ "Relay 1 Volts",    "Volts" },
{ "Relay 2 Volts",    "Volts" },
{ "Relay 3 Volts",    "Volts" },
{ "Relay 4 Volts",    "Volts" },
{ "Relay 1 Amps",   "Amps" },
{ "Relay 2 Amps",   "Amps" },
{ "Relay 3 Amps",   "Amps" },
{ "Relay 4 Amps",   "Amps" },
{ "Stop Time",      "Hours" },
{ "Start Time",     "Hours" },
{ "Rain Calibration", "" },
{ "Cloud Calibration",  "" },
{ "Month",        "" },
{ "Day",        "" },
{ "Week Day",     "" },
{ "Year",       "" },
{ "Hours",        "" },
{ "Minutes",      "" },
{ "Seconds",      "" }
};
int parameters[numOfScreens];


const int numOfScreens_nonAdmin = 7;
int currentScreen_nonAdmin = 0;
String screens_nonAdmin[numOfScreens][2] = {
	{ "   Power Wall", "" }, // BLANK, and BLANK
{ "", "" }, // TIME, and DATE
{ "Bat Volt", "Watts" }, // Battery Voltage, and Inverter Amps
{ "I Amps", "H Amps" }, // Watts, and BLANK
{ "L1 Volts", "L2 Volts" }, // L1, L2
{ "Relay 1","Relay 2" }, // Relay 1 (ON OR OFF), Relay 2 (ON OR OFF)
{ "Relay 3","Relay 4" } // Relay 3(ON OR OFF), Relay 4 (ON OR OFF)
};

String parameters_nonAdmin[numOfScreens_nonAdmin][2];

boolean isInAdministratorMode = true;
#pragma endregion

#pragma region Energy Monitor Instance Data

//create 2 instances of the energy monitor lib
EnergyMonitor emon1;
EnergyMonitor emon2;

EnergyMonitor emon3;
EnergyMonitor emon4;

//arrays to hold the sample data
double volts1[SAMPLE_COUNT];
double amps1[SAMPLE_COUNT];
double watts1[SAMPLE_COUNT];

double volts2[SAMPLE_COUNT];
double amps2[SAMPLE_COUNT];
double watts2[SAMPLE_COUNT];

double amps3[SAMPLE1_COUNT];
double amps4[SAMPLE1_COUNT];

#pragma endregion

#pragma region Threading Callback Functions

// callback for myThread
void updateLCD()
{
}

void pagerLCD()
{

	if (!isInAdministratorMode)
	{
		currentScreen_nonAdmin++;
		if (currentScreen_nonAdmin >= numOfScreens_nonAdmin)
		{
			currentScreen_nonAdmin = 0;
		}

	}
}

void renderLCD()
{
	if (!isInAdministratorMode)
	{
		for (auto i = 0; i < numOfScreens_nonAdmin; i++)
		{
			switch (i)
			{

			case 0:
				parameters_nonAdmin[i][1] = "V" + _VERSION;
				break;
			case 1:
				parameters_nonAdmin[i][0] = (String)(weekDay[now->dayOfWeek() - 1]) +
					" " +
					String(now->hour()) + ":" + String(now->minute()) + ":" + String(now->second());

				parameters_nonAdmin[i][1] = String(now->month()) + "/" + String(now->date()) + "/" + String(now->year());
				break;
			case 2:
				parameters_nonAdmin[i][0] = Voltage1; // Battery Voltage
				parameters_nonAdmin[i][1] = totalWatt1; // Total Watts
				break;
			case 3:
				parameters_nonAdmin[i][0] = totalAmp; // Inverter Amps
				parameters_nonAdmin[i][1] = totalAmp2; // House Amps
				break;
			case 4:
				parameters_nonAdmin[i][0] = Volts1; // L1 AC Voltages
				parameters_nonAdmin[i][1] = Volts2; // L2 AC Voltages
				break;
			case 5:
				parameters_nonAdmin[i][0] = (isRelayActivated(1) ? "ON" : "OFF"); // Relay 1
				parameters_nonAdmin[i][1] = (isRelayActivated(2) ? "ON" : "OFF"); // Relay 2
				break;
			case 6:
				parameters_nonAdmin[i][0] = (isRelayActivated(3) ? "ON" : "OFF"); // Relay 3
				parameters_nonAdmin[i][1] = (isRelayActivated(4) ? "ON" : "OFF"); // Relay 4
				break;
			default:
				parameters_nonAdmin[i][0] = ""; // BLANKS
				parameters_nonAdmin[i][1] = ""; // BLANKS
				break;
			}
		}
	}

	printScreen();
}

void updateNetwork()
{
	// HTTP Client stuff.
}

void runCalculations()
{
	now = &rtc.now(); //get the current date-time

	Serial.print(now->hour(), DEC);
	Serial.print(':');
	Serial.print(now->minute(), DEC);
	Serial.print(':');
	Serial.print(now->second(), DEC);
	Serial.print(' ');
	Serial.print(weekDay[now->dayOfWeek() - 1]);
	Serial.print(' ');
	Serial.print(now->month(), DEC);
	Serial.print('/');
	Serial.print(now->date(), DEC);
	Serial.print('/');
	Serial.print(now->year(), DEC);
	Serial.print(' ');

	Serial.println();
	//}

	// reset the var that keeps track of the number of samples taken 
	// (loop back around to 0 on the array for our running total)
	if (counter >= SAMPLE_COUNT)
	{
		counter = 0;
	}
	if (counter1 >= SAMPLE1_COUNT)
	{
		counter1 = 0;
	}

	// calculate the most recent readings
	emon1.calcVI(20, 2000);
	emon2.calcVI(20, 2000);
	emon3.calcVI(20, 2000);
	emon4.calcVI(20, 2000);

	// save the voltage, current, watts to the array for later averaging
	amps1[counter] = emon1.Irms;
	volts1[counter] = emon1.Vrms;
	watts1[counter] = emon1.Vrms * emon1.Irms * 2;

	amps2[counter] = emon2.Irms;
	volts2[counter] = emon2.Vrms;
	watts2[counter] = emon2.Vrms * emon2.Irms * 2;

	amps3[counter1] = emon3.Irms;
	amps4[counter1] = emon4.Irms;

	counter1++;
	counter++;

	// setup the vars to be averaged
	double wattAvg1 = 0;
	double voltAvg1 = 0;
	double ampAvg1 = 0;

	double wattAvg2 = 0;
	double voltAvg2 = 0;
	double ampAvg2 = 0;

	double ampAvg3 = 0;
	double ampAvg4 = 0;

	// add em up for averaging
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		for (int i = 0; i < SAMPLE1_COUNT; i++)
		{
			wattAvg1 += watts1[i];
			voltAvg1 += volts1[i];
			ampAvg1 += amps1[i];

			wattAvg2 += watts2[i];
			voltAvg2 += volts2[i];
			ampAvg2 += amps2[i];

			ampAvg3 += amps3[i];
			ampAvg4 += amps4[i];
		}
	}

	// get the final average by dividing by the # of samples
	wattAvg1 /= SAMPLE_COUNT;
	ampAvg1 /= SAMPLE_COUNT;
	voltAvg1 /= SAMPLE_COUNT;

	wattAvg2 /= SAMPLE_COUNT;
	ampAvg2 /= SAMPLE_COUNT;
	voltAvg2 /= SAMPLE_COUNT;

	ampAvg3 /= SAMPLE1_COUNT;
	ampAvg4 /= SAMPLE1_COUNT;


	// calculate the total amps and watts
	totalAmp = ampAvg1 + ampAvg2;
	totalAmp2 = ampAvg3 + ampAvg4;

	totalWatt1 = wattAvg1;
	totalWatt2 = wattAvg2;
	// double totalvolt = voltAvg1 + voltAvg2;
	// send the power info to the ESP module through Serial1


	sendPowerInfo(voltAvg1, voltAvg2, totalAmp, totalWatt1, totalWatt2);
}
#pragma endregion

void setup() {
	Serial.begin(9600);
	while (!Serial);
	Wire.begin();
	rtc.begin();
	now = &rtc.now(); //get the current date-time

#pragma region Initialize Parameter Values
	parameters[0] = DC_VOLT1;
	parameters[1] = DC_VOLT2;
	parameters[2] = CURRENT_CAL1;
	parameters[3] = CURRENT_CAL2;
	parameters[4] = CURRENT_CAL3;
	parameters[5] = CURRENT_CAL4;
	parameters[6] = VOLT_CAL1;
	parameters[7] = VOLT_CAL2;
	parameters[8] = 4;
	parameters[9] = 4;
	parameters[10] = 46;
	parameters[11] = 46;
	parameters[12] = 8;
	parameters[13] = 8;
	parameters[14] = 8;
	parameters[15] = 8;
	parameters[16] = 20;
	parameters[17] = 6;
	parameters[18] = 200;
	parameters[19] = 1200;
	parameters[20] = now->month();
	parameters[21] = now->date();
	parameters[22] = now->dayOfWeek();
	parameters[23] = now->year();
	parameters[24] = now->hour();
	parameters[25] = now->minute();
	parameters[26] = now->second();
#pragma endregion


#pragma region Initialize Energy Monitors

	emon1.voltage(voltagePin1, parameters[6], 1.7);  // Voltage: input pin, calibration, phase_shift
	emon1.current(currentPin1, parameters[2]);       // Current: input pin, calibration.

	emon2.voltage(voltagePin2, parameters[7], 1.7);  // Voltage: input pin, calibration, phase_shift
	emon2.current(currentPin2, parameters[3]);       // Current: input pin, calibration.

	emon3.current(currentPin3, parameters[4]);       // Current: input pin, calibration.
	emon4.current(currentPin4, parameters[5]);       // Current: input pin, calibration.

#pragma endregion

#pragma region Initialize PINs
	pinMode(Relay1, OUTPUT);
	pinMode(Relay2, OUTPUT);
	pinMode(Relay3, OUTPUT);
	pinMode(Relay4, OUTPUT);
	pinMode(batPen1, INPUT);
	pinMode(batPen2, INPUT);
	pinMode(RainV, INPUT);
	pinMode(CloudsV, INPUT);
	pinMode(4, OUTPUT);
	digitalWrite(4, HIGH);
	for (int i = 0; i < numOfInputs; i++) {
		pinMode(inputPins[i], INPUT);
		digitalWrite(inputPins[i], HIGH); // pull-up 20k
	}
#pragma endregion

	lcd.begin(16, 2);

#if UpdateDate
	rtc.setDateTime(dt);
#endif

#pragma region Initialize Ethernet
	Ethernet.begin(mac, ip);
	server.begin();
	Serial.print("server is at ");
	Serial.println(Ethernet.localIP());
#pragma endregion

	// Do not change anything here.
#pragma region Initialize Threading
	// Configure myThread
	lcdThread->onRun(updateLCD);
	lcdThread->setInterval(50);
	lcdRenderThread->onRun(renderLCD);
	lcdRenderThread->setInterval(250);
	lcdPagerThread->onRun(pagerLCD);
	lcdPagerThread->setInterval(pageSwitchSeconds * 1000);

	calculationThread->onRun(runCalculations);
	calculationThread->setInterval(500);

	controll.add(lcdThread);
	controll.add(lcdRenderThread);
	controll.add(lcdPagerThread);
	controll.add(calculationThread);
#pragma endregion


}

// Update the Threads, nothing else.
void loop()
{
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) {
		Serial.println("new client");
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");  // the connection will be closed after completion of the response
					client.println("Refresh: 5");  // refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					client.println("<body bgcolor=black text=lime link=gold vlink=gold alink=gold>");
					// output the value of each analog input pin
					for (int analogChannel = 0; analogChannel < 1; analogChannel++) {
						// client.println("<br />");
						now = &rtc.now(); //get the current date-time
						client.print(now->hour(), DEC);
						client.print(':');
						client.print(now->minute(), DEC);
						client.print(':');
						client.print(now->second(), DEC);
						client.print(' ');
						client.print(weekDay[now->dayOfWeek() - 1]);
						client.print(' ');
						client.print(now->month(), DEC);
						client.print('/');

						client.print(now->date(), DEC);
						client.print('/');

						client.print(now->year(), DEC);

						client.println(' ');
						client.println(' ');
						client.print(" | Bat1 Volt ");
						client.print(Voltage1);
						client.print(" | Bat2 Volt ");
						client.print(Voltage2);
						client.print(" | I_Amps ");
						client.print(totalAmp);
						client.print(" | H_Amps ");
						client.print(totalAmp2);
						client.print(" | Watts1 ");
						client.print(totalWatt1);
						client.print(" | Watts2 ");
						client.print(totalWatt2);
						client.print(" | Clouds ");
						client.print(CloudsV);
						client.print(" | Rain ");
						client.print(RainV);
						client.print(" | AC L1 ");
						client.print(Volts1);
						client.print(" | AC L2 ");
						client.println(Volts2);
						client.println("<br />");
						client.println(' ');

						client.print(" | DC_VOLT1 ");
						client.print(parameters[0]);
						client.print(" | DC_VOLT2 ");
						client.print(parameters[1]);
						client.print(" | CURRENT_CAL1 ");
						client.print(parameters[2]);
						client.print(" | CURRENT_CAL2 ");
						client.print(parameters[3]);
						client.print(" | CURRENT_CAL3 ");
						client.print(parameters[4]);
						client.print(" | CURRENT_CAL4 ");
						client.print(parameters[5]);
						client.print(" |VOLT_CAL1 ");
						client.print(parameters[6]);
						client.println(" | VOLT_CAL2 ");
						client.println(' ');
						client.println(parameters[7]);
						client.print(" | r1 volt ");
						client.print(parameters[8]);
						client.print(" | r2 volt ");
						client.print(parameters[9]);
						client.print(" | r3 volt ");
						client.print(parameters[10]);
						client.print(" | r4 volt ");
						client.print(parameters[11]);
						client.print(" | r1 amp ");
						client.print(parameters[12]);
						client.print(" | r2 amp ");
						client.print(parameters[13]);
						client.print(" | r3 amp ");
						client.print(parameters[14]);
						client.print(" | r4 amp ");
						client.print(parameters[15]);
						client.print(" | stop time ");
						client.print(parameters[16]);
						client.print(" |start time ");
						client.print(parameters[17]);
						client.print(" | clouds ");
						client.print(parameters[18]);
						client.print(" | rain ");
						client.println(' ');
						client.println(parameters[19]);
						client.print(" | month ");
						client.print(parameters[20]);
						client.print(" | date ");
						client.print(parameters[21]);
						client.print(" | dayOfWeek ");
						client.print(parameters[22]);
						client.print(" | year ");
						client.print(parameters[23]);
						client.print(" | hour ");
						client.print(parameters[24]);
						client.print(" | minute ");
						client.print(parameters[25]);
						client.print(" | second ");
						client.println(parameters[26]);

					}
					client.println("</html>");
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
				}
				else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
		Serial.println("client disconnected");
	}
	//} 
	controll.run();

	setInputFlags();
	resolveInputFlags();
}

#pragma region Helper Functions

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

	Serial.print((stat ? " ON" : " OFF"));
	Serial.print(" | Bat1 Volt ");
	Serial.print(Voltage1);
	Serial.print(" | Bat2 Volt ");
	Serial.print(Voltage2);
	Serial.print(" | I_Amps ");
	Serial.print(totalAmp);
	Serial.print(" | H_Amps ");
	Serial.print(totalAmp2);
	Serial.print(" | Watts1 ");
	Serial.print(totalWatt1);
	Serial.print(" | Watts2 ");
	Serial.print(totalWatt2);
	Serial.print(" | Clouds ");
	Serial.print(CloudsV);
	Serial.print(" | Rain ");
	Serial.print(RainV);
	Serial.print(" | AC L1 ");
	Serial.print(Volts1);
	Serial.print(" | AC L2 ");
	Serial.println(Volts2);

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
#pragma endregion

#pragma region LCD Functions

void setInputFlags() {
	for (int i = 0; i < numOfInputs; i++)
	{
		int reading = digitalRead(inputPins[i]);
		if (reading != lastInputState[i])
		{
			lastDebounceTime[i] = millis();
		}

		if ((millis() - lastDebounceTime[i]) > debounceDelay)
		{
			if (reading != inputState[i])
			{
				inputState[i] = reading;
				if (inputState[i] == HIGH)
				{
					inputFlags[i] = HIGH;
				}
			}
		}
		lastInputState[i] = reading;
	}
}

void resolveInputFlags()
{
	for (int i = 0; i < numOfInputs; i++)
	{

		if (inputFlags[4] == HIGH)
		{
			inputFlags[4] = LOW;
			isInAdministratorMode = !isInAdministratorMode;

			Serial.print("Administrator Button was Pressed: ");
			Serial.println(isInAdministratorMode ? "ACTIVATED" : "DEACTIVATED");

			currentScreen_nonAdmin = 0;

			printScreen();

			break;
		}

		if (inputFlags[i] == HIGH && isInAdministratorMode)
		{
			inputAction(i);
			inputFlags[i] = LOW;
			printScreen();
		}
	}
}

void inputAction(int input)
{
	if (input == 0)
	{
		if (currentScreen == 0)
		{
			currentScreen = numOfScreens - 1;
		}
		else {
			currentScreen--;
		}

		parameters[20] = now->month();
		parameters[21] = now->date();
		parameters[22] = now->dayOfWeek();
		parameters[23] = now->year();
		parameters[24] = now->hour();
		parameters[25] = now->minute();
		parameters[26] = now->second();
	}
	else if (input == 1)
	{
		if (currentScreen == numOfScreens - 1)
		{
			currentScreen = 0;
		}
		else {
			currentScreen++;
		}

		parameters[20] = now->month();
		parameters[21] = now->date();
		parameters[22] = now->dayOfWeek();
		parameters[23] = now->year();
		parameters[24] = now->hour();
		parameters[25] = now->minute();
		parameters[26] = now->second();
	}
	else if (input == 2)
	{
		parameterChange(0);
	}
	else if (input == 3)
	{
		parameterChange(1);
	}
}

void parameterChange(int key)
{
	if (key == 0)
	{
		parameters[currentScreen]++;
	}
	else if (key == 1)
	{
		parameters[currentScreen]--;
	}

	if (parameters[0] < LOWEST_DC_VOLT1)
		parameters[0] = LOWEST_DC_VOLT1;

	if (parameters[1] < LOWEST_DC_VOLT2)
		parameters[1] = LOWEST_DC_VOLT2;

	if (parameters[2] < LOWEST_CURRENT1)
		parameters[2] = LOWEST_CURRENT1;
	else if (parameters[2] > HIGHEST_CURRENT1)
		parameters[2] = HIGHEST_CURRENT1;

	if (parameters[3] < LOWEST_CURRENT2)
		parameters[3] = LOWEST_CURRENT2;
	else if (parameters[3] > HIGHEST_CURRENT2)
		parameters[3] = HIGHEST_CURRENT2;

	if (parameters[4] < LOWEST_CURRENT3)
		parameters[4] = LOWEST_CURRENT3;
	else if (parameters[4] > HIGHEST_CURRENT3)
		parameters[4] = HIGHEST_CURRENT3;

	if (parameters[5] < LOWEST_CURRENT4)
		parameters[5] = LOWEST_CURRENT4;
	else if (parameters[5] > HIGHEST_CURRENT4)
		parameters[5] = HIGHEST_CURRENT4;

	if (parameters[6] < LOWEST_VOLTAGE1)
		parameters[6] = LOWEST_VOLTAGE1;
	else if (parameters[6] > HIGHEST_VOLTAGE1)
		parameters[6] = HIGHEST_VOLTAGE1;

	if (parameters[7] < LOWEST_VOLTAGE2)
		parameters[7] = LOWEST_VOLTAGE2;
	else if (parameters[7] > HIGHEST_VOLTAGE2)
		parameters[7] = HIGHEST_VOLTAGE2;

	if (parameters[8] < LOWEST_VOLTS1)
		parameters[8] = LOWEST_VOLTS1;
	else if (parameters[8] > HIGHEST_VOLTS1)
		parameters[8] = HIGHEST_VOLTS1;

	if (parameters[9] < LOWEST_VOLTS2)
		parameters[9] = LOWEST_VOLTS2;
	else if (parameters[9] > HIGHEST_VOLTS2)
		parameters[9] = HIGHEST_VOLTS2;

	if (parameters[10] < LOWEST_VOLTS3)
		parameters[10] = LOWEST_VOLTS3;
	else if (parameters[10] > HIGHEST_VOLTS3)
		parameters[10] = HIGHEST_VOLTS3;

	if (parameters[11] < LOWEST_VOLTS4)
		parameters[11] = LOWEST_VOLTS4;
	else if (parameters[11] > HIGHEST_VOLTS4)
		parameters[11] = HIGHEST_VOLTS4;

	if (parameters[12] < LOWEST_AMPS1)
		parameters[12] = LOWEST_AMPS1;
	else if (parameters[12] > HIGHEST_AMPS1)
		parameters[12] = HIGHEST_AMPS1;

	if (parameters[13] < LOWEST_AMPS2)
		parameters[13] = LOWEST_AMPS2;
	else if (parameters[13] > HIGHEST_AMPS2)
		parameters[13] = HIGHEST_AMPS2;

	if (parameters[14] < LOWEST_AMPS3)
		parameters[14] = LOWEST_AMPS3;
	else if (parameters[14] > HIGHEST_AMPS3)
		parameters[14] = HIGHEST_AMPS3;

	if (parameters[15] < LOWEST_AMPS4)
		parameters[15] = LOWEST_AMPS4;
	else if (parameters[15] > HIGHEST_AMPS4)
		parameters[15] = HIGHEST_AMPS4;

	if (parameters[16] < LOWEST_STOP_TIME)
		parameters[16] = HIGHEST_STOP_TIME;
	else if (parameters[16] > HIGHEST_STOP_TIME)
		parameters[16] = LOWEST_STOP_TIME;

	if (parameters[17] < LOWEST_START_TIME)
		parameters[17] = HIGHEST_START_TIME;
	else if (parameters[17] > HIGHEST_START_TIME)
		parameters[17] = LOWEST_START_TIME;

	// Skip 18 no limitations.

	// Skip 19, No limitations.

	if (parameters[20] < 1)
		parameters[20] = 12;
	else if (parameters[20] > 12)
		parameters[20] = 1;

	if (parameters[21] < 1)
		parameters[21] = 31;
	else if (parameters[21] > 31)
		parameters[21] = 1;

	if (parameters[22] < 1)
		parameters[22] = 7;
	else if (parameters[22] > 7)
		parameters[22] = 1;

	// Skip 23 no limitations.

	if (parameters[24] < 0)
		parameters[24] = 24;
	else if (parameters[24] > 24)
		parameters[24] = 0;

	if (parameters[25] < 0)
		parameters[25] = 60;
	else if (parameters[25] > 60)
		parameters[25] = 0;

	if (parameters[26] < 0)
		parameters[26] = 60;
	else if (parameters[26] > 60)
		parameters[26] = 0;

	// Only update date and time, if in the screens for it.
	if (currentScreen >= 19 && currentScreen <= 25)
	{
		DateTime newTimeSet(parameters[23], parameters[20], parameters[21], parameters[24], parameters[25], parameters[26], parameters[22]);

		rtc.setDateTime(newTimeSet);
	}

	switch (currentScreen)
	{
	case 2:
		emon1.current(currentPin1, parameters[2]);       // Current: input pin, calibration.
		break;
	case 3:
		emon2.current(currentPin2, parameters[3]);       // Current: input pin, calibration.
		break;
	case 4:
		emon3.current(currentPin3, parameters[4]);       // Current: input pin, calibration.
		break;
	case 5:
		emon4.current(currentPin4, parameters[5]);       // Current: input pin, calibration.
		break;
	case 6:
		emon1.voltage(voltagePin1, parameters[6], 1.7);  // Voltage: input pin, calibration, phase_shift
		break;
	case 7:
		emon2.voltage(voltagePin2, parameters[7], 1.7);  // Voltage: input pin, calibration, phase_shift
		break;
	default:
		break;
	}
}


void printScreen()
{
	lcd.clear();
	if (isInAdministratorMode)
	{
		lcd.print(screens[currentScreen][0]);
		lcd.setCursor(0, 1);

		if (currentScreen == 22)
			lcd.print(weekDay[parameters[currentScreen] - 1]);
		else
			lcd.print(parameters[currentScreen]);

		lcd.print(" ");

		lcd.print(screens[currentScreen][1]);

		lcd.print(" ");

		if (currentScreen == 0)
			lcd.print(Voltage1);
		else if (currentScreen == 1)
			lcd.print(Voltage2);
		else if (currentScreen == 2)
			lcd.print(totalAmp);
		else if (currentScreen == 3)
			lcd.print(totalAmp);
		else if (currentScreen == 4)
			lcd.print(totalAmp2);
		else if (currentScreen == 5)
			lcd.print(totalAmp2);
		else if (currentScreen == 6)
			lcd.print(Volts1);
		else if (currentScreen == 7)
			lcd.print(Volts2);
		else if (currentScreen == 8)
			lcd.print(Voltage1);
		else if (currentScreen == 9)
			lcd.print(Voltage1);
		else if (currentScreen == 10)
			lcd.print(Voltage1);
		else if (currentScreen == 11)
			lcd.print(Voltage1);
		else if (currentScreen == 12)
			lcd.print(totalAmp);
		else if (currentScreen == 13)
			lcd.print(totalAmp);
		else if (currentScreen == 14)
			lcd.print(totalAmp);
		else if (currentScreen == 15)
			lcd.print(totalAmp);
		else if (currentScreen == 16)
			lcd.print(now->hour());
		else if (currentScreen == 17)
			lcd.print(now->hour());
		else if (currentScreen == 18)
			lcd.print(RainV);
		else if (currentScreen == 19)
			lcd.print(CloudsV);
	}
	else
	{
		for (auto i = (int)
			((String(parameters_nonAdmin[currentScreen_nonAdmin][0]).length() + String(screens_nonAdmin[currentScreen_nonAdmin][0]).length() + 1) * 2);
			i < 16; i++)
		{
			lcd.print(" ");
		}
		lcd.print(screens_nonAdmin[currentScreen_nonAdmin][0]);
		lcd.print(screens_nonAdmin[currentScreen_nonAdmin][0] == "" ? "" : " ");

		lcd.print(parameters_nonAdmin[currentScreen_nonAdmin][0]);

		lcd.setCursor(0, 1);
		for (auto i = (int)
			((String(parameters_nonAdmin[currentScreen_nonAdmin][1]).length() + String(screens_nonAdmin[currentScreen_nonAdmin][1]).length() + 1) * 2);
			i < 16; i++)
		{
			lcd.print(" ");
		}
		lcd.print(screens_nonAdmin[currentScreen_nonAdmin][1]);
		lcd.print(screens_nonAdmin[currentScreen_nonAdmin][1] == "" ? "" : " ");

		lcd.print(parameters_nonAdmin[currentScreen_nonAdmin][1]);
	}
}
#pragma endregion

//--------------------------------------------------
// send the power info to the ESP module through Serial1 (comma separated and starting with *)
void sendPowerInfo(double VoltsL1, double VoltsL2, double Amps, double Watts, double Watts2)
{

	readValueB1 = analogRead(batPen1);
	readValueB2 = analogRead(batPen2);
	readValueC = analogRead(CloudsValue);
	readValueR = analogRead(rainValue);
	Voltage1 = (parameters[0] / 1023.)*readValueB1;
	Voltage2 = (parameters[1] / 1023.)*readValueB2;
	RainV = readValueR;
	CloudsV = readValueC;
	Volts1 = VoltsL1;
	Volts2 = VoltsL2;

	// Stops operation, if the Clouds value is too high.
	if (CloudsV >= parameters[19] && now->hour() < parameters[16])
	{
		Serial.println("Ouch, it burns, I don't like the sun!");
		setStatusOfRelay(0, false);
		return;
	}

	// Stops operation, if its raining.
	if (RainV <= parameters[18])
	{
		Serial.println("Ew, it's raining.");
		setStatusOfRelay(0, false);
		return;
	}

	// Turn off at Stop Time
	if (now->hour() >= parameters[16])
	{
		setStatusOfRelay(0, false);
	}
	// Allow to turn during Start Time.
	else if (now->hour() >= parameters[17])
	{
		if (((Amps)< parameters[12]) && (Voltage1) > parameters[8])
		{
			setStatusOfRelay(1, true);
		}
		else
		{
			setStatusOfRelay(1, false);
		}

		if (((Amps)< parameters[13]) && (Voltage1) > parameters[9])
		{
			setStatusOfRelay(2, true);
		}
		else
		{
			setStatusOfRelay(2, false);
		}

		if (((Amps)< parameters[14]) && (Voltage1) > parameters[10])
		{
			setStatusOfRelay(3, true);
		}
		else
		{
			setStatusOfRelay(3, false);
		}

		if (((Amps)< parameters[15]) && (Voltage1) > parameters[11])
		{
			setStatusOfRelay(4, true);
		}
		else
		{
			setStatusOfRelay(4, false);
		}
	}
	// If there's a brief time where it doesn't know what do, it will automatically turn off.
	else
	{
		setStatusOfRelay(0, false);

	}
}

