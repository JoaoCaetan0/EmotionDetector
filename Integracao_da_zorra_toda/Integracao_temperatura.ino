/*
Autores: Aaron Levi dos Santos Palma e João Pedro de Oliveira Caetano
Data: 09/12/21
Descrição: Código responsável pelo sensor de temperatura que tem como retorno a 
leitura em tempo real em graus celsius de onde estiver sendo usado o sensor.
*/

float TempAvg(int sensorPin){
  int total = 0;
  //digitalWrite(pinoLedR, 1);
 int amostras = 20;
  for (int i = 0; i <amostras; i++){
  total += analogRead(sensorPin);
  delay(20);  
  }
  return total/amostras;
}

float SensorTemp() {
  float temperatura = TempAvg(ADCPIN);
  temperatura = temperatura * 1100/(1024*10);
  return temperatura;
}
