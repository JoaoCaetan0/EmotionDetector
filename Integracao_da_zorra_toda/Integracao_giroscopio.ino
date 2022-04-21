



char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

int detectaMovimento(int movimento) { // Função que contabiliza quantos movimentos ocorreram
  unsigned long int tempoAnterior = 0;
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

  while ((millis() - tempoAnterior) < 250){
  if (accelerometer_x < 1000 && accelerometer_y < -8000) {        // pra cima
    digitalWrite(LED_LB, 0);
    digitalWrite(LED_RB, 0);
    digitalWrite(LED_RT, 1);
    digitalWrite(LED_LT, 1);
    movimento = movimento + 1;
    tempoAnterior = millis();
  } else if (accelerometer_x < 1000 && accelerometer_y > 8000) {    // pra baixo
    digitalWrite(LED_LB, 1);
    digitalWrite(LED_RB, 1);
    digitalWrite(LED_RT, 0);
    digitalWrite(LED_LT, 0);
    movimento = movimento + 1;
    tempoAnterior = millis();
  } else if (accelerometer_x > 8000 && accelerometer_y < 1000) {     // pra direita
    digitalWrite(LED_LB, LOW);
    digitalWrite(LED_RB, HIGH);
    digitalWrite(LED_RT, HIGH);
    digitalWrite(LED_LT, LOW);
    movimento = movimento + 1;
    tempoAnterior = millis();
  } else if (accelerometer_x < -8000 && accelerometer_y < 1000) {     // pra esquerda
    digitalWrite(LED_LB, HIGH);
    digitalWrite(LED_RB, LOW);
    digitalWrite(LED_RT, LOW);
    digitalWrite(LED_LT, HIGH);
    movimento = movimento + 1;
    tempoAnterior = millis();
  } else {
    digitalWrite(LED_LB, LOW);
    digitalWrite(LED_RB, LOW);
    digitalWrite(LED_RT, LOW);
    digitalWrite(LED_LT, LOW);
    // Não se mexeu
    tempoAnterior = millis();
  }
  }
return movimento;
}
