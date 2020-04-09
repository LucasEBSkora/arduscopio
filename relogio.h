#ifndef __RELOGIO_H__
#define __RELOGIO_H__

#include "Arduino.h"

class relogio {
  private:
    unsigned long t0;
    unsigned long tf;
    unsigned long dt;
  
  public:
    relogio();
    void reiniciar();
    unsigned long variacao();
    
  
};

#endif