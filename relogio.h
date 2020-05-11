#ifndef __RELOGIO_H__
#define __RELOGIO_H__

#include "Arduino.h"

//Uma classe simples para calcular variações de tempo

class relogio {
  private:
    unsigned long t0;
    unsigned long tf;
    unsigned long dt;
  
  public:
    //Construtor sem parâmetros
    relogio();
    //Destrutor vazio
    ~relogio();

    //reinicia o timer
    void reiniciar();
    
    //retorna o tempo em milissegundos desde à última chamada a variacao ou reiniciar 
    unsigned long variacao();
    
  
};

#endif