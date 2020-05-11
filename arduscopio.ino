#define PINO_AQUISICAO A0

#include "limits.h"

#include "relogio.h"
relogio Relogio;

#include "configuracoes.h"
configuracoes Configuracoes;

//Onde os valores medidos são armazenados. Pode ser armazenado em um unsigned short pois os valores nunca são negativos e, na resolução máxima, ocupam até 12 bits.
uint16_t* valores;

//manipula os registradores para permitir a leitura manual do ADC (sem usar analogRead)
unsigned int setupADC() {

  // Uma das referências usadas: http://frenki.net/2013/10/fast-analogread-with-arduino-due/

  //Liga o clock do ADC
  //isso provavelmente é feito por padrão, mas é melhor garantir

  //PMC_PCER1 é um registrador que controla se os clocks dos periféricos estão ligados.
  bitSet(PMC->PMC_PCER1, 5);
  //PMC_PCDR1 é um registrador que controla se os clocks dos periféricos estão desligados
  bitClear(PMC->PMC_PCDR1, 5);

  //basicamente, para garantir que o clock está ligado, é necessário que o bit correspondete
  //no primeiro registrador está em 1 e no segundo está em 0 

  //Valor padrão do registrador de configuração do ADC (ADC_MR): 272105984

  //inicializa registrador de configuração do ADC
  
  byte PRESCAL = 2;
  byte STARTUP = 8;
  byte SETTLING = 3;
  byte TRANSFER = 1;
  ADC->ADC_MR = (PRESCAL << 8) + (STARTUP << 16) + (SETTLING << 20) + (TRANSFER << 28);

  //ADC channel enable register: liga o canal 7, que corresponde ao pino ADC0 do arduino due

  // 0b0100000 = 0x80
  ADC->ADC_CHER = 0x80;

//  Serial.println((PRESCAL << 8) + (STARTUP << 16) + (SETTLING << 20) + (TRANSFER << 28));
//  Serial.println(ADC->ADC_MR);
}


unsigned int adquirirUnico() { //Função usada para fazer uma única leitura

  ADC->ADC_CR = 2;//Começa conversão
  while (!(ADC->ADC_ISR & 0x80)); //espera conversão
  //retorna o valor lido no canal 7 já com o valor adaptado para a resolução desejada usando shift para a direita 
  // (descartando os bits menos signifcativos se necessário)

  //TODO: esse jeito de fazer é burro, faça o shift depois de ler tudo
  return ADC->ADC_CDR[7] >> (12 - Configuracoes.resolucao);
}


//adquire os valores necessários de acordo com as configurações feitas
void adquirir(unsigned int *valores, unsigned int numeroAmostras) {

  int i;

  //acumulador do tempo passado entre ciclos, usado para controlar o tempo passado entre amostragens
  unsigned int deltaT = 0;
  
   if (Configuracoes.tipoTrigger == desativado) { //Sem trigger: Simplesmente começa a adquirir o sinal imediatamente
     i = 0;
   } else if (Configuracoes.tipoTrigger == subida) { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger subindo, o sinal começa a ser adquirido "de verdade"
    

    int anterior = adquirirUnico(), atual = adquirirUnico();

    Relogio.reiniciar();


    //Fica lendo até o último valor lido ser maior que o nivelTrigger e o penúltimo menor, ou seja, acabou de passar pelo valor do trigger e está subindo
    //ou se 10k*o tempo desejado entre amostras passar e o sinal não passar pelo trigger, começa a adquirir mesmo assim, para o sistema não ficar bloqueado.

    while ((anterior >= Configuracoes.nivelTrigger || atual < Configuracoes.nivelTrigger) && deltaT <= 10000*Configuracoes.microMinEntreAmostras) {
      anterior = atual;
      atual = adquirirUnico();
      deltaT += Relogio.variacao();
    }
    
    //guarda esse valor 
    valores[0] = atual;

    i = 1;
  } else { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger descendo, o sinal começa a ser adquirido "de verdade"
    
    //Mesma lógica que o trigger de subida
    
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


  //Se o tempo desejado entre amostras é 0, o sistema adquirirá valores o mais rápido possível
  if (Configuracoes.microMinEntreAmostras == 0) {
    //TODO: testar se é mais rápido usando aritmética de ponteiros
    for (; i < numeroAmostras; ++i) {
      valores[i] = adquirirUnico();
  
    }  
  } else {    


    //Fica esperando o tempo passar até o intervalo desde a última amostra ser maior do que o tempo desejado.
    //Para valores muito pequenos, não funcionará
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
    //converte valores do ADC para valores de tensão
    Serial.println(3.3*valores[i]/Configuracoes.valorMax);
    //Serial.println(valores[i]);
  }
}



void setup() {
  Serial.begin(115200);  


  //Linhas abaixo podem ser substituidas por calloc?

  //TODO: permitir configurar número de amostras
  valores = (uint16_t*) malloc(sizeof(uint16_t) * Configuracoes.numeroAmostras);

  //Inicializa valores em 0
  for(int i = 0; i < Configuracoes.numeroAmostras; ++i) valores[i] = 0;
    
  setupADC();
  
  Serial.setTimeout(10);

  

}


//Lê uma palavra da interface serial (definindo uma palavra como os caracteres entre dois espaços (' ')) e remove caracteres '\n' das palavras adquiridas
String adquirirPalavra() {
  String comando = Serial.readStringUntil(' ');
  
  if (comando[comando.length() - 1] == '\n') //se houver, remove new line
    comando.remove(comando.length() - 1);

   return comando;
}


void loop() {
  
  //Se há dados na interface serial, os lê 
  //e usa o comando passado para fazer alguma coisa
  if (Serial.available()) {
      String comando = adquirirPalavra();
      
      if (comando == "LER") {

        Serial.println(0);
        adquirir(valores, Configuracoes.numeroAmostras);
        transmitir(valores, Configuracoes.numeroAmostras);
      
      } else if (comando == "TRIG") {
      
        comando = adquirirPalavra();
        Configuracoes.setTrig(comando);
      
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
