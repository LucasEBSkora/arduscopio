#define PINO_AQUISICAO A0

#include "limits.h"

#include "relogio.h"
relogio Relogio;

#include "configuracoes.h"
configuracoes Configuracoes;


unsigned int* valores;

unsigned int setupADC() {

  //http://frenki.net/2013/10/fast-analogread-with-arduino-due/
  //Liga o clock do ADC
  bitSet(PMC->PMC_PCER1, 5);
  bitClear(PMC->PMC_PCDR1, 5);

  //272105984

  //inicializa registrador de configuração do ADC
  
  byte PRESCAL = 2;
  byte STARTUP = 8;
  byte SETTLING = 3;
  byte TRANSFER = 1;
  ADC->ADC_MR = (PRESCAL << 8) + (STARTUP << 16) + (SETTLING << 20) + (TRANSFER << 28);

  ADC->ADC_CHER = 0x80;

//  Serial.println((PRESCAL << 8) + (STARTUP << 16) + (SETTLING << 20) + (TRANSFER << 28));
//  Serial.println(ADC->ADC_MR);
}


unsigned int adquirirUnico() { //Função usada para fazer uma única leitura

  ADC->ADC_CR = 2;//Começa conversão
  while (!(ADC->ADC_ISR & 0x80)); //espera conversão
  return ADC->ADC_CDR[7] >> (12 - Configuracoes.resolucao);
}


//adquire os valores necessários de acordo com as configurações feitas
void adquirir(unsigned int *valores, unsigned int numeroAmostras) {
  int i;
  unsigned int deltaT = 0;
  
   if (Configuracoes.tipoTrigger == desativado) {
     i = 0;
   } else if (Configuracoes.tipoTrigger == subida) { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger subindo, o sinal começa a ser adquirido "de verdade"
    
    int anterior = adquirirUnico(), atual = adquirirUnico();

    Relogio.reiniciar();
    
    while ((anterior >= Configuracoes.nivelTrigger || atual < Configuracoes.nivelTrigger) && deltaT <= 10000*Configuracoes.microMinEntreAmostras) {
      anterior = atual;
      atual = adquirirUnico();
      deltaT += Relogio.variacao();
    }
    
    valores[0] = atual;

    i = 1;
  } else { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger descendo, o sinal começa a ser adquirido "de verdade"
    
    int anterior = adquirirUnico(), atual = adquirirUnico();

    Relogio.reiniciar();
    
    while ((anterior <= Configuracoes.nivelTrigger || atual > Configuracoes.nivelTrigger) && deltaT <= 10000*Configuracoes.microMinEntreAmostras) {
      anterior = atual;
      atual = adquirirUnico();
      deltaT += Relogio.variacao(); 
    }
    valores[0] = atual;
    i = 1;
  }

  if (Configuracoes.microMinEntreAmostras == 0) {//adquirir o mais rápido possível
    for (; i < numeroAmostras; ++i) {
      valores[i] = adquirirUnico();
  
    }  
  } else {    



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


  int i;

  for (i = 0; i < numeroAmostras; ++i) {
    Serial.println(3.3*valores[i]/Configuracoes.valorMax);
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

String adquirirPalavra() {
  String comando = Serial.readStringUntil(' ');
  
  if (comando[comando.length() - 1] == '\n') //se houver, remove new line
    comando.remove(comando.length() - 1);

   return comando;
}


void loop() {
  
  if (Serial.available()) {
      String comando = adquirirPalavra();
      
      if (comando == "LER") {
        Serial.println(0);
        adquirir(valores, Configuracoes.numeroAmostras);
        transmitir(valores, Configuracoes.numeroAmostras);
      } else if (comando == "TRIG") {
        comando = adquirirPalavra();
        Configuracoes.setTrig(comando);
        //Serial.println(Configuracoes.tipoTrigger); 
      } else if (comando == "TRIGNIVEL") {
        comando = adquirirPalavra();
        Configuracoes.setNivelTrigger(atof(comando.c_str()));
        
      } else if (comando == "RESOLUCAO") {
        comando = adquirirPalavra();
        Configuracoes.setRes(int(comando.c_str()));
      } else if (comando == "MINTEMPO") {
        comando = adquirirPalavra();
        Configuracoes.setMinTempo(atoi(comando.c_str()));
      }
      
    }

}
