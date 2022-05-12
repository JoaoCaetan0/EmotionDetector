int bpmIdade(int idade){
  int bpmEsperado = 0;
  if (idade <= 7){
    bpmEsperado = 120;    // Média entre 100 - 140
  }
  if ((idade <= 17) && (idade >= 8)){
    bpmEsperado = 90;     // Média entre 100 - 80
  }
  if ((idade <= 65) && (idade >= 18)){
    bpmEsperado = 75;     // Média entre 70 - 80
  }
  if (idade > 65){
    bpmEsperado = 55;     //Média entre 50 - 60 
  }
  return bpmEsperado;
}

float mediaBpm( int idade){ 
  int controleBpm = 0;
  int bpmVetor[100];
  float BpmMedio = 0.00;
  int conf = 0;
  while (conf != 1){
  if ((BPM > 40 && BPM < 150) && (controleBpm < 100)){                      //Limpar ruídos da medição
    bpmVetor[controleBpm] = BPM;
    controleBpm++;
    conf = 1;
      }
      else{
        Serial.println("Erro de leitura nos BPM: ");
        Serial.print(BPM);                               
      }
         int soma = 0;
         for(int i = 0; i<100; i++){
           soma = soma + bpmVetor[i];
         }
         BpmMedio = soma/100;         //Média primária com 100 elementos       
  }        
       BpmMedio = (bpmIdade(idade) + BpmMedio)/2;   
       return BpmMedio;
}

void ledPiscando(int led){
  
  //desliga os leds
  for(int i = 10; i <= 13; i++){
    digitaWrite(i, 0);
  }
  
  //Pisca o led 20 vezes
  for(int i = 0; i < 20; i++){
    digitalWrite(led, 1);
    delay(250);
    digitalWrite(led, 0);
    }
}
