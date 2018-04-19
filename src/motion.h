/*
Motion.h - Library for detecting motion and proximity!
Created by Connor Nishijima, January 7th 2018.
Released under the GPLv3 license.
*/

#ifndef motion_h
#define motion_h
#include "Arduino.h"

void motion_start();
int16_t motion();
int16_t proximity();
void calibrate_proximity();

uint8_t _apin = A0;

const uint8_t _read_count = 35;
const uint8_t _out_smooth = 15;

volatile uint8_t _readings[2][_read_count];

volatile int16_t _motion[2] = {0,0};

volatile int16_t _current_motion = 0;
volatile int16_t _old_motion = 0;
volatile int16_t _motion_speed = 0;

volatile uint8_t _read_flip = 0;

volatile int16_t _motion_mn = 0;
volatile int16_t _motion_mx = 0;

volatile int16_t _prox_cal = 0;
volatile int16_t _motion_history[_out_smooth];
volatile int16_t _prox_history[_out_smooth];
volatile uint8_t _history_index = 0;

volatile uint8_t _iter = 0;

volatile int16_t _motion_smooth = 0;
volatile int16_t _prox_smooth = 0;

void motion_start(uint8_t p){
  _apin = p;
  for (uint8_t i = 0; i < _read_count; i++) {
    _readings[0][i] = 0;
    _readings[1][i] = 0;
  }
  for (uint8_t i = 0; i < _out_smooth; i++) {
    _motion_history[i] = 0;
    _prox_history[i] = 0;
  }

  // defines for setting and clearing register bits
  #ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
  #endif
  #ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
  #endif

  // set prescale to 16
  sbi(ADCSRA, ADPS2) ;
  cbi(ADCSRA, ADPS1) ;
  cbi(ADCSRA, ADPS0) ;

  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0

  OCR1A = F_CPU / 4150;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 8 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
  delay(1000);
  calibrate_proximity();
}

ISR(TIMER1_COMPA_vect) {
  _read_flip = !_read_flip;
  
  for (uint8_t i = 0; i < _read_count - 1; i++) {
    _readings[_read_flip][i] = _readings[_read_flip][i + 1];
  }
  _readings[_read_flip][_read_count - 1] = analogRead(_apin) / 4;
  _motion[_read_flip] = _readings[_read_flip][_read_count - 1] - _readings[_read_flip][0];

  _current_motion = _motion[0]+_motion[1];

  if(_current_motion > _motion_mx){
    _motion_mx = _current_motion;
  }
  if(_current_motion < _motion_mn){
    _motion_mn = _current_motion;
  }
  _motion_mx--;
  _motion_mn++;

  int16_t motion_reading = _motion_mx+(_motion_mn);
  int16_t prox_reading = _motion_mx-_motion_mn;

  _motion_history[_history_index] = motion_reading;
  _prox_history[_history_index] = prox_reading;

  _history_index++;
  if(_history_index >= _out_smooth){
    _history_index = 0;
  }

  int16_t sum = 0;
  for(uint8_t i = 0; i < _out_smooth; i++){
    sum+=_motion_history[i];
  }
  _motion_smooth = sum/_out_smooth;

  sum = 0;
  for(uint8_t i = 0; i < _out_smooth; i++){
    sum+=_prox_history[i];
  }
  _prox_smooth = sum/_out_smooth;  
}

int16_t motion() {
	return _motion_smooth;
}

int16_t proximity() {
	return _prox_smooth-_prox_cal;
}

void calibrate_proximity() {
	_prox_cal = _prox_smooth;
}

#endif