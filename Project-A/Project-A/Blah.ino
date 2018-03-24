
#define VOLT_CAL1 120.0    //voltage calibration
#define VOLT_CAL2 120.0    //voltage calibration
#define CURRENT_CAL1 43.6  //sensor 1 calibration
#define CURRENT_CAL2 42.6  //sensor 2 calibration
#define CURRENT_CAL3 43.6  //sensor 3 calibration
#define CURRENT_CAL4 42.6  //sensor 4 calibration

To

#define offnow.hour= offhour)   //Stop Time
#define startnow.hour= onhour)  //Start Time

#define VOLT_CAL1 (1Vnumber)    //L1 AC voltage calibration
#define VOLT_CAL2 (2Vnumber)    //L2 AC voltage calibration
#define CURRENT_CAL1 (1Anumber) //sensor 1 calibration
#define CURRENT_CAL2 (2Anumber) //sensor 2 calibration
#define CURRENT_CAL3 (3Anumber) //sensor 3 calibration
#define CURRENT_CAL4 (4Anumber) //sensor 4 calibration
#define Voltage1 = (1Bnumber)   //bat1 volt Calibrate
#define Voltage2 = (2Bnumber)   //bat2 volt Calibrate

lcd prints these numbers to start from


int VOLT_CAL1Vnumber = 120.0    //L1 AC Calibrate
int VOLT_CAL2Vnumber = 120.0    //L2 AC Calibrate
int CURRENT_CAL1Anumber = 43.6  //CT1 Calibrate
int CURRENT_CAL2Anumber = 43.6  //CT2 Calibrate
int CURRENT_CAL3Anumber = 43.6  //CT3 Calibrate
int CURRENT_CAL4Anumber = 43.6  //CT4 Calibrate 

								//above numbers will go threw cali to get totalAmp(Anumber) ,totalvolt(Vnumber)

	int offhour = 20                //Stop Time
	int onhour = 6                  //Start Time
	int 1Bnumber = 208               //bat1 volt Calibrate
	int 2Bnumber = 208               //bat2 volt Calibrate
	int Date_TimeTYear = 2018       // Year
	int Date_TimeTMounth = 3        //Mounth
	int Date_TimeTDay = 11          //Day
	int Date_TimeTHour = 14         //Hour
	int Date_TimeTMinute = 22       //Minute
	int Date_TimeTSeconds = 0       //Seconds
	int Date_TimeTDay Of Week = 7   //Day Of Week 7 = sunday
	int Rain_RainVRnumber = 800     //Rain Calibrate

	int lcdinputpin 23 = (1Bnumber)  //button down -numbers
	int lcdinputpin 25 = (1Bnumber)  //button up +numbers
	int lcdinputpin 23 = (2Bnumber)  //button down -numbers
	int lcdinputpin 25 = (2Bnumber)  //button up +numbers
	int lcdinputpin 23 = (1Vnumber)  //button down -numbers
	int lcdinputpin 25 = (1Vnumber)  //button up +numbers
	int lcdinputpin 23 = (2Vnumber)  //button down -numbers
	int lcdinputpin 25 = (2Vnumber)  //button up +numbers
	int lcdinputpin 23 = (1Anumber)  //button down -numbers
	int lcdinputpin 25 = (1Anumber)  //button up +numbers
	int lcdinputpin 23 = (2Anumber)  //button down -numbers
	int lcdinputpin 25 = (2Anumber)  //button up +numbers
	int lcdinputpin 23 = (3Anumber)  //button down -numbers
	int lcdinputpin 25 = (3Anumber)  //button up +numbers
	int lcdinputpin 23 = (4Anumber)  //button down -numbers
	int lcdinputpin 25 = (4Anumber)  //button up +numbers
	int lcdinputpin 23 = (offhour)   //button down -numbers
	int lcdinputpin 25 = (offhour)   //button up +numbers
	int lcdinputpin 23 = (onhour)    //button down -numbers
	int lcdinputpin 25 = (onhour)    //button up +numbers
	int lcdinputpin 23 = (TYear)     //button down -numbers
	int lcdinputpin 25 = (TYear)     //button up +numbers
	int lcdinputpin 23 = (TMounth)   //button down -numbers
	int lcdinputpin 25 = (TMounth)   //button up +numbers
	int lcdinputpin 23 = (TDay)      //button down -numbers
	int lcdinputpin 25 = (TDay)      //button up +numbers
	int lcdinputpin 23 = (THour)     //button down -numbers
	int lcdinputpin 25 = (THour)     //button up +numbers
	int lcdinputpin 23 = (TMinute)   //button down -numbers
	int lcdinputpin 25 = (TMinute)   //button up +numbers
	int lcdinputpin 23 = (TSeconds)  //button down -numbers
	int lcdinputpin 25 = (TSeconds)  //button up +numbers
	int lcdinputpin 23 = (TDay Of Week)  //button down -numbers
	int lcdinputpin 25 = (TDay Of Week)  //button up +numbers
	int lcdinputpin 23 = (Rnumber)       //button down -numbers
	int lcdinputpin 25 = (Rnumber)       //button up +numbers

	DateTime dt(2018, 3, 11, 14, 22, 0, 7);
DateTime dt(TYear, TMounth, TDay, THour, TMinute, TSeconds, TDay Of Week);

int totalAmp = (Anumber)
int totalvolt = (Vnumber)

Voltage1 = (208. / 1023.)*readValueB1;
Voltage1 = (1Bnumber. / 1023.)*readValueB1;
Voltage2 = (2Bnumber. / 1023.)*readValueB1;

if (RainV = Rnumber)
if (now.hour() >= offhour)
if (((Amps)< (Anumber)) && (Voltage1)< (Vnumber) && now.hour() >= onhour)
	Relay1 if (((Amps)< (Anumber1)) && (Voltage1)< (Vnumber1)
		Relay2 if (((Amps)< (Anumber2)) && (Voltage1)< (Vnumber2)
			Relay3 if (((Amps)< (Anumber3)) && (Voltage1)< (Vnumber3)
				Relay4 if (((Amps)< (Anumber4)) && (Voltage1)< (Vnumber4)

					String screens[numOfScreens][2] = { { "Bat1 Volts",1Bnumber"Voltage1" },{ "Bat2 Volts",2Bnumber "Voltage2" },
					{ "CT1 Calibrate",1Anumber"Amps1" },{ "CT2 Calibrate",2Anumber"Amps2" },{ "CT3 Calibrate",3Anumber"Amps3" },{ "CT4 Calibrate",4Anumber"Amps4" },
					{ "L1 AC Calibrate",1Vnumber"AC-Volts" },{ "L2 AC Calibrate",1Vnumber"AC-Volts" },{ "Stop Time",offhour "Hour" },{ "Start Time",onhour"Hour" },
					{ "Relat1",Vnumber1"Volts" },{ "Relat2",Vnumber2"Volts" },{ "Relat3",Vnumber3"Volts" },{ "Relat4",Vnumber4"Volts" },
					{ "Relat1",Anumber1"Amps" },{ "Relat2",Anumber2"Amps" },{ "Relat3",Anumber3"Amps" },{ "Relat4",Anumber4"Amps" } ,
					{ "Date Time",2018"Year " },{ "Date Time", 3"Mounth " },{ "Date Time",11"Day " },{ "Date Time",14"Hour " },
					{ "Date Time", 22"Minute " },{ "Date Time", 0"Seconds " },{ "Date Time", 7"Day Of Week " },{ "Rain", Rnumber"Rain " } };