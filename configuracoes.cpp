#include "configuracoes.h"

configuracoes::configuracoes() {
  tipoTrigger = subida;
  nivelTrigger = 2000;
  microMinEntreAmostras = 10;
  numeroAmostras = 20;
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

void configuracoes::setNivelTrigger(float nivel) {
  nivelTrigger = nivel*(valorMax/3.3);
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
