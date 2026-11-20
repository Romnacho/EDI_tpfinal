#include "main.h"

void setup() {
  pinMode(PIN_ROJO, OUTPUT); // LED indicador de estado
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_AZUL, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT); // buzzer
  pinMode(PIN_BOTON, INPUT_PULLUP); // boton

  pinMode(PIN_LED_BOLSA, OUTPUT);       // el LED para el suero 
  digitalWrite(PIN_LED_BOLSA, HIGH);    // prendido todo el tiempo

  Wire.begin();  // MPU
  Wire.beginTransmission(MPU); 
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void loop() {
  leerSensorInclinacion(); // veo si el sistema esta inclinado
  leerSensorBolsa();       // veo si la bolsa esta vacia
  
  bool boton = botonApretado();

  if (!inclinado) 
    inclinadoIgnorado = false; // reseteo inclinado ignorado una vez que ya no esta inclinado
  bool hayInclinacion = inclinado && !inclinadoIgnorado; // cuando el sistema esta inclinado y no fue ignorado, hay inclinacion

  mde(boton, hayInclinacion);
}