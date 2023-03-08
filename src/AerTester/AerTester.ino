#include "Tester.h"
#include "errorlist.h"
#include <Display.h>
#include <Buttons.h>
#include <avr/eeprom.h>

#define SELFTEST_DELAY			100
#define BATTERY_CHAR			0x9F
#define EMPTY_BATTERY_CHAR		0x78
#define BRIGHT_KOEFF_K			13.5
#define BRIGHT_KOEFF_B			170
#define VIN_PIN					A7
#define DIV_R1					10
#define DIV_R2					1
#define VREF					1.1

#define RED_LED_PIN				A2
#define GREEN_LED_PIN			13

#define CHOOSE_CHAR				0x93

#define FILTER_K				0.5

int 		*report;
bool		simple_inf = 0;
uint16_t	selftest_counter = 0;
static char msg0[16];

float expRunningAverage(float newVal) {
  static float filVal = 0;
  filVal += (newVal - filVal) * FILTER_K;
  return filVal;
}


//**************EEPROM*DEFINITION************//

void readSimp() {
	simple_inf = eeprom_read_byte(0) & 0x01;
}

void saveSimp() {
	eeprom_update_byte(0, simple_inf);
}

uint8_t readModel() {
	return eeprom_read_byte(1);
}

void saveModel(uint8_t num){
	eeprom_update_byte(1, num);
}

uint8_t readBckl() {
	return eeprom_read_byte(2);
}

void saveBckl(uint8_t val) {
	eeprom_update_byte(2, val);
}

//**************EEPROM*DEFINITION************//


//**************TESTER*DEFINITION************//
// A A5
// B A6
// C A4
// D A3
// E 2
// F 3


const measuringGroup mGroup[] = {	// list of measure groups
	{2, A5, A6, 995},		// {pinH, pinL, pinI, Ballast resistance}
	{3, A3, A4, 981}
};

const models AER[] = {			// List of modelss
	{15, 40, 85},			// {model name (power rate), lower limit, upper limit}
	{30, 25, 55},
	{55, 10, 30},
	{75, 15, 35}
};

Tester	tester;
//**************TESTER*DEFINITION************//



//**************DISPLAY*DEFINITION***********//

ChrBlock batteryBlock[] = {
	{15, 0, 0x9F}
};

const  StrBlock BReportStrS[] = {
	{1, 0, "Неисправность"},
	{5, 1, "No"}
};

IntBlock BReportIntS[] = {
	{8, 1, 1} // {pos_x, pos_y, lefty}
};

Screen BReportScreenS(
	BReportStrS,	sizeof(BReportStrS),
	BReportIntS,	sizeof(BReportIntS),
	batteryBlock,	sizeof(batteryBlock)
	);

const  StrBlock GReportStrS[]= {
	{6, 0, "Тест"},
	{5, 1, "пройден"}
};

Screen GReportScreenS(
	GReportStrS,	sizeof(GReportStrS),
	NULL, NULL,
	NULL, NULL
	);

const StrBlock BReportStr[] = {
	{0, 0, "вх:"},
	{0, 1, "ст:"},
	{8, 1, "др:"},
	{8, 0, "неиспр:"}
};

IntBlock BReportInt[] = {
	{3,  0, 0}, // in
	{3,  1, 0}, // st
	{12, 1, 0}, // dr
	{15, 0, 1}  // er
};

Screen BReportScreen(
	BReportStr,	sizeof(BReportStr),
	BReportInt,	sizeof(BReportInt),
	NULL,		NULL	
	);

const StrBlock GReportStr[] = {
	{0, 0, "вх:"},
	{0, 1, "ст:"},
	{8, 1, "др:"},
	{8, 0, "исправна"}
};

IntBlock GReportInt[] = {
	{3,  0, 0}, // in
	{3,  1, 0}, // st
	{11, 1, 0}  // dr
};

Screen GReportScreen(
	GReportStr,	sizeof(GReportStr),
	GReportInt,	sizeof(GReportInt),
	NULL,		NULL	
	);


const StrBlock helloScreenStr[] = {
	{3, 0, "AER Тестер"},
	{2, 1, "Версия 4.1.0"}
};

Screen helloScreen(
	helloScreenStr,	sizeof(helloScreenStr),
	NULL,			NULL,
	NULL,			NULL
	);

const StrBlock idleStrBlock[] = {
	{0, 1, "Лампа"},
	{9, 1, "Вт"},
	{0, 0, "Ожидание"}
};

IntBlock modelIntBlock[] = {
	{5, 1, 0, AER[tester.get_current_model()].name}
};
//\
IntBlock debugIntBlock[] = {\
	{0, 0, 0},\
	{4, 0, 0},\
	{8, 0, 0},\
	{4, 1, 0}\
};

