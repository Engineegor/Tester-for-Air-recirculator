#include "Tester.h"
#include "errorList.h"

// H ______  E/F
//        _|_
//       |   |
//       |   |
//       |___|
// I ______|_________A/C
//   
//   
// L ________________B/D

Tester::Tester(int t_delay) {
	_t_delay = t_delay;
}


void Tester::set_measure_param(int pin_sd, int m_delay, int m_count) {
	_pin_setup_delay = pin_sd;
	_measure_count = m_count;
	_measure_delay = m_delay;
}

void Tester::change_current_model() {
	_current_model++;
	if (_current_model >= _model_count) _current_model = 0;
}

uint8_t Tester::set_current_model(uint8_t num) {
	if (num <= _model_count) _current_model = num;
}

uint8_t Tester::get_current_model() {
	return _current_model;
}

void Tester::set_pins(int group) {
	pinMode(_measuringGroup[group].pinH, OUTPUT);
	pinMode(_measuringGroup[group].pinL, OUTPUT);
	digitalWrite(_measuringGroup[group].pinH, HIGH);
	digitalWrite(_measuringGroup[group].pinL, LOW);
}

void Tester::reset_pins(int group) {
	pinMode(_measuringGroup[group].pinH, INPUT);
	pinMode(_measuringGroup[group].pinL, INPUT);
}


void Tester::setup(models* AER, int mLen, measuringGroup* pins, int gLen) {
	
	_model = AER;
	_measuringGroup = pins;
	_group_count = gLen;
	_model_count = mLen;

	set_measure_param();
	
	for (int i = 0; i < _group_count; i++) {
		reset_pins(i);
	}
}

int Tester::v2r(int val, int group) {			// convert measured value into Ohms
  float U = U_SUPPLY * float(K_ADC_VAL * float(val)  - V_MIN) / float(V_MAX - V_MIN);
  return int (_measuringGroup[group].Rb * U / (U_SUPPLY - U));
}


int Tester::measure_group(int group) {
	if (group < _group_count) {
		set_pins(group);
		delay(_pin_setup_delay);
		long dataM = 0;
	
		for (int i = 0; i < _measure_count; i++) {
			dataM += analogRead(_measuringGroup[group].pinI);
			delay(_measure_delay);
		}
		int val = dataM / _measure_count;
	
		reset_pins(group);
		return v2r(val, group);
	} else return -123; 
}

bool Tester::inRange(int val, int min, int max) {
	if ((val >= min) && (val <= max))	return 1;
	else								return 0;
}

bool Tester::inRange(int val, int min) {
	if ((val >= min))	return 1;
	else				return 0;
}

uint8_t Tester::rangeCode(int val, int rsc, int rmin, int rmax, int rcb) {
	if (inRange(val, 0, rsc))			return 1;
	else if (inRange(val, rsc, rmin))	return 2;
	else if (inRange(val, rmin, rmax))	return 3;
	else if (inRange(val, rmax, rcb))	return 4;
	else if (inRange(val, rcb))			return 5;
	else if (val < 0)					return 5;
}

uint8_t Tester::ballastCode(int val, int rmin, int rmax, int rup) {
	if (val < 0)						return 1;
	else if (inRange(val, 0, rmin))		return 2;
	else if (inRange(val, rmin, rmax))	return 3;
	else if (inRange(val, rmax, rup))	return 4;
	else if (val >= rup)				return 5;
}

int *Tester::getReport() {
	return lastReport;
}

void Tester::test() {

	_Rab = measure_group(0);
	_Rcd = measure_group(1);
	_Rdr = _Rcd - _Rab;

	int8_t i_code = -1;
	int8_t s_code = -1;
	int8_t b_code = -1;
	int8_t code = -1;


	i_code = rangeCode(_Rab, R_SHORT_CIRCUIT, R_MIN, R_MAX, R_CIRCUIT_BREAK);
	s_code = rangeCode(_Rcd, R_SHORT_CIRCUIT, R_MIN, R_MAX, R_CIRCUIT_BREAK);
	b_code = ballastCode(_Rdr, _model[_current_model].Rb_min, _model[_current_model].Rb_max, R_MAX - R_MIN);

	if (i_code == 5)				_Rab = -1;
	if (s_code == 5)				_Rcd = -1;
	if (b_code == 1 || b_code == 5)	_Rdr = -1;

	if (i_code == 3 && s_code == 3 && b_code == 3)	code = 0;
	else if (i_code == 5)							code = 1;
	else if (s_code == 5)				 			code = 2;
	else if (i_code == 1)				 			code = 3;
	else if (s_code == 1)				 			code = 4;
	else if (b_code == 2 || b_code == 4)			code = 5;
	else if (b_code == 1)				 			code = 6;
	else if (i_code == 1 || i_code == 4)			code = 7;


	lastReport[0] = _Rab;
	lastReport[1] = _Rcd;
	lastReport[2] = _Rdr;
	lastReport[3] = code;
}