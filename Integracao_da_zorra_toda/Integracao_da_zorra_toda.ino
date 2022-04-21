#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2
#define ADCPIN 1
#include "wire_asukiaaa.h"


const int pinoLedG = 12;                   //PINO DIGITAL UTILIZADO PELO LED
const int pinoLedY = 13;                   //PINO DIGITAL UTILIZADO PELO LED
const int pinoLedR = 11;
const int pinoLedB = 10;

unsigned long int tempoAnterior = -1000;

int idade;


/////////////////////////////////////////////////////////////////////////////////////   // SENSOR CARDIACO //   //////////////////////////////////////////////////////////////////////////////

//int controleBpm = 0;
//volatile int bpmVetor[100];                
//int BpmAlto;                              // Batimento mais alto
//int BpmBaixo;                             // Batimento mais baixo
int BpmMedio;                             // Média entre o Batimento mais alto e o mais baixo

int pulsePin = 0;                         // Cabo do sensor de batimentos conectado na porta analógica pin 0
int fadePin = 5;                          // pin que esmaece
int fadeRate = 0;                         // define a frequência de esmaecimento do LED
static int outputType = SERIAL_PLOTTER;
volatile int rate[10];                    // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find IBI
volatile int P =512;                      // used to find peak in pulse wave, seeded
volatile int T = 512;                     // used to find trough in pulse wave, seeded
volatile int thresh = 530;                // used to find instant moment of heart beat, seeded
volatile int amp = 0;                   // used to hold amplitude of pulse waveform, seeded
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

int configuracao1 = 1;


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
  pinMode(pulsePin,INPUT);
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

void loop() {

  while(configuracao1 != 0){                        
      Configurando(tempoAnterior);
      movimento = detectaMovimento(movimento);
      temperatura = SensorTemp();
      agitacao = DetectaAgitacao(); 
      if ((millis() - tempoAnterior) < 15000){
        movimentoMedio = movimento/15;                          //Movimento médio é a taxa de movimento por segundo
        Serial.println("Digite sua idade: ");
        if (Serial.available() > 0) {                           // lê do buffer o dado recebido
          idade = Serial.read();
          Serial.print("idade recebida: ");                     // responde com o dado recebido:
          Serial.println(idade);
      }
        BpmMedio = mediaBpm(BPM, idade);                        //Chama função que calcula Média dos valores lidos com o esperado pela idade
        temperaturaMedia = temperatura; 

        // Passando pelo console
           
      configuracao1 = 0;
      Serial.println("Configurado com sucesso!");
      delay(2000);
      digitalWrite(pinoLedY, 0);  
        }
         delay(1);                   
      }


     serialOutput();       
     configuracao1 = 1;
}