Screen idleScreen(
	idleStrBlock,	sizeof(idleStrBlock),
	modelIntBlock,	sizeof(modelIntBlock),
	batteryBlock,	sizeof(batteryBlock)
	);


const StrBlock modelScreenStr[] = {
	{0, 0, "Модель лампы"},
	{9, 1, "Вт"}
};

ChrBlock settingsChrBlock[] = {
	{15, 0, 0x87},
	{15, 1, 0x86}
};

Screen modelScreen(
	modelScreenStr,		sizeof(modelScreenStr),
	modelIntBlock,		sizeof(modelIntBlock),
	settingsChrBlock,	sizeof(settingsChrBlock)
	);

const StrBlock testScreenStr[] = {
	{3, 0, "Измерение"}
};

Screen testScreen(
	testScreenStr,	sizeof(testScreenStr),
	NULL,			NULL,
	batteryBlock,	sizeof(batteryBlock)
	);

StrBlock simplyScreenStr[] = {
	{0, 0, "Подробная"},
	{0, 1, "информация"}
};

Screen simplyScreen(
	simplyScreenStr,	sizeof(simplyScreenStr),
	NULL,				NULL,
	settingsChrBlock,	sizeof(settingsChrBlock)
	);

StrBlock msgScreenStr[] = {
	{0, 0, "Неизвестная"},
	{0, 1, "неисправность"}
};

Screen msgScreen(
	msgScreenStr,	sizeof(msgScreenStr),
	NULL,			NULL,
	NULL,			NULL	
	);

StrBlock bcklScreenStr[] = {
	{0, 0, "Подсветка"},
	{0, 1, "включена"}
};

Screen bcklScreen(
	bcklScreenStr,		sizeof(bcklScreenStr),
	NULL,				NULL,
	settingsChrBlock,	sizeof(settingsChrBlock)
	);

// Display definition ---------------------\

Display display(5, 6, 50);
//-----------------------------------------/

//**************DISPLAY*DEFINITION***********//



//**************BUTTONS*DEFINITION***********//

// Button action functions -------------\


void startPressed() {
	if (display._screen == &modelScreen) {
		tester.change_current_model();
		display.updateIntVal(modelIntBlock, 0, AER[tester.get_current_model()].name);
	} else if (display._screen == &simplyScreen) {
		simple_inf = !simple_inf;
		if (simple_inf) {display.updateStrVal(simplyScreenStr, 0, "Упрощенная");}
		else			{display.updateStrVal(simplyScreenStr, 0, "Подробная ");}
	} else if (display._screen == &bcklScreen) {
		display.setBacklight(!display.getBacklight());
		if (display.getBacklight()) {display.updateStrVal(bcklScreenStr, 1, "включена ");}
		else						{display.updateStrVal(bcklScreenStr, 1, "выключена");}
	} else if (display._screen == &idleScreen) {
		display.showScreen(&testScreen);
		tester.test();
		report = tester.getReport();

		/*
		if (report[3] > 0) {
			Serial.println("Bad result");
			char buff[20];
			strcpy_P(buff, (char*)pgm_read_word(&(err_msgs_h[report[3] - 1])));
			Serial.write(buff);
			Serial.println();
			strcpy_P(buff, (char*)pgm_read_word(&(err_msgs_h[report[3] - 1 + 7])));
			Serial.write(buff);
			Serial.println();
		}//*/

		if (report[3] == 0) { // If circuit is good
			display.updateIntVal(GReportInt, 0, report[0]);
			display.updateIntVal(GReportInt, 1, report[1]);
			display.updateIntVal(GReportInt, 2, report[2]);
			if (simple_inf)	{
				display.showScreen(&GReportScreenS);
			} else {
				display.showScreen(&GReportScreen);
			}
		} else {	// If circuit is bad
			display.updateIntVal(BReportInt,  0, report[0]);
			display.updateIntVal(BReportInt,  1, report[1]);
			display.updateIntVal(BReportInt,  2, report[2]);
			display.updateIntVal(BReportInt,  3, report[3]);
			display.updateIntVal(BReportIntS, 0, report[3]);
			if (simple_inf)	{
				display.showScreen(&BReportScreenS);
			} else {
				display.showScreen(&BReportScreen);
			}
		}
	} else if (display._screen == &GReportScreenS || display._screen == &GReportScreen) {
		display.showScreen(&idleScreen);
	} else if (display._screen == &BReportScreenS || display._screen == &BReportScreen) {
		display.showScreen(&idleScreen);
	} else if (display._screen == &msgScreen) {		display.showScreen(&idleScreen);
	}
}

