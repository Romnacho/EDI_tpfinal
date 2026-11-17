#include "parametros.h"

bool botonApretado() {                       // antirrebote
  bool lectura = digitalRead(PIN_BOTON); 
  if (lectura != botonLectura) {                
    botonLectura = lectura;
    tCambioBoton = millis();                   
  }
  if (millis() - tCambioBoton > 30 && lectura != botonEstable) {
    botonEstable = lectura;                     
    if (botonEstable == LOW) 
      return true;       
  }
  return false;
}

void setColor(bool r, bool g, bool b) {   // cambia el color del LED
  digitalWrite(PIN_ROJO, r ? HIGH : LOW); // si r es true, lo pone en high, sino, en low
  digitalWrite(PIN_VERDE, g ? HIGH : LOW);
  digitalWrite(PIN_AZUL, b ? HIGH : LOW);
}

void leerSensorInclinacion() { // lee los valores del MPU y actualiza la variable inclinado segun el umbral 
  static unsigned long tUltimaInclinacion = 0;
  if (millis() - tUltimaInclinacion < 50) 
    return;
  tUltimaInclinacion = millis();

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) {
    return; 
  }
  
  if (Wire.requestFrom(MPU, 6, true) != 6) {
    return; 
  }

  int16_t ax = (Wire.read() << 8) | Wire.read(); // lee el primer byte y lo desplaza 8 lugares hacia la izquierda
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();

  float gX = (float)ax / 16384.0; // convierte el numero crudo a unidades de gravedad
  float gY = (float)ay / 16384.0;
  float gZ = (float)az / 16384.0;

  float anguloX = atan2(gX, sqrt(gY * gY + gZ * gZ)) * 180.0 / PI; // calcula el angulo de inclinacion en radianes y lo transforma a grados sexagesimales de -90 a +90 
  float anguloY = atan2(gY, sqrt(gX * gX + gZ * gZ)) * 180.0 / PI;

  if (abs(anguloX) > UMBRAL_INCLINACION || abs(anguloY) > UMBRAL_INCLINACION) { // lee el valor absoluto y actualiza inclinado segun corresponda
    inclinado = true;
  } 
  else {
    inclinado = false;
  }                     
}

void leerSensorBolsa() {
  if (!modoSensor) // DEBUG; en modo manual no se usa el sensor
    return;                     
  if (millis() - tUltimaMedicion < T_MEDICION) // toma mediciones cada T_MEDICION
    return;   
  tUltimaMedicion = millis();

  int lectura = analogRead(PIN_RECEPTOR_BOLSA);    
  if(lectura > UMBRAL_LUZ) // si la bolsa esta llena
    vacia = false;
  else { // si la bolsa esta vacia
    vacia = true; 
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
