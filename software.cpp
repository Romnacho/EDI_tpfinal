#include "software.h"

void mde(bool boton, bool hayInclinacion) {
  int tono = 0; // por default en silencio

  switch (estado) {
    case IDLE:
      if (inclinadoIgnorado && (millis() - tReconocido >= T_GRACIA)) {
        inclinadoIgnorado = false; // si pasan T_GRACIA segundos y sigue inclinado, vuelve a habilitarse
      }

      if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA);  // si esta inclinado voy a pre alarma            
      } 
      else if (vacia) {
        if (!vacioReconocido) { 
          cambiarEstado(ALARMA_VACIO);  // si se detecta la bolsa vacia y no fue reconocido voy a alarma vacio        
        } 
        else if (millis() - tReconocido >= T_RECARGA) { // si se toca el boton pero la bolsa sigue vacia despues de cierto tiempo, vuelve a alarma vacio
          vacioReconocido = false;             
          cambiarEstado(ALARMA_VACIO);         
        }
      } 
      else {
        vacioReconocido = false; // para que no quede en TRUE si se toca el boton y se cambia a tiempo             
      }

      if (vacia && vacioReconocido) 
        setColor(false, true, true);  // cian
      else 
        setColor(false, true, false); // verde
      tono = 0;
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

      setColor(true, true, false);    // amarillo 
      tono = buzz_intermitente(2000, 0, 400);
      break;

    case ALARMA_CAIDO:
      if (boton) {
        inclinadoIgnorado = true;   // solo vuelve a IDLE si se apreta el boton            
        cambiarEstado(IDLE);
      }

      setColor(true, false, false); // rojo
      tono = buzz_intermitente(3500, 2500, 300);
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

      setColor(true, false, true); // magenta 
      tono = buzz_intermitente(3000, 1000, 1000);
      break;
  }

  if (tono != tonoActual) { // reseteo el tono solo cuando cambia
    if (tono == 0) 
      noTone(PIN_BUZZER);
    else 
      tone(PIN_BUZZER, tono);
    tonoActual = tono;
  }
}

void cambiarEstado(Estado nuevo) {
  estado = nuevo;        // actualiza el estado
  tEstado = millis();    // reinicia el contador de tiempo en el estado
}