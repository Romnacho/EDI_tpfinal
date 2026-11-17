#include "parametros.h"


void actualizarSalidas() {
  int tono = 0; // por default en silencio

  switch (estado) {
    case IDLE:
      if (vacia && vacioReconocido) 
        setColor(false, true, true);  // cian
      else 
        setColor(false, true, false); // verde
      break;
    case PRE_ALARMA:
      setColor(true, true, false);    // amarillo 
      tono = buzz_intermitente(2000, 0, 400);
      break;
    case ALARMA_CAIDO:
      setColor(true, false, false); // rojo
      tono = buzz_intermitente(3500, 2500, 300);
      break;
    case ALARMA_VACIO:
      setColor(true, false, true); // magenta 
      tono = buzz_intermitente(3000, 1000, 1000);
      break;
  }

  if (tono != tonoActual) { //reseteo el tono
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