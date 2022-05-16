#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2
#define ADCPIN 1

#include "wire_asukiaaa.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const int pinoLedG = 12;                   //PINO DIGITAL UTILIZADO PELO LED
const int pinoLedY = 13;                   //PINO DIGITAL UTILIZADO PELO LED
const int pinoLedR = 9;
const int pinoLedB = 10;

unsigned long tempoAnterior = 0;



/////////////////////////////////////////////////////////////////////////////////////   // SENSOR CARDIACO //   //////////////////////////////////////////////////////////////////////////////


int idade = 18;
int tempo = 15000; //ms
//int controleBpm = 0;              
//int BpmAlto;                            // Batimento mais alto
//int BpmBaixo;                           // Batimento mais baixo
float bpmMedio;                             // Média entre o Batimento mais alto e o mais baixo

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
int movimentoMedio = 0;
const int pinoMicroondas = 8; //PINO DIGITAL UTILIZADO PELO SENSOR

                                            // Variáveis para testes
                        //int BpmH[10] = {150,80,70,0,0,0,0,0,0,0};     // Em teste
                        //int BpmL[10] = {0,0,0,0,0,0,0,0,0,0};     // Em teste

int configuracao = 0;


//////////////////////////////////////////////////////////////////////////////////////////    // GIROSCÓPIO //    ////////////////////////////////////////////////////////////////////////////

                          
const int MPU_ADDR=0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

Adafruit_MPU6050 mpu;

int controleMovimento =0; //impedir contagem de movimentos infinitos
int ultimoX = 0;
int ultimoY = 0;
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

void loop() {                  // responde com o dado recebido
 
  while(configuracao == 0){                     
    Serial.println("Configurando...");
    digitalWrite(pinoLedY, 1);
    digitalWrite(pinoLedR, 1);
    digitalWrite(pinoLedG, 1);
    digitalWrite(pinoLedB, 1);
    temperatura = SensorTemp();
    Serial.print("A temperatura é: ");
    Serial.println(temperatura);
    agitacao = detectaAgitacao();      
    while ((millis() - tempoAnterior) < tempo){       //Tempo = 15seg
      movimento = detectaMovimento(movimento);
      //Movimento médio é a taxa de movimento por segundo (Fórmula = dM/dT)
     bpmMedio = mediaBpm();                        //Chama função que calcula Média dos valores lidos com o esperado pela idade
     temperatura = SensorTemp();
     temperaturaMedia = temperatura; 
        // Passando pelo console
    }     
    tempoAnterior= millis();
    Serial.println("Configurado com sucesso!");
    delay(2000); 
    digitalWrite(pinoLedY, 0);
    digitalWrite(pinoLedR, 0);
    movimentoMedio = movimento/(tempo/1000); //Tempo/1000 para igualar escala
    Serial.println("movimentoMedio é: ");
    Serial.println(movimentoMedio);  

    Serial.println("BPM Médio é: ");
    Serial.println(bpmMedio); 

    Serial.println("Temperatura Média é: ");
        temperatura = SensorTemp();
     temperaturaMedia = temperatura; 
    Serial.println(temperaturaMedia);
    configuracao = 1;                  
  }
  Serial.print("BPM É:");
  serialOutput(); 
  Serial.print("Temperatura É:");
  temperatura = SensorTemp();
  Serial.println(temperatura);
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
        if (((movimento/((millis() - tempoAnterior))/1000)) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Sem emoção, apenas exercício físico!");
                piscaLed(pinoLedR);
                

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Aumento muito intenso de atividade corporal. Você está se exercitando?");
              piscaLed(pinoLedY);


            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura
            
            //Não houve aumento expressivo de temperatura
            else{
               //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Ansiedade detectada!");
                ligaLed(pinoLedY); delay(5000);

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha (else) Não agitou
              
            }//Fecha não houve aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha Agitação
            //Não agitou
            else{
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/((millis() - tempoAnterior)/1000)) < ((movimentoMedio * 1.15))) && (((movimento/((millis() - tempoAnterior)/1000)) >= (movimentoMedio * 0.85)))){          //Movimento/(- tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação indiferente
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);
              
            }//Fecha aumento expressivo de temperatura
            
            //Não houve aumento expressivo
            else{
              //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha (else) Não agitou
            }//Fecha não houve aumento expressivo

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação Indiferente
            Serial.println("Calmo");
            ligaLed(pinoLedB); delay(5000);

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Ansiedade detectada!");
                ligaLed(pinoLedY); delay(5000);

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);
            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

            //Não Houve aumento expressivo de temperatura
          else{
            
            //Agitação Indiferente
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);
            
            }//Fechaa não Houve aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);

            }//Fecha Agitação
            //Não agitou
            else{
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação Indiferente
            Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento

      }//Fecha houve aumento expressivo BPM


      //Não Houve Aumento Expressivo
      else{
        //Aumento de 15% na taxa de movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) >= (movimentoMedio * 1.15)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Ansiedade detectada!");
                ligaLed(pinoLedY); delay(5000);

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

            //Não houve aumento expressivo de temperatura
            else{
              
              if(detectaAgitacao() == true){
                Serial.println("Felicidade detectada!");
                ligaLed(pinoLedG); delay(5000);

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);

            }//Fecha (else) Não agitou
              }//Fecha não houve aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação indiferente
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedG); delay(5000);

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/((millis() - tempoAnterior))) < ((movimentoMedio * 1.15))) && (((movimento/((millis() - tempoAnterior)/1000)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha aumento expressivo de temperatura
            //Não houve aumento expressivo de temperatura
            else{
              
              //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha (else) Não agitou
              }//Fecha não houve aumento expressivo de temperatura
            
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha Agitação
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação indiferente
              Serial.println("Estresse detectado!");
              ligaLed(pinoLedR); delay(5000);

            }//Fecha aumento expressivo de temperatura

            //Não houve aumento expressivo de temperatura
            else{

              //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);

            }//Fecha (else) Não agitou
              }//Fecha não houve aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedG); delay(5000);

            }//Fecha Agitação
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);


            }//Fecha (else) Não agitou
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);
            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);


            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento


      } //Fecha (else) não houve aumento

    }//Fecha Aumento Expressivo

  }//Fecha aumento cardíaco de 15%

