#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2
#define ADCPIN 1
#include "wire_asukiaaa.h"


const int pinoLedG = 12;                   //PINO DIGITAL UTILIZADO PELO LED
const int pinoLedY = 13;                   //PINO DIGITAL UTILIZADO PELO LED
const int pinoLedR = 11;
const int pinoLedB = 10;

unsigned long tempoAnterior = 0;

int idade;


/////////////////////////////////////////////////////////////////////////////////////   // SENSOR CARDIACO //   //////////////////////////////////////////////////////////////////////////////




//int controleBpm = 0;
//volatile int bpmVetor[100];                
//int BpmAlto;                            // Batimento mais alto
//int BpmBaixo;                           // Batimento mais baixo
int bpmMedio;                             // Média entre o Batimento mais alto e o mais baixo

int pulsePin = 0;                         // Cabo do sensor de batimentos conectado na porta analógica pin 0
int fadePin = 5;                          // pin que esmaece
int fadeRate = 0;                         // define a frequência de esmaecimento do LED
static int outputType = SERIAL_PLOTTER;
volatile int rate[10];                    // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0; // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;  // used to find IBI
volatile int P =512;                      // used to find peak in pulse wave, seeded
volatile int T = 512;                     // used to find trough in pulse wave, seeded
volatile int thresh = 530;                // used to find instant moment of heart beat, seeded
volatile int amp = 0;                     //used to hold amplitude of pulse waveform, seeded
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = false;      // used to seed rate array so we startup with reasonable BPM
volatile int BPM;                         // int para o valor da leitura do sensor. Atualiza a cada 2mS
volatile int Signal;                      // Segura os dados que serão processados
volatile int IBI = 600;                   // int que armazena os intervalos entre as batidas!
volatile boolean Pulse = false;           // "True" Quando uma batida é detectada. "False" quando não é
volatile boolean QS = false;              // Recebe "True" quando uma batida é uma detectada.



//////////////////////////////////////////////////////////////////////////////////////////////////     // FIM CARDIACO //   //////////////////////////////////////////////////////////////////


boolean agitacao;
float temperatura = 0;
float temperaturaMedia = 0;
int movimento = 0;
int movimentoMedio = 0;// incerto se é ou não INT --------
const int pinoMicroondas = 8; //PINO DIGITAL UTILIZADO PELO SENSOR

                                            // Variáveis para testes
                        //int BpmH[10] = {150,80,70,0,0,0,0,0,0,0};     // Em teste
                        //int BpmL[10] = {0,0,0,0,0,0,0,0,0,0};     // Em teste

int configuracao = 0;


//////////////////////////////////////////////////////////////////////////////////////////    // GIROSCÓPIO //    ////////////////////////////////////////////////////////////////////////////

                          
const int MPU_ADDR=0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
 int16_t temperature; // variables for temperature data

#define LED_LB 2 // LED left bottom
#define LED_RB 3 // LED right bottom
#define LED_RT 4 // LED right top
#define LED_LT 5 // LED left top

char tmp_str[7]; // temporary variable used in convert function


//////////////////////////////////////////////////////////////////////////////////////////    // FIM GIROSCÓPIO //    ////////////////////////////////////////////////////////////////////////

                          

void setup() {
  pinMode(pinoLedR,OUTPUT);
  
  pinMode(pinoLedY, OUTPUT);
  pinMode(pinoLedG,OUTPUT);                 // pin que pisca quando detecta um batimento!
  pinMode(pinoLedB, OUTPUT);
  pinMode(fadePin,OUTPUT);                 // pin que faz o efeito de esmaecer!
  pinMode (pinoMicroondas, INPUT);         //DEFINE O PINO COMO ENTRADA
  analogReference(INTERNAL);              //Referência analógica interna para maior precisão
  Serial.begin(115200);                   // velocidade de leitura do sensor
  pinMode(LED_LB, OUTPUT);
  pinMode(LED_RB, OUTPUT);
  pinMode(LED_RT, OUTPUT);
  pinMode(LED_LT, OUTPUT);
  digitalWrite(LED_LB, LOW);
  digitalWrite(LED_RB, LOW);
  digitalWrite(LED_RT, LOW);
  digitalWrite(LED_LT, LOW);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  interruptSetup();
  
}

