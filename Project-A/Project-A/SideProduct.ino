
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "Sodaq_DS3231.h"
#include "EmonLib.h"

#define SAMPLE1_COUNT 5
#define SAMPLE_COUNT 5
#define VOLT_CAL1 120.0    //voltage calibration
#define VOLT_CAL2 120.0    //voltage calibration
#define CURRENT_CAL1 43.6 //sensor 1 calibration
#define CURRENT_CAL2 42.6 //sensor 2 calibration

#define CURRENT_CAL3 43.6 //sensor 1 calibration
#define CURRENT_CAL4 42.6 //sensor 2 calibration
const int currentPin1 = A2;
const int currentPin2 = A1;
const int voltagePin1 = A3;
const int voltagePin2 = A0;
const int currentPin3 = A8;
const int currentPin4 = A9;


//A4,A5 CLOCK
int Relay1 = 22;
int Relay2 = 24;
int Relay3 = 26;
int Relay4 = 28;//
int batPen1 = A10;
int batPen2 = A11;
int RainV = 0;
int CloudsV = 0;
float rainValue = A7;
float CloudsValue = A6;
float Voltage1;
float Voltage2;
double Volts1;
double Volts2;
int readValueB1;
int readValueB2;
int readValueC;
int readValueR;
//counter to keep track of the current sample location
int counter = 0;
int counter1 = 0;
bool relaysEnabled[] = { false, false, false, false };

//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also.
//DateTime dt(2018, 3, 11, 14, 22, 0, 7);
DateTime now;
double totalAmp;
double totalAmp2;
double totalWatt;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
	0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
//relay pins{2, 3, 4, 5, 6, 7, 8, 9, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
//35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53};
// initialize the library by providing the nuber of pins to it
//LiquidCrystal lcd(8,9,4,5,6,7);//lcd with buttons

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);//LCD NO BUTTONS
//LiquidCrystal lcd(36, 34, 41, 39, 37, 35);//LCD NO BUTTONS
//LiquidCrystal lcd(36, 34, 44, 42, 40, 38);//LCD NO BUTTONS
//const int rs = 36, en = 34, d4 = 48, d5 = 44, d6 = 42, d7 = 40;
//const int rs = 36, en = 34, d4 = 44, d5 = 42, d6 = 40, d7 = 38;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LiquidCrystal lcd(36, 34, 44, 42, 40, 38);
//Ethernet 4,10,11,12,13
//Relays 2,11,12,13
//-------------------------------------------
const int numOfInputs = 4;

const int inputPins[numOfInputs] = { 23,25,27,29 };
int inputState[numOfInputs];
int lastInputState[numOfInputs] = { LOW,LOW,LOW,LOW };
bool inputFlags[numOfInputs] = { LOW,LOW,LOW,LOW };
long lastDebounceTime[numOfInputs] = { 0,0,0,0 };
long debounceDelay = 5;

//LCD Menu Logic
const int numOfScreens = 10;
int currentScreen = 0;
String screens[numOfScreens][2] = { { "Bat1 Volts","DC-Volts" },{ "Bat2 Volts", "DC-Volts" },
{ "CT1 Calibrate","Amps" },{ "CT2 Calibrate","Amps" },{ "CT3 Calibrate","Amps" },{ "CT4 Calibrate","Amps" },
{ "L1 AC Calibrate","AC-Volts" },{ "L2 AC Calibrate","AC-Volts" },{ "Stop Time", "Hour" },{ "Start Time","Hour" } };
int parameters[numOfScreens];
//-------------------------------------------------------------------
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