/*
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

  //Tolerância cardíaca
  if((BPM < (bpmMedio * 1.15)) && (BPM >= (bpmMedio * 0.85))){
    
    //Aumento de 15% na taxa de movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                
              Serial.println("Estresse");
              ligaLed(pinoLedR); delay(5000); //Acende o Led Vermelho
              //Liga o Led

              }//Fecha Agitação
              //Não agitou
            else{
              
              Serial.println("Ansiedade");
              ligaLed(pinoLedY); delay(5000); //Acende o Led Amarelo
              //Liga o Led

            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

            //Não Houve Aumento expressivo de temperatura
            else {

              //Agitação
              if(detectaAgitacao() == true){
                
              Serial.println("Ansieade");
              ligaLed(pinoLedY); delay(5000); //Acende o Led Amarelo
              //Liga o Led

              }//Fecha Agitação
              //Não agitou
            else{
              
              Serial.println("Estresse");
              ligaLed(pinoLedR); delay(5000); //Acende o Led Vermelho
              //Liga o Led

            }//Fecha (else) Não agitou
              
              }//Fecha não houve aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
              //Liga o Led

            }//Fecha Agitação
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
              //Liga o Led

            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade");
              ligaLed(pinoLedG); delay(5000); //Acende o Led Verde
              //Liga o Led

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
              //Liga o Led

            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/((millis() - tempoAnterior)/1000)) < ((movimentoMedio * 1.15))) && (((movimento/((millis() - tempoAnterior)/1000)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Estresse");
                Serial.println();
                ligaLed(pinoLedR); delay(5000); //Acende o Led Vermelho
                //Liga o Led

              }//Fecha Agitação
              //Não agitou
              else{
                Serial.println("Ansiedade");
                Serial.println();
                ligaLed(pinoLedY); delay(5000); //Acende o Led Amarelo
                //Liga o Led


              }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

              //Não Houve Aumento expressivo de temperatura
            else {

              //Agitação
              if(detectaAgitacao() == true){
                
              Serial.println("Felicidade");
              Serial.println();
              ligaLed(pinoLedG); delay(5000); //Acende o Led Verde
              //Liga o Led

              }//Fecha Agitação
              //Não agitou
            else{
              
              Serial.println("Ansiedade");
              Serial.println();
              ligaLed(pinoLedY); delay(5000); //Acende o Led Amarelo
              //Liga o Led

            }//Fecha (else) Não agitou
              
              }//Fecha não houve aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade");
              Serial.println();
              ligaLed(pinoLedG); delay(5000); //Acende o Led Verde
              //Liga o Led

            }//Fecha Agitação
            //Não agitou
            else{
             Serial.println("Calmo");
             Serial.println();
             ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
              //Liga o Led


            }//Fecha (else) Não agitou

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){
            //Agitação Indiferente neste caso
             Serial.println("Calmo");
             Serial.println();
             ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
             //Liga o Led

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento


        //Queda no movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Ansiedade");
                Serial.println();
                ligaLed(pinoLedY); delay(5000); //Acende o Led Amarelo
                //Liga o Led

              }//Fecha Agitação
              //Não agitou
            else{
               Serial.println("Instabilidade fisiológica (Temperatura muito alta)");
               Serial.println();
               
               for (int i = 0; i< 20; i++) {      //Pisca os LEDS Durante 10Segundos
                 ligaLed(pinoLedY); delay(5000); //Acende o Led Amarelo
                 ligaLed(pinoLedR); delay(5000); //Acende o Led Vermelho
                 delay(500);
                 digitalWrite(pinoLedY, LOW); //Acende o Led Amarelo
                 digitalWrite(pinoLedR, LOW); //Acende o Led Vermelho
                 movimento = detectaMovimento(movimento); //Reseta variáveis 10 vezes
                 temperatura = SensorTemp();
                 agitacao = detectaAgitacao(); 
                 
                } //Fecha o For de piscar



            }//Fecha (else) Não agitou

            }//Fecha aumento expressivo de temperatura

            //Não Houve Aumento expressivo de temperatura
            else {

              //Agitação
              if(detectaAgitacao() == true){
                
              Serial.println("Felicidade");
              Serial.println();
              ligaLed(pinoLedG); delay(5000); //Acende o Led Verde
              //Liga o Led

              }//Fecha Agitação
              //Não agitou
            else{
              
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
              //Liga o Led

            }//Fecha (else) Não agitou
              
              }//Fecha não houve aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação Indiferente neste caso
            Serial.println("Calmo");
            ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
            //Liga o Led

            
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Felicidade");
              ligaLed(pinoLedB); delay(5000); //Acende o Led Verde
              //Liga o Led

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000); //Acende o Led Azul
              //Liga o Led
              
            }//Fecha Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento

  }//Fecha Tolerância cardíaca


/*
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */
 
  //Queda de 15% no BPM
  if(BPM < (bpmMedio * 0.85)) {

        //Aumento de 15% na taxa de movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) >= (movimentoMedio * 1.15)){


          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){
              
              //Agitação indiferente
              Serial.println("Ansiedade detectada!");
              ligaLed(pinoLedY); delay(5000);

            }//Fecha aumento expressivo de temperatura
            //Não houve aumento expressivo de temperatura
            else{

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Felicidade detectada!");
                ligaLed(pinoLedG); delay(5000);

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha (else) Não agitou
              
            }//Fecha não houve aumento expressivo de temperatura
          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação Indiferente
              Serial.println("Felicidade detectada!");
              ligaLed(pinoLedB); delay(5000);

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação Indiferente
            Serial.println("Calmo");
            ligaLed(pinoLedB); delay(5000);

          }//Fecha Queda de temperatura

        }//Fecha aumento do movimento

        //Tolerância de 15% no movimento
        if(((movimento/((millis() - tempoAnterior)/1000)) < ((movimentoMedio * 1.15))) && (((movimento/((millis() - tempoAnterior)/1000)) >= (movimentoMedio * 0.85)))){          //Movimento/(millis() - tempoAnterior) é o movimento pelo tempo         

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){
              Serial.println("Anomalia de leitura");
              piscaLed(pinoLedR);


            }//Fecha aumento expressivo de temperatura
            //Não houve aumento expressivo de temperatura
            else{

              //Agitação
              if(detectaAgitacao() == true){
                Serial.println("Felicidade detectada!");
              ligaLed(pinoLedB); delay(5000);

              }//Fecha Agitação
              //Não agitou
            else{
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha (else) Não agitou
              
            }//Fecha não houve aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação Indiferente
            Serial.println("Calmo");
            ligaLed(pinoLedB); delay(5000);
              
          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);
              
            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Tristeza detectada! (Experimental)");
              piscaLed(pinoLedR);
              digitalWrite(pinoLedB, 1);
              digitalWrite(pinoLedY, 1);
            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Tolerância de movimento

        //Queda no movimento
        if (((movimento/((millis() - tempoAnterior)/1000))) < (movimentoMedio * 0.85)){

          //Aumento de 5% na temperatura
          if (temperatura > temperaturaMedia * 1.05){
            movimento = detectaMovimento(movimento); //Reseta variáveis
            temperatura = SensorTemp();
            agitacao = detectaAgitacao(); 

            //Aumento expressivo de temperatura
            if(temperatura >= temperaturaMedia * 1.15){
              Serial.println("Anomalia de leitura");
              piscaLed(pinoLedR);

            }//Fecha aumento expressivo de temperatura
            //Não houve aumento expressivo de temperatura
            else{
              //Agitação Indiferente
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);
               
            }//Fecha não houve aumento expressivo de temperatura

          }//Fecha aumento de temperatura

          //Tolerância de temperatura
          if ((temperatura >= temperaturaMedia * 0.95) && (temperatura <= temperaturaMedia * 1.05)){

            //Agitação Indiferente

              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

          }//Fecha Tolerância de temperatura

          //Queda de 5% na temperatura
          if (temperatura < temperaturaMedia * 0.95){

            //Agitação
            if(detectaAgitacao() == true){
              Serial.println("Calmo");
              ligaLed(pinoLedB); delay(5000);

            }//Fecha Agitação 
            //Não agitou
            else{
              Serial.println("Anomalia de leitura. Você está bem?");
              piscaLed(pinoLedB);
              digitalWrite(pinoLedB, 1);
              digitalWrite(pinoLedY, 1);
              digitalWrite(pinoLedG, 1);

            }//Fecha (else) Não agitou

          }//Fecha Queda de temperatura

        }//Fecha Queda no movimento

  }//Fecha Queda
  
}//Fecha loop
