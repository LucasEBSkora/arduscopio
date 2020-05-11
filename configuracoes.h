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
    
    //tipo de trigger: pode ser subida, descida ou sem trigger
    configTrigger tipoTrigger;

    //qual é o nível do trigger. Pode ser qualquer valor entre 0 e valorMax
    unsigned int nivelTrigger;

    //Resolução em bytes lida do ADC. A resolução nativa é de 10 ou 12 bits
    byte resolucao;
    //menor tempo desejado entre amostras (se for muito pequeno não funcionará)
    unsigned long microMinEntreAmostras; 
    //Quantas amostras são tomadas por aquisição (números muito grandes podem encher a memória).
    unsigned int numeroAmostras;

    //Uma constante representando o máximo valor antes da conversão para float que a leitura pode assumir ((2 ^ resolucao) -1)
    unsigned int valorMax;
    
    //Construtor vazio, inicializa com valores padrão
    configuracoes();

    //Configura o tipo de Trigger. Pode receber os valores "DESATIVADO", "SUBIDA" e "DESCIDA"
    void setTrig(String comando);

    //Recebe um valor entre 0 e 3.3 e seta o valor de nivelTRigger de acordo com a resolução usada
    void setNivelTrigger(float nivel);
    
    //Seta a resolução. Pode receber valores entre 1 e 12. 
    void setRes(byte resolucao);

    //Seta o menor tempo desejado entre amostras
    void setMinTempo(int micrs);
};

#endif
