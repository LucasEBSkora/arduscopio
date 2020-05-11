#include "relogio.h"
#include "Arduino.h"

relogio::relogio() {
  reiniciar();
}

relogio::~relogio() {

}

void relogio::reiniciar() {
  t0 = micros();
}

unsigned long relogio::variacao() {
  tf = micros();
  dt = tf - t0;
  t0 = micros();
  return dt;
}