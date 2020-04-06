#define PINO_AQUISICAO A0


enum configTrigger {
  desativado,
  subida,
  descida
};


struct conf {
    configTrigger tipoTrigger;
    unsigned int nivelTrigger;
  
    byte resolucao;
    unsigned long frequencia;

};

//struct global com configuracoes
struct conf Configuracoes;

unsigned int* valores;
unsigned int numeroAmostras;


//adquire os valores necessários de acordo com as configurações feitas
void adquirir(unsigned int *valores, unsigned int numeroAmostras) {
  int i;

  if (Configuracoes.tipoTrigger == desativado) {
    for (i = 0; i < numeroAmostras; ++i) {
      valores[i] = analogRead(PINO_AQUISICAO);
    }
    
  } else if (Configuracoes.tipoTrigger = subida) {
    int anterior = -1, atual = -1;
    while (anterior > Configuracoes.nivelTrigger || atual < Configuracoes.nivelTrigger) {
      atual = analogRead(PINO_AQUISICAO);
      anterior = atual;
    }
    
    valores[0] = atual;
    
    for (i = 1; i < numeroAmostras; ++i) {
      valores[i] = analogRead(PINO_AQUISICAO);
    }
  } else {
    int anterior = -1, atual = -1;
    while (anterior < Configuracoes.nivelTrigger || atual > Configuracoes.nivelTrigger) {
      atual = analogRead(PINO_AQUISICAO);
      anterior = atual;
    }
    
    valores[0] = atual;
    
    for (i = 1; i < numeroAmostras; ++i) {
      valores[i] = analogRead(PINO_AQUISICAO);
    }
  }
  

}

//Transmite os sinais para o computador
void transmitir(unsigned int *valores, unsigned int numeroAmostras) {
  int i;

  for (i = 0; i < numeroAmostras; ++i) {
    Serial.println(valores[i]);
  }
}



void setup() {
  Serial.begin(115200);  

  Configuracoes.tipoTrigger = descida;
  Configuracoes.nivelTrigger = 500;
  Configuracoes.resolucao = 12;
  Configuracoes.frequencia = 0;

  numeroAmostras = 500;

  valores = (unsigned int*) malloc(sizeof(unsigned int) * numeroAmostras);

  pinMode(PINO_AQUISICAO, INPUT);
  
  //cria sinal placeholder à amostrar
  pinMode(2, OUTPUT);
  analogWrite(2, 0.5);

  adquirir(valores, numeroAmostras);
  transmitir(valores, numeroAmostras);
}

void loop() {


 
}