void setup(void)
{
	Serial.begin(9600);

	Wire.begin();
	rtc.begin();
	//rtc.setDateTime(dt);


	emon1.voltage(voltagePin1, VOLT_CAL1, 1.7);  // Voltage: input pin, calibration, phase_shift
	emon1.current(currentPin1, CURRENT_CAL1);       // Current: input pin, calibration.

	emon2.voltage(voltagePin2, VOLT_CAL2, 1.7);  // Voltage: input pin, calibration, phase_shift
	emon2.current(currentPin2, CURRENT_CAL2);       // Current: input pin, calibration.

	emon3.current(currentPin3, CURRENT_CAL3);       // Current: input pin, calibration.
	emon4.current(currentPin4, CURRENT_CAL4);       // Current: input pin, calibration.
	
	pinMode(Relay1, OUTPUT);
	pinMode(Relay2, OUTPUT);
	pinMode(Relay3, OUTPUT);
	pinMode(Relay4, OUTPUT);
	pinMode(batPen1, INPUT);
	pinMode(batPen2, INPUT);


	//----------------------------------------------
	for (int i = 0; i < numOfInputs; i++) {
		pinMode(inputPins[i], INPUT);
		digitalWrite(inputPins[i], HIGH); // pull-up 20k
	}
	//---------------------------------------------

	pinMode(RainV, INPUT);
	pinMode(CloudsV, INPUT);
	//}
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	lcd.begin(16, 2);

	// start the Ethernet connection and the server:
	Ethernet.begin(mac, ip);
	server.begin();
	Serial.print("server is at ");
	Serial.println(Ethernet.localIP()); 
	
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
	if (counter >= SAMPLE_COUNT)
	{
		counter = 0;
	}
	if (counter1 >= SAMPLE1_COUNT)
	{
		counter1 = 0;
	}

	//calculate the most recent readings
	emon1.calcVI(20, 5000);
	emon2.calcVI(20, 5000);
	emon3.calcVI(20, 5000);
	emon4.calcVI(20, 5000);


	//save the voltage, current, watts to the array for later averaging


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


	//setup the vars to be averaged

	double wattAvg1 = 0;
	double voltAvg1 = 0;
	double ampAvg1 = 0;

	double wattAvg2 = 0;
	double voltAvg2 = 0;
	double ampAvg2 = 0;

	double ampAvg3 = 0;
	double ampAvg4 = 0;



	//add em up for averaging
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		for (int i = 0; i < SAMPLE1_COUNT; i++)
		{
			wattAvg1 += watts1[i];
			voltAvg1 += volts1[i] * 2;
			ampAvg1 += amps1[i];

			wattAvg2 += watts2[i];
			voltAvg2 += volts2[i] * 2;
			ampAvg2 += amps2[i];

			ampAvg3 += amps3[i];
			ampAvg4 += amps4[i];
		}
	}
	//get the final average by dividing by the # of samples
	wattAvg1 /= SAMPLE_COUNT;
	ampAvg1 /= SAMPLE_COUNT;
	voltAvg1 /= SAMPLE_COUNT * 2;

	wattAvg2 /= SAMPLE_COUNT;
	ampAvg2 /= SAMPLE_COUNT;
	voltAvg2 /= SAMPLE_COUNT * 2;

	ampAvg3 /= SAMPLE1_COUNT;
	ampAvg4 /= SAMPLE1_COUNT;


	//calculate the total amps and watts
	totalAmp = ampAvg1 + ampAvg2;
	totalAmp2 = ampAvg3 + ampAvg4;
	//totalWatt = wattAvg1 + wattAvg2;
	//double totalvolt = voltAvg1 + voltAvg2;
	//send the power info to the ESP module through Serial1
	sendPowerInfo(voltAvg1, voltAvg2, totalAmp, totalWatt);

	setInputFlags();
	resolveInputFlags();
	////delay(1000);
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
	Serial.print((stat ? " ON" : " OFF"));
	Serial.print(" | Bat1 Volt ");
	Serial.print(Voltage1);
	Serial.print(" | Bat2 Volt ");
	Serial.print(Voltage2);
	Serial.print(" | I_Amps ");
	Serial.print(totalAmp);
	Serial.print(" | H_Amps ");
	Serial.print(totalAmp2);
	Serial.print(" | Watts ");
	Serial.print(totalWatt);
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

void setInputFlags() {
	for (int i = 0; i < numOfInputs; i++) {
		int reading = digitalRead(inputPins[i]);
		if (reading != lastInputState[i]) {
			lastDebounceTime[i] = millis();
		}
		if ((millis() - lastDebounceTime[i]) > debounceDelay) {
			if (reading != inputState[i]) {
				inputState[i] = reading;
				if (inputState[i] == HIGH) {
					inputFlags[i] = HIGH;
				}
			}
		}
		lastInputState[i] = reading;
	}
}

void resolveInputFlags() {
	for (int i = 0; i < numOfInputs; i++) {
		if (inputFlags[i] == HIGH) {
			inputAction(i);
			inputFlags[i] = LOW;
			printScreen();
		}
	}
}

void inputAction(int input) {
	if (input == 0) {
		if (currentScreen == 0) {
			currentScreen = numOfScreens - 1;
		}
		else {
			currentScreen--;
		}
	}
	else if (input == 1) {
		if (currentScreen == numOfScreens - 1) {
			currentScreen = 0;
		}
		else {
			currentScreen++;
		}
	}
	else if (input == 2) {
		parameterChange(0);
	}
	else if (input == 3) {
		parameterChange(1);
	}
}

void parameterChange(int key) {
	if (key == 0) {
		parameters[currentScreen]++;
	}
	else if (key == 1) {
		parameters[currentScreen]--;
	}
}
void printScreen() {

	lcd.clear();
	lcd.print(screens[currentScreen][0]);
	lcd.setCursor(0, 1);
	lcd.print(parameters[currentScreen]);
	lcd.print(" ");
	lcd.print(screens[currentScreen][1]);

}

