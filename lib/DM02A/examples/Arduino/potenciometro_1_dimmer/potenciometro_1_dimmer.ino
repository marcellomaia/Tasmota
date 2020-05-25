/*
EXEMPLO: ACIONAMENTO DE APENAS 1 MÓDULO DIMMER DM02A DE 2 CANAIS COM USO DE 2 POTENCIÔMETROS

LIGAÇÕES DOS POTENCIÔMETROS AO ARDUINO
  *POTENCIÔMETRO 1: CONECTE O TERMINAL EXTREMO ESQUERDO DO POTENCIÔMETRO NO GND, CONECTE O TERMINAL EXTREMO DIREITO NO 5V (SE INVERTER, SÓ IRÁ MUDAR O SENTIDO QUE IRÁ AUMENTAR OU DIMINUIR O NÍVEL), 
    CONECTE O TERMINAL CENTRAL DESSE MESMO POTENCIÔMETRO NO PINO DE ENTRADA ANALÓGICA A0 DO ARDUINO

  *POTENCIÔMETRO 2: CONECTE O TERMINAL EXTERMO ESQUERDO DO POTENCIÔMETRO NO GND, CONECTE O TERMINAL EXTREMO DIREITO NO 5V (SE INVERTER, SÓ IRÁ MUDAR O SENTIDO QUE IRÁ AUMENTAR OU DIMINUIR O NÍVEL), 
    CONECTE O TERMINAL CENTRAL DESSE MESMO POTENCIÔMETRO NO PINO DE ENTRADA ANALÓGICA A1 DO ARDUINO
  
USO:
  GIRE O POTENCIÔMETRO 1, O CANAL 1 IRÁ MUDAR O NÍVEL DA SAÍDA
  GIRE O POTENCIÔMETRO 2, O CANAL 2 IRÁ MUDAR O NÍVEL DA SAÍDA
    
LIGAÇÕES DO MÓDULO AO ARDUINO:
  GND: CONECTAR AO GND DO ARDUINO
  VCC: CONECTAR AO PINO DE SAÍDA DE 5V
  SIG: CONECTAR AO PINO 12 DO ARDUINO
  CH: CONECTAR AO PINO 11 DO ARDUINO
  EN: SE FOR USAR APENAS UM MÓDULO, DEIXAR SEM CONEXÃO
  B1+, B1-, B2+, B2-: SÓ USAR SE DESEJAR CONECTAR INTERRUPTORES EXTERNOS E NESSE CASO DEVEM CHAVEAR O GND
  
LIGAÇÕES DO MÓDULO À REDE/CARGA:

  FASE: LIGAR A FASE 127V OU 220V
  NEUTRO: LIGAR O NEUTRO DA REDE OU FASE 2 (TODOS OS BORNES 'NEUTRO' ESTÃO INTERLIGADOS, ENTÃO USE UM PARA ENTRAR COM O NEUTRO E OS DEMAIS PARA SAIR PARA A CARGA)
  SAÍDA 2: SAÍDA 2 DO MÓDULO
  SAÍDA 1: SAÍDA 1 DO MÓDULO  
  
  
DURAÇÃO DOS PULSOS NO PINO 'SIG':

  Duração entre 500uS e 35000uS: Define o nível atual, sendo cada 500uS 1 nível, totalizando 70 níveis
  Menos de 500uS: Nível 0, saída desligada
  Entre 40000uS e 50000uS: Solicita o nível atual do canal em selecionado. Após enviar o pulso, coloque o pino que enviou o pulso como entrada. 100uS após receber o comando, o dimmer
	colocará o pino SIG em nível 0, passados mais 100uS o pino SIG enviará um pulso com duração correspondente ao nível atual do canal, sendo: 500uS para o nível 1, 35000uS para o nível 70 e 100uS para o nível 0.
*/

#include <DM02A.h>  //Inclui a biblioteca para comunicar com o dimmer DM02A

//Cria um novo objeto
DM02A dimmer(12, 11);//SIG, CH, EN - Se não usar o EN, deixar sem informar

/*Comandos disponíveis: 
  dimmer.EnviaNivel(nivel, canal);  //Define o nível no canal selecionado
  dimmer.feedback(canal); //Retorna o nível do canal selecionado
*/

char pot1 = A0;  //Pinos conectados aos potenciômetros
char pot2 = A1;

unsigned int med = 0;  //Armazena a média das leituras no pino de entrada analógica
char cont = 0; //Variável auxiliar

unsigned long last_tempo = 0; ////Armazena o momento da última verificação dos potenciômetros, a função time() reinicia em 50 dias com o Arduino constantemente ligado

int nivel_pot1_tmp = 0;
int nivel_pot2_tmp = 0;

int last_nivel_pot1 = 0; //Armazena o último valor lido no potenciômetro
int last_nivel_pot2 = 0;

void setup(){
  //Força os dois canais iniciarem desligados
  dimmer.EnviaNivel(0,0);
  dimmer.EnviaNivel(0,1);
  //analogReference(DEFAULT); //5V para placas que funcionam com 5V, se sua placa usar 3.3V, precisará refazer os cálculos
}

void loop(){
  //O valor retornado pela leitura nas portas analógicas serão entre 0 e 1023, sendo 1024 possibilidades. Os níveis para o dimmer são entre 0 e 70
  //A cada 14 passos na leitura analógica iremos considerar um nível do dimmer, então chegará no limite em 980 (14 x 70);

  //A cada 500ms verifica se algum dos potenciômetros mudaram de posição
  if(millis() > (last_tempo+500)){//Verifica se já se passou 500ms, se sim, verifica se houve atualização do nível pelos botões do módulo dimmer ou por chaves externas
    last_tempo = millis();//Atualiza o tempo registrado para o tempo atual
    nivel_pot1_tmp = lePot(pot1);//Lê o valor do potenciômetro 1
    if(last_nivel_pot1 != nivel_pot1_tmp){
      delay(100);
      nivel_pot1_tmp = lePot(pot1);//Lê novamente o valor do potenciômetro 1
      if(last_nivel_pot1 != nivel_pot1_tmp){
        last_nivel_pot1 = nivel_pot1_tmp;//Atualiza o nível lido no potenciômetro
        dimmer.EnviaNivel((nivel_pot1_tmp/14),0); //Atualiza o nível no dimmer
      }
    }

    nivel_pot2_tmp = lePot(pot2);//Lê o valor do potenciômetro 1
    if(last_nivel_pot2 != nivel_pot2_tmp){
      delay(100);
      nivel_pot2_tmp = lePot(pot2);//Lê novamente o valor do potenciômetro 1
      if(last_nivel_pot2 != nivel_pot2_tmp){
        last_nivel_pot2 = nivel_pot2_tmp;//Atualiza o nível lido no potenciômetro
        dimmer.EnviaNivel((nivel_pot2_tmp/14),1); //Atualiza o nível no dimmer
      }
    }
  }  
}

//Faz 40 leituras no pino de entrada analógica e faz uma média.
//Isso se faz necessário porque os valores lidos no pino de entrada analógica tem variações
int lePot(char _pot){
    cont = 0;
    med = 0;
    while(cont < 40){
      med = med + analogRead(_pot);
      delay(1);
      cont = cont + 1; 
    }
    med = med / 40;
    return med;
}
