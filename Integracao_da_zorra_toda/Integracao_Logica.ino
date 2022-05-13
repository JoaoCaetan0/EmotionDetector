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
  int soma = 0;
  while (controleBpm < 100){
    bpmVetor[controleBpm] = BPM;
    controleBpm++;
    }   
   for(int i = 0; i<100; i++){
     soma = soma + bpmVetor[i];
   }
  BpmMedio = soma/100;         //Média primária com 100 elementos       
  BpmMedio = (bpmIdade(idade) + BpmMedio)/2;   
  return BpmMedio;
}

void piscaLed(int led){
  
  //desliga os leds
  for(int i = 10; i <= 13; i++){
    digitalWrite(i, 0);
  }
  
  //Pisca o led 20 vezes
  for(int i = 0; i < 20; i++){
    digitalWrite(led, 1);
    delay(250);
    digitalWrite(led, 0);
    }
}

void ligaLed(int led){
  
  //desliga os leds
  for(int i = 10; i <= 13; i++){
    if(i == led){
      digitalWrite(led, 1);
    }
    else{
      digitalWrite(i, 0);
    }
  }
    delay(3000);
}
