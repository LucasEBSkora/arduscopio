#define PINO_AQUISICAO A0

#include "limits.h"

#include "relogio.h"
relogio Relogio;

#include "configuracoes.h"
configuracoes Configuracoes;


unsigned int* valores;

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
    
    int anterior = adquirirUnico(), atual = adquirirUnico();
    
    while (anterior >= Configuracoes.nivelTrigger || atual < Configuracoes.nivelTrigger) {
      anterior = atual;
      atual = adquirirUnico();
    }
    
    valores[0] = atual;

    i = 1;
  } else { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger descendo, o sinal começa a ser adquirido "de verdade"
    
    int anterior = adquirirUnico(), atual = adquirirUnico();
    while (anterior <= Configuracoes.nivelTrigger || atual > Configuracoes.nivelTrigger) {
      anterior = atual;
      atual = adquirirUnico();
     
    }
    valores[0] = atual;
    i = 1;
  }

  //Serial.println(i);

  if (Configuracoes.microMinEntreAmostras == 0) {//adquirir o mais rápido possível
    for (; i < numeroAmostras; ++i) {
      valores[i] = adquirirUnico();
  
    }  
  } else {    
    unsigned int deltaT = 0;


    Relogio.reiniciar();
    
    while (i < numeroAmostras) {

      deltaT += Relogio.variacao();
      
      if (deltaT > Configuracoes.microMinEntreAmostras) {
        
        valores[i++] = adquirirUnico();
        deltaT -= Configuracoes.microMinEntreAmostras;
  
      }
      
    } 
      
  }
  
}

//Transmite os sinais para o computador
void transmitir(unsigned int *valores, unsigned int numeroAmostras) {

  float max = pow(2, Configuracoes.resolucao) - 1;
  int i;

  for (i = 0; i < numeroAmostras; ++i) {
    Serial.println(3.3*valores[i]/max);
    //Serial.println(valores[i]);
  }
}



void setup() {
  Serial.begin(115200);  

  valores = (unsigned int*) malloc(sizeof(unsigned int) * Configuracoes.numeroAmostras);

  for(int i = 0; i < Configuracoes.numeroAmostras; ++i) valores[i] = 0;
    
  setupADC();
  
  Serial.setTimeout(10);

  

}

void loop() {
  
  if (Serial.available()) {
      String comando = Serial.readStringUntil(' ');
      Serial.println(comando);
      
      if (comando == "LER") {
        adquirir(valores, Configuracoes.numeroAmostras);
        transmitir(valores, Configuracoes.numeroAmostras);
      } else if (comando == "TRIG") {
        comando = Serial.readStringUntil(' ');
        
      }
      
    }

}