void loop() {                  // responde com o dado recebido:
   serialOutput();  
  while(configuracao == 0){            
    Serial.println(millis());          
    Serial.println("Configurando...");
    digitalWrite(pinoLedY, 1);
    digitalWrite(pinoLedG, 0);
    Serial.println(movimento);
    movimento = detectaMovimento(movimento);
    Serial.println(movimento);
    temperatura = SensorTemp();
    Serial.print("A temperatura é: ");
    Serial.println(temperatura);
    agitacao = detectaAgitacao(); 
    if ((millis() - tempoAnterior) < 1000){
      Serial.print(configuracao + 5); 
      tempoAnterior= millis();
      movimentoMedio = movimento/15;                          //Movimento médio é a taxa de movimento por segundo (Fórmula = dM/dT)
      movimento = 0;
      idade = 18;
     bpmMedio = mediaBpm(BPM, idade);                        //Chama função que calcula Média dos valores lidos com o esperado pela idade
      temperaturaMedia = temperatura; 
        // Passando pelo console
      configuracao = 0;
      Serial.println("Configurado com sucesso!");
      delay(2000);
      digitalWrite(pinoLedY, 0);  
    }  
    configuracao = 1; 
    Serial.println("pastel");                  
  }
  Serial.print("BPM É:");
  serialOutputWhenBeatHappens(); 
  //====================================== Árvore de decisões ======================================================
  //Árvore Re-formatada

  float acompanhamentoExpressivo = 0;
  int controle = 0;//Variável para monitorar a variação nociva

  //Aumento de 15% no batimento cardíaco
  if(BPM >= (bpmMedio * 1.15)) {  


    //Aumento expressivo de 50%
    if(BPM > (bpmMedio * 1.5)) {
      acompanhamentoExpressivo = 0;
      controle = 0;//Variável para monitorar a variação nociva
      movimento = detectaMovimento(movimento); //Reseta variáveis
      temperatura = SensorTemp();
      agitacao = detectaAgitacao();

      //Monitoramento dedicado de 5 segundos armazenando 100 leituras
      while ( controle < 100){             
        acompanhamentoExpressivo= BPM + acompanhamentoExpressivo;
        controle++;
        delay(50);                            //delay de 50ms 
      } //Fecha Monitoramento


      //Houve aumento expressivo bpm
      if (acompanhamentoExpressivo/100 >= bpmMedio * 1.45){

        //Aumento de 15% na taxa de movimento
        if (((movimento/(millis() - tempoAnterior))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Sem emoção, apenas exercício!");

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/(millis() - tempoAnterior)) < ((movimentoMedio * 1.15))) && (((movimento/(millis() - tempoAnterior)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/(millis() - tempoAnterior))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento

      }//Fecha houve aumento expressivo BPM


      //Não Houve Aumento Expressivo
      else{
        //Aumento de 15% na taxa de movimento
        if (((movimento/(millis() - tempoAnterior))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/(millis() - tempoAnterior)) < ((movimentoMedio * 1.15))) && (((movimento/(millis() - tempoAnterior)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/(millis() - tempoAnterior))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento


      } //Fecha (else) não houve aumento

    }//Fecha Aumento Expressivo

  }//Fecha aumento cardíaco de 15%

  //Tolerância cardíaca
  if((BPM < (bpmMedio * 1.15)) && (BPM >= (bpmMedio * 0.85))){
    
    //Aumento de 15% na taxa de movimento
        if (((movimento/(millis() - tempoAnterior))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/(millis() - tempoAnterior)) < ((movimentoMedio * 1.15))) && (((movimento/(millis() - tempoAnterior)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/(millis() - tempoAnterior))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento

  }//Fecha Tolerância cardíaca

  //Queda de 15% no BPM
  if(BPM < (bpmMedio * 0.85)) {


    //Queda expressiva de 50%
    if(BPM > (bpmMedio * 1.5)) { 
      acompanhamentoExpressivo = 0;
      controle = 0;//Variável para monitorar a variação nociva
      movimento = detectaMovimento(movimento); //Reseta variáveis
      temperatura = SensorTemp();
      agitacao = detectaAgitacao();

      //Monitoramento dedicado de 5 segundos armazenando 100 leituras
      while ( controle < 100){             
        acompanhamentoExpressivo= BPM + acompanhamentoExpressivo;
        controle++;
        delay(50);                            //delay de 50ms 
      } //Fecha Monitoramento
      
      //Houve queda expressiva bpm
      if (acompanhamentoExpressivo/100 <= bpmMedio * 0.65){

        //Aumento de 15% na taxa de movimento
        if (((movimento/(millis() - tempoAnterior))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/(millis() - tempoAnterior)) < ((movimentoMedio * 1.15))) && (((movimento/(millis() - tempoAnterior)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/(millis() - tempoAnterior))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento

      }//Fecha houve queda expressivo BPM


      //Não Houve queda Expressivo
      else{
        //Aumento de 15% na taxa de movimento
        if (((movimento/(millis() - tempoAnterior))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/(millis() - tempoAnterior)) < ((movimentoMedio * 1.15))) && (((movimento/(millis() - tempoAnterior)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/(millis() - tempoAnterior))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){

              }//Fecha Agitação
              //Não agitou
            else{


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação
            //Não agitou
            else{


            }//Fecha (else) Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){

            }//Fecha Agitação 
            //Não agitou
            else{


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento


      } //Fecha (else) não houve queda expressiva

      
    }//Fecha Queda Expressiva
    
  }//Fecha Queda
  
}//Fecha loop