void nextPressed() {
	if 		(display._screen == &idleScreen) {
		display.showScreen(&modelScreen);
	} else if (display._screen == &modelScreen) {
		display.showScreen(&simplyScreen);
		saveModel(tester.get_current_model());
	} else if (display._screen == &simplyScreen) {
		saveSimp();
		display.showScreen(&bcklScreen);
	} else if (display._screen == &bcklScreen) {
		saveBckl(display.getBacklight() & 0x01);
		display.showScreen(&idleScreen);
	} else if (display._screen == &GReportScreenS || display._screen == &GReportScreen) {
		display.showScreen(&idleScreen);
	} else if (display._screen == &BReportScreenS || display._screen == &BReportScreen) {
		display.showScreen(&idleScreen);
	} else if (display._screen == &msgScreen) {
		display.showScreen(&idleScreen);
	}
}

void backPressed() {
	if (display._screen == &modelScreen)		saveModel(tester.get_current_model());
	else if (display._screen == &simplyScreen)	saveSimp();
	else if (display._screen == &bcklScreen) 	saveBckl(display.getBacklight() & 0x01);

	if (display._screen != &idleScreen)			display.showScreen(&idleScreen);
}

//--------------------------------------/


buttonGroup	 bGroup[] = {	// List of buttons
	{A0,	INPUT_PULLUP, NO, nextPressed,  NULL},	// Start {pin, pin type, NO/NC, press act, release act}
	{A1,	INPUT_PULLUP, NO, startPressed,  NULL},	// Next
	{4,		INPUT_PULLUP, NO, backPressed, NULL}	// Back
};

Buttons	buttons(70);
//**************BUTTONS*DEFINITION***********//


//**************OTHER*FUNCTIONS**************//

void selftest_thread() {

	if (selftest_counter > SELFTEST_DELAY) {
		selftest_counter = 0;

		analogReference(INTERNAL);
		
		float voltage = expRunningAverage(float(analogRead(VIN_PIN) * VREF * (DIV_R1 + DIV_R2) / (DIV_R2 * 1023.0)));
		int battery_state = int(voltage * 1.07 - 5.643);

		display.setBrightness(uint8_t(BRIGHT_KOEFF_B - int(BRIGHT_KOEFF_K * voltage)));		
		
		//\
		display.updateIntVal(debugIntBlock, 0, int(voltage * 100));\
		display.updateIntVal(debugIntBlock, 1, display.getBrightness());\
		display.updateIntVal(debugIntBlock, 2, analogRead(VIN_PIN));

		if (battery_state < 0 || battery_state > 4) {
			display.updateChrVal(batteryBlock, 0, EMPTY_BATTERY_CHAR);
		} else {
			display.updateChrVal(batteryBlock, 0, BATTERY_CHAR - battery_state);
		} 

		analogReference(DEFAULT);
	} selftest_counter++;
}



void red(bool state) {
	digitalWrite(RED_LED_PIN, state);
}

void green(bool state) {
	digitalWrite(GREEN_LED_PIN, state);
}

void led_thread() {
	if (display._screen == &GReportScreenS || display._screen == &GReportScreen) {
		red(0);
		green(1);
	} else if (display._screen == &BReportScreenS || display._screen == &BReportScreen) {
		red(1);
		green(0);
	} else {
		red(0);
		green(0);
	}
}

//**************OTHER*FUNCTIONS**************//



void setup() {
	pinMode(RED_LED_PIN, OUTPUT);
	pinMode(GREEN_LED_PIN, OUTPUT);
	
	/*
	Serial.begin(9600);
	Serial.println();
	Serial.println("///////////");
	Serial.println("Start");
	//*/

	tester.setup(
		AER,	sizeof(AER) / sizeof(models),
		mGroup,	sizeof(mGroup) / sizeof(measuringGroup)
		);

	tester.set_current_model(readModel());

	buttons.setup(bGroup, sizeof(bGroup) / sizeof(buttonGroup));
	display.setup();
	readSimp();
	display.setBacklight(readBckl() & 0x01);
	if (simple_inf) {display.updateStrVal(simplyScreenStr, 0, "Упрощенная");}
	else			{display.updateStrVal(simplyScreenStr, 0, "Подробная ");}
	if (display.getBacklight()) {display.updateStrVal(bcklScreenStr, 1, "включена ");}
	else						{display.updateStrVal(bcklScreenStr, 1, "выключена");}
	display.updateIntVal(modelIntBlock, 0, AER[tester.get_current_model()].name);

	display.showScreen(&helloScreen);
	delay(1000);
	display.showScreen(&idleScreen);
}

void loop() {
	buttons.thread();
	display.thread();
	selftest_thread();
	led_thread();

	delay(1);
}
