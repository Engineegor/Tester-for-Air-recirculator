#include "Arduino.h"

#define DEFAULT_DELAY	500

#define PIN_SETUP_DELAY	700		// delay between setup pins and measure start
#define MEASURE_DELAY	10		// delay between measurements
#define MEASURE_COUNT	15		// measurements count

#define U_SUPPLY		4.7		// supply voltage
#define V_MAX			1023	// upper limit of measured value
#define V_MIN			0		// lower limit of measured value
#define R_MAX			2400	// max resistance
#define R_MIN			200		// min resistance
#define R_SHORT_CIRCUIT	5		// Short circuit limit
#define R_CIRCUIT_BREAK 5000	// Ciruit break range start

#define K_ADC_VAL		1023.0 / float(V_MAX - V_MIN)

struct models 
{
	uint16_t name;
	uint16_t Rb_min;
	uint16_t Rb_max;
};

struct measuringGroup
{
	uint8_t pinH;
	uint8_t pinL;
	uint8_t pinI;
	float	Rb;
};

class Tester {
	public :
		Tester(int t_delay = DEFAULT_DELAY);
		void			setup(models* AER, int mLen, measuringGroup* pins, int gLen);
		void			change_current_model();
		uint8_t			set_current_model(uint8_t num);
		uint8_t			get_current_model();
		void 			set_measure_param(int pin_sd = PIN_SETUP_DELAY, int m_delay = MEASURE_DELAY, int m_count = MEASURE_COUNT);
		int				*getReport();
		void			test();
		void 			errorMsg(char *buff0, char *buff1);

	private:
		int				measure_group(int group);
		bool 			inRange(int val, int min, int max);
		bool 			inRange(int val, int min);
		uint8_t			rangeCode(int val, int rsc, int rmin, int rmax, int rcb);
		uint8_t			ballastCode(int val, int rmin, int rmax, int rup);
		void 			set_pins(int group);
		void 			reset_pins(int group);
		int 			v2r(int val, int group);

		uint16_t		_t_delay, _t_counter;

		uint8_t 		_model_count;
		uint8_t 		_current_model;
		uint8_t 		_group_count;

		uint16_t		_pin_setup_delay;
		uint16_t 		_measure_delay;
		uint16_t 		_measure_count;

		uint16_t		_r_max;
		uint16_t		_r_min;

		int				_Rab;
		int				_Rcd;
		int				_Rdr;

		int				lastReport[4];

		models			*_model;
		measuringGroup	*_measuringGroup;

};