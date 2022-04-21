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


void Configurando(unsigned long int tempoAnterior){
  if((millis() - tempoAnterior) < 1000){
    Serial.println("Configurando...");
    tempoAnterior = millis();
    digitalWrite(pinoLedY, 1);
    digitalWrite(pinoLedG, 0);
  }
}

int mediaBpm(int bpm, int idade){
  int controleBpm = 0;
  volatile int bpmVetor[100];
  int BpmMedio = 0;
  int conf = 0;
  while (conf != 1){
  if ((bpm > 40 && bpm < 150) && (controleBpm > 100)){                      //Limpar ruídos da medição
    bpmVetor[controleBpm] = bpm;
    controleBpm++;
      }                                         
       if (bpmVetor[100] != 0){
         int soma = 0;
         for(int i = 0; i<100; i++){
           soma = soma + bpmVetor[i];
         }
         BpmMedio = soma/100;         //Média primária com 100 elementos
         conf = 1;
       }
  }        
       BpmMedio = (bpmIdade(idade) + BpmMedio)/2;   
       return BpmMedio;
}