//--------------------------------------------------
//send the power info to the ESP module through Serial1 (comma separated and starting with *)
void sendPowerInfo(double VoltsL1, double VoltsL2, double Amps, double Watts) {

	readValueB1 = analogRead(batPen1);
	readValueB2 = analogRead(batPen2);
	readValueC = analogRead(CloudsValue);
	readValueR = analogRead(rainValue);
	Voltage1 = (208. / 1023.)*readValueB1;
	Voltage2 = (208. / 1023.)*readValueB2;
	RainV = readValueR;
	CloudsV = readValueC;
	Volts1 = VoltsL1;
	Volts2 = VoltsL2;

	//delay(5000);

	// Stops operation, if the Clouds value is too high.
	if (CloudsV <= 100)
	{
		Serial.println("Ouch, it burns, I don't like the sun!");
		setStatusOfRelay(0, false);
		return;
	}

	// Stops operation, if its raining.
	if (RainV <= 100)
	{
		Serial.println("Ew, it's raining.");
		setStatusOfRelay(0, false);
		return;
	}

	if (now.hour() >= 20)
		// if(Voltage1 < 47)
	{
		{
			setStatusOfRelay(0, false);
		}
	}
	else
	{
		if (((Amps)< 800) && (Voltage1)< 44 && now.hour() >= 5)

		{
			setStatusOfRelay(1, true);
		}
		else
		{
			setStatusOfRelay(1, false);
		}

		if (((Amps)< 800) && (Voltage1)< 44 && now.hour() >= 5)
		{
			setStatusOfRelay(2, true);
		}
		else
		{
			setStatusOfRelay(2, false);
		}

		if (((Amps)< 8) && (Voltage1)< 44 && now.hour() >= 5)
		{
			setStatusOfRelay(3, true);
		}
		else
		{
			setStatusOfRelay(3, false);
		}

		if (((Amps)< 8) && (Voltage1)< 44 && now.hour() >= 5)
		{
			setStatusOfRelay(4, true);
		}
		else
		{
			setStatusOfRelay(4, false);
		}
	}

	/*
	//lcd.begin(16, 2);              // start the library
	lcd.setCursor(0,0);
	lcd.print(' ');
	lcd.print("POWER WALL"); // print a simple message
	lcd.print(' ');
	lcd.print(weekDay[now.dayOfWeek()]); // print a simple message
	lcd.setCursor(0,1);
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

	lcd.clear( );
	lcd.setCursor(0,0);
	lcd.print(' ');
	lcd.print(' ');
	lcd.print(' ');
	lcd.print(' ');
	lcd.print("Bat"); // print a simple message
	lcd.print(' ');
	lcd.print(Voltage1);
	lcd.setCursor(0,1);
	lcd.print("InverterAmp");
	lcd.print(' ');
	lcd.print(Amps );
	delay(2000);

	lcd.clear( );
	lcd.setCursor(0,0);
	lcd.print(' ');
	lcd.print(' ');
	lcd.print("Watts"); // print a simple message
	lcd.print(' ');
	lcd.print(Watts);
	lcd.setCursor(0,1);
	lcd.print("L1");
	lcd.print(' ');
	lcd.print(Volts1 );
	lcd.print(' ');
	lcd.print("L2");
	lcd.print(' ');
	lcd.print(Volts2 );
	delay(2000);

	lcd.clear( );
	lcd.setCursor(0,0);
	lcd.print("Relay ");
	lcd.setCursor(6,0);
	lcd.print("1");
	lcd.setCursor(11,0);
	lcd.print("2");
	//lcd.setCursor(12,0);
	lcd.setCursor(0,1);
	lcd.print("Relay");
	lcd.setCursor(6,1);
	lcd.print("3");
	lcd.setCursor(11,1);
	lcd.print("4");
	//lcd.setCursor(12,1);
	if (isRelayActivated(1)) {
	lcd.setCursor(8,0);
	lcd.print("ON");
	} else {
	lcd.setCursor(8,0);
	lcd.print("OFF");
	}
	if (isRelayActivated(2)) {
	lcd.setCursor(13,0);
	lcd.print("ON");
	} else {
	lcd.setCursor(13,0);
	lcd.print("OFF");
	}
	if (isRelayActivated(3)) {
	lcd.setCursor(8,1);
	lcd.print("ON");
	} else {
	lcd.setCursor(8,1);
	lcd.print("OFF");
	}
	if (isRelayActivated(4)) {
	lcd.setCursor(13,1);
	lcd.print("ON");
	} else {
	lcd.setCursor(13,1);
	lcd.print("OFF");
	return;
	////delay(60000);
	*/

	// listen for incoming clients
	/*EthernetClient client = server.available();
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
					// output the value of each analog input pin
					for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
						int sensorReading = analogRead(analogChannel);
						client.print("analog input ");
						client.print(analogChannel);
						client.print(" is ");
						client.print(sensorReading);
						client.println("<br />");
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
	*/
}
//}
