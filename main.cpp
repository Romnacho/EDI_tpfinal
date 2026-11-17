#include "parametros.h"

void setup() {

  pinMode(PIN_ROJO, OUTPUT); // LED indicador de estado
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_AZUL, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT); // buzzer
  pinMode(PIN_BOTON, INPUT_PULLUP); // boton

  pinMode(PIN_LED_BOLSA, OUTPUT);       // el LED para el suero 
  digitalWrite(PIN_LED_BOLSA, HIGH);    // prendido todo el tiempo

  Wire.begin();
  Serial.begin(115200);

  Wire.beginTransmission(MPU); // MPU
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  
  Serial.println("Sistema iniciado");
}

void loop() {
  leerSensorInclinacion(); // veo si el sistema esta inclinado
  leerSensorBolsa();       // veo si la bolsa esta vacia
  leerSerialBolsa();       // FUNCION DE DEBUG; PONE EL ESTADO QUE LE INDIQUES POR SERIAL
  
  bool boton = botonApretado();

  if (!inclinado) inclinadoIgnorado = false; // reseteo inclinado ignorado una vez que ya no esta inclinado
  bool hayInclinacion = inclinado && !inclinadoIgnorado; // cuando el sistema esta inclinado y no fue ignorado, hay inclinacion
  switch (estado) {
    case IDLE:
      if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA);  // si esta inclinado voy a pre alarma            
      } 
      else if (vacia) {
        if (!vacioReconocido) { 
          cambiarEstado(ALARMA_VACIO);  // si se detecta la bolsa vacia y no fue reconocido voy a alarma vacio        
        } 
        else if (millis() - tReconocido >= T_RECARGA) { //si se toca el boton pero la bolsa sigue vacia despues de cierto tiempo, vuelve a alarma vacio
          vacioReconocido = false;             
          cambiarEstado(ALARMA_VACIO);         
        }
      } 
      else {
        vacioReconocido = false; // para que no quede en TRUE si se toca el boton y se cambia a tiempo             
      }
      break;

    case PRE_ALARMA:
      if (boton) {
        inclinadoIgnorado = true;       // si se toca el boton se cancela la alarma       
        cambiarEstado(IDLE);
      } 
      else if (!inclinado) {            // si se endereza el sistema vuelve a IDLE solo
        cambiarEstado(IDLE);                   
      } 
      else if (millis() - tEstado >= T_CAE) { // despues de cierto tiempo sin volver a IDLE, el sistema pasa a alarma caido
        cambiarEstado(ALARMA_CAIDO);           
      }
      break;

    case ALARMA_CAIDO:
      if (boton) {
        inclinadoIgnorado = true;   // solo vuelve a IDLE si se apreta el boton            
        cambiarEstado(IDLE);
      }
      break;

    case ALARMA_VACIO:
      if (boton) {
        vacioReconocido = true;                 
        tReconocido = millis();     // si se apreta el boton se reconoce el vacio y se da tiempo a cambiar la bolsa            
        cambiarEstado(IDLE);
      } 
      else if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA); // si se detecta inclinacion en este estado, pasa a pre alarma
      }
      break;
  }

  actualizarSalidas(); // se actualizan el LED indicador y el buzzer segun el estado                          
}