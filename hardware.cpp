bool botonApretado() { // antirrebote, solo acepta un cambio si la lectura se mantuvo estable más de 30 ms, y devuelve true únicamente en el flanco de bajada
  bool lectura = digitalRead(PIN_BOTON);
  if (lectura != botonLectura) {                
    botonLectura = lectura;
    tCambioBoton = millis();                   
  }
  if (millis() - tCambioBoton > 30 && lectura != botonEstable) {
    botonEstable = lectura;                     
    if (botonEstable == LOW) return true;       
  }
  return false;
}

void setColor(bool r, bool g, bool b) {
  digitalWrite(PIN_ROJO, r ? HIGH : LOW);
  digitalWrite(PIN_VERDE, g ? HIGH : LOW);
  digitalWrite(PIN_AZUL, b ? HIGH : LOW);

}

void leerSensorInclinacion() { // toma valores del MPU y actualiza la variable inclinado segun corresponda
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);                                       
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // pide 6 bytes (2 por eje) para escribir

  if (Wire.available() >= 6) {
    int16_t ax = (Wire.read() << 8 | Wire.read()); 
    int16_t ay = (Wire.read() << 8 | Wire.read());
    int16_t az = (Wire.read() << 8 | Wire.read());

    float gX = (float)ax / 16384.0; // se dividen por 16384 para pasarlos a gravedad
    float gY = (float)ay / 16384.0;
    float gZ = (float)az / 16384.0;

    float anguloX = atan2(gX, sqrt(gY * gY + gZ * gZ)) * 180.0 / PI; // calculo el angulo
    float anguloY = atan2(gY, sqrt(gX * gX + gZ * gZ)) * 180.0 / PI;

    if (abs(anguloX) > UMBRAL_INCLINACION || abs(anguloY) > UMBRAL_INCLINACION) { // compara con el umbral y actualiza inclinado
      inclinado = true;
    } else {
      inclinado = false;
    }
  }
}

void leerSensorBolsa() {
  static unsigned long tUltimoDebug = 0;
  if (!modoSensor) return;

  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol == NEC &&
        IrReceiver.decodedIRData.address == 0x00 &&
        IrReceiver.decodedIRData.command == 0x45) {
      tUltimaRecepcionIR = millis();
    }
    IrReceiver.resume();
  }

  if (millis() - tUltimaEmisionIR >= T_MEDICION) {
    tUltimaEmisionIR = millis();
    IrSender.sendNEC(0x00, 0x45, 0);
  }

  senalIR = (tUltimaRecepcionIR != 0 && millis() - tUltimaRecepcionIR < T_TIMEOUT_IR);
  bool candidata = !senalIR;

  if (candidata == vacia) {
    bolsaCambiando = false;
  } else if (!bolsaCambiando) {
    bolsaCambiando = true;
    tCandidata = millis();
  } else if (millis() - tCandidata >= T_FILTRO_BOLSA) {
    vacia = candidata;
    bolsaCambiando = false;
    Serial.println(vacia ? "sensor bolsa: VACIA" : "sensor bolsa: LLENA");
  }

  if (DEBUG_SENSOR && millis() - tUltimoDebug >= 500) {
    tUltimoDebug = millis();
    Serial.print("IR: ");
    Serial.print(senalIR ? "RECIBIENDO" : "SIN SENAL");
    Serial.print(" | Bolsa: ");
    Serial.println(vacia ? "VACIA" : "LLENA");
  }
}

void leerSerialBolsa() {
  while (Serial.available() > 0) {
    char t = Serial.read();
    if (t == 'v') { modoSensor = false; vacia = true;  Serial.println("manual: VACIA"); }
    if (t == 'l') { modoSensor = false; vacia = false; Serial.println("manual: LLENA"); }
    if (t == 's') { modoSensor = true; bolsaCambiando = false; Serial.println("modo SENSOR activado"); }
  }
}

int buzz_intermitente(int tono1, int tono2, int intervalo) { // alterna entre dos tonos de buzzer
  static unsigned long tBuzzer = 0;
  static bool beepOn = false;
  if (millis() - tBuzzer >= intervalo) { 
    tBuzzer = millis();
    beepOn = !beepOn;
  }
  return beepOn ? tono1 : tono2; 
}

