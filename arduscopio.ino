#define PINO_AQUISICAO A0

#include "limits.h"

#include "relogio.h"
relogio Relogio;

#include "configuracoes.h"
configuracoes Configuracoes;

//Onde os valores medidos são armazenados. Pode ser armazenado em um unsigned short pois os valores nunca são negativos e, na resolução máxima, ocupam até 12 bits.
uint16_t* valores;

//guarda o tempo usado para medir todas as amostras.
uint32_t tempo;

//manipula os registradores para permitir a leitura manual do ADC (sem usar analogRead)
unsigned int setupADC() {

  // Uma das referências usadas: http://frenki.net/2013/10/fast-analogread-with-arduino-due/
  //essse site caiu, tem a referência salva em texto

  //Liga o clock do ADC
  //isso provavelmente é feito por padrão, mas é melhor garantir

  //PMC_PCER1 é um registrador que controla se os clocks dos periféricos estão ligados.
  bitSet(PMC->PMC_PCER1, 5);
  //PMC_PCDR1 é um registrador que controla se os clocks dos periféricos estão desligados
  bitClear(PMC->PMC_PCDR1, 5);

  //basicamente, para garantir que o clock está ligado, é necessário que o bit correspondente
  //no primeiro registrador está em 1 e no segundo está em 0 

  //Valor padrão do registrador de configuração do ADC (ADC_MR): 272105984

  //inicializa registrador de configuração do ADC
  
  byte PRESCAL = 0;
  byte FREERUN = 1;
  byte STARTUP = 1;
  byte SETTLING = 3;
  byte TRANSFER = 3;
  ADC->ADC_MR = (FREERUN << 7) +  (PRESCAL << 8) + (STARTUP << 16) + (SETTLING << 20) + (TRANSFER << 28);

  //ADC channel enable register: liga o canal 7, que corresponde ao pino ADC0 do arduino due

  // 0b0100000 = 0x80
  ADC->ADC_CHER = 0x80;

//  Serial.println((PRESCAL << 8) + (STARTUP << 16) + (SETTLING << 20) + (TRANSFER << 28));
//  Serial.println(ADC->ADC_MR);
}


unsigned int adquirirUnico() { //Função usada para fazer uma única leitura

  //Essas linhas só são importantes fora do modo free-running
  //ADC->ADC_CR = 2;//Começa conversão
  //while (!(ADC->ADC_ISR & 0x80)); //espera conversão 
  
  return ADC->ADC_CDR[7];
}


//adquire os valores necessários de acordo com as configurações feitas
void adquirir(uint16_t *valores, unsigned int numeroAmostras) {

  uint32_t timeout = (Configuracoes.microMinEntreAmostras != 0 ? Configuracoes.microMinEntreAmostras : 1) * 10000; 
  int i;
  
   if (Configuracoes.tipoTrigger == desativado) { //Sem trigger: Simplesmente começa a adquirir o sinal imediatamente
     i = 0;
     tempo = micros();
   } else if (Configuracoes.tipoTrigger == subida) { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger subindo, o sinal começa a ser adquirido "de verdade"
    
    int anterior = adquirirUnico(), atual = adquirirUnico();
    Relogio.reiniciar();

    //Fica lendo até o último valor lido ser maior que o nivelTrigger e o penúltimo menor, ou seja, acabou de passar pelo valor do trigger e está subindo
    //ou se 10k*o tempo desejado entre amostras passar e o sinal não passar pelo trigger, começa a adquirir mesmo assim, para o sistema não ficar bloqueado.

    while ((anterior >= Configuracoes.nivelTrigger || atual < Configuracoes.nivelTrigger) && Relogio.variacao() <= timeout) {
      anterior = atual;
      tempo = micros();
      atual = adquirirUnico();
    }
    
    //guarda esse valor 
    valores[0] = atual;

    i = 1;
  } else { //Com trigger: quando o sinal passar do valor de Configuracoes.nivelTrigger descendo, o sinal começa a ser adquirido "de verdade"
    
    //Mesma lógica que o trigger de subida
    int anterior = adquirirUnico(), atual = adquirirUnico();
    Relogio.reiniciar();
    
    while ((anterior <= Configuracoes.nivelTrigger || atual > Configuracoes.nivelTrigger)  && Relogio.variacao() <= timeout) {
      anterior = atual;
      tempo = micros();
      atual = adquirirUnico();
    }
    
    valores[0] = atual;
    i = 1;
  }
  //Se o tempo desejado entre amostras é 0, o sistema adquirirá valores o mais rápido possível
  if (Configuracoes.microMinEntreAmostras == 0) {
    for (; i < numeroAmostras; ++i) {
      valores[i] = adquirirUnico();
    }  
  } else {    
    //Fica esperando o tempo passar até o intervalo desde a última amostra ser maior do que o tempo desejado.
    //Para valores muito pequenos, não funcionará
    Relogio.reiniciar();

    while (i < numeroAmostras) {
        
        if (Relogio.variacao() >= Configuracoes.microMinEntreAmostras) {
          valores[i++] =ADC->ADC_CDR[7];
          Relogio.reiniciar();
        }
    }
  }
  tempo = micros() - tempo;

  //faz o shift lógico para manter a resolução desejada
  for (int i = 0; i < numeroAmostras; ++i) {
    valores[i] = valores[i] >> (12 - Configuracoes.resolucao);
  }
}

//Transmite os sinais para o computador
void transmitir(uint16_t *valores, unsigned int numeroAmostras) {
    Serial.write((byte*)valores, 2*numeroAmostras);
    Serial.write((byte*)&tempo, 4);  
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
  
  if (comando[comando.length() - 1] == '\n') //se houver, remove \n
    comando.remove(comando.length() - 1);

   return comando;
}


void loop() {
  //Se há dados na interface serial, os lê 
  //e usa o comando passado para fazer alguma coisa
  if (Serial.available()) {
      String comando = adquirirPalavra();
      
      if (comando == "LER") {
        
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
        Configuracoes.setRes(atoi(comando.c_str()));
      
      } else if (comando == "MINTEMPO") {
      
        comando = adquirirPalavra();
        Configuracoes.setMinTempo(atoi(comando.c_str()));
      
      }
      
    }

}
