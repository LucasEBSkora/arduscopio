#define PINO_AQUISICAO A0

#include "limits.h"

enum configTrigger {
  desativado,
  subida,
  descida
};


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

relogio::relogio() {
  reiniciar();
};

void relogio::reiniciar() {
  t0 = micros();
};

unsigned long relogio::variacao() {
  tf = micros();
  dt = tf - t0;
  t0 = micros();
  return dt;
};
    

relogio Relogio;


struct conf {
    configTrigger tipoTrigger;
    unsigned int nivelTrigger;
  
    byte resolucao;
    unsigned long microMinEntreAmostras; //menor tempo desejado entre amostras (se for muito pequeno não funcionará)
    bool continuo;

};

struct conf Configuracoes;

unsigned int* valores;
unsigned int numeroAmostras;

unsigned int setupADC() {

  analogReadResolution(Configuracoes.resolucao);
  pinMode(PINO_AQUISICAO, INPUT);

}


unsigned int adquirirUnico() { //Função usada para fazer uma única leitura
  return analogRead(PINO_AQUISICAO);
}


//adquire os valores necessários de acordo com as configurações feitas
void adquirir(unsigned int *valores, unsigned int numeroAmostras) {
  int i;

   if (Configuracoes.tipoTrigger == desativado) {
     i = 0;
   } else if (Configuracoes.tipoTrigger == subida) { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger subindo, o sinal começa a ser adquirido "de verdade"
    
    int anterior, atual = 0;
    
    while (anterior > Configuracoes.nivelTrigger || atual < Configuracoes.nivelTrigger) {
      anterior = atual;
      atual = adquirirUnico();
    }
    
    valores[0] = atual;

    i = 1;
  } else { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger descendo, o sinal começa a ser adquirido "de verdade"
    
    int anterior, atual = 0;
    while (anterior < Configuracoes.nivelTrigger || atual > Configuracoes.nivelTrigger) {
      anterior = atual;
      atual = adquirirUnico();
     
    }
    valores[0] = atual;
    i = 1;
  }

  if (Configuracoes.microMinEntreAmostras == 0) {//adquirir o mais rápido possível
    for (; i < numeroAmostras; ++i) {
      valores[i] = adquirirUnico();
  
    }  
  } else {
    int deltaT = 0;
    
    Relogio.reiniciar();
    
    for (; i < numeroAmostras; ++i) {

      deltaT += Relogio.variacao();
      
      if (deltaT > Configuracoes.microMinEntreAmostras) {
        
        valores[i] = adquirirUnico();
        deltaT -= Configuracoes.microMinEntreAmostras;
      }
  
    }  
  }
  

}

//Transmite os sinais para o computador
void transmitir(unsigned int *valores, unsigned int numeroAmostras) {

  int max = pow(2, Configuracoes.resolucao) - 1;
  int i;

  for (i = 0; i < numeroAmostras; ++i) {
    Serial.println(3.3*valores[i]/max);
  }
}



void setup() {
  Serial.begin(115200);  

  Configuracoes.tipoTrigger = subida;
  Configuracoes.nivelTrigger = 2000;
  Configuracoes.resolucao = 12;
  Configuracoes.microMinEntreAmostras = 100;
  Configuracoes.continuo = false;
  numeroAmostras = 500;

  valores = (unsigned int*) malloc(sizeof(unsigned int) * numeroAmostras);

  setupADC();
  
  Serial.setTimeout(10);
  

}

void loop() {
  
  /*int temp = micros();

   valores[0] = adquirirUnico(); //o resultado dessa operação é 5~6 microsegundos, mas, teoricamente, a frequência de amostragem do ADC é 1MHz, e isso até antes de tentar fazer overclock

  Serial.println(micros() - temp);*/
  
  if (Configuracoes.continuo) {

    //int temp = micros();
    
    adquirir(valores, numeroAmostras);

    //Serial.println(micros() - temp);
    
    transmitir(valores, numeroAmostras);
  } else {
    
    if (Serial.readString() == "LER") {
      adquirir(valores, numeroAmostras);
      transmitir(valores, numeroAmostras);
    }
  }


 
}
