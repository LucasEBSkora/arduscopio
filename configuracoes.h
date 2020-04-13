#ifndef __CONFIGURACOES_H__
#define __CONFIGURACOES_H__

#include "Arduino.h"

enum configTrigger {
  desativado,
  subida,
  descida
};

class configuracoes {
  public:
    
    configTrigger tipoTrigger;
    unsigned int nivelTrigger;

    byte resolucao;
    unsigned long microMinEntreAmostras; //menor tempo desejado entre amostras (se for muito pequeno não funcionará)
    unsigned int numeroAmostras;

    float valorMax;
    
    configuracoes();
    void setTrig(String comando);
    void setNivelTrigger(float nivel);
    void setRes(byte resolucao);
    void setMinTempo(int micrs);
};

#endif
