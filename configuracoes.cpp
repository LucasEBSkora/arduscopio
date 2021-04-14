#include "configuracoes.h"

configuracoes::configuracoes() {
  tipoTrigger = subida;
  nivelTrigger = 500;
  microMinEntreAmostras = 0;
  numeroAmostras = 500;
  resolucao = 12;
  valorMax = pow(2, resolucao) - 1;
}

void configuracoes::setTrig(String comando) {
  //Serial.print('\"'); Serial.print(comando); Serial.println('\"');
  if (comando == "DESATIVADO")
    tipoTrigger = desativado;
  else if (comando == "SUBIDA")
    tipoTrigger = subida;
  else if (comando == "DESCIDA")
    tipoTrigger = descida;
}

void configuracoes::setNivelTrigger(unsigned short nivel) {
  nivelTrigger = nivel;
}

void configuracoes::setRes(byte res) {
  if (res < 1 || res > 12) return;
  nivelTrigger *= (pow(2, res) - 1)/valorMax;
  resolucao = res;
  valorMax = pow(2, resolucao) - 1;
}

void configuracoes::setMinTempo(int micrs) {
  microMinEntreAmostras = micrs;
}
