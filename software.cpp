void actualizarSalidas() {
  int tono = 0; // por default en silencio

  switch (estado) {
    case IDLE:
      if (vacia && vacioReconocido) setColor(false, true, true); 
      else setColor(false, true, false); // verde
      break;
    case PRE_ALARMA:
      setColor(true, true, false); // amarillo 
      tono = buzz_intermitente(500, 0, 400);
      break;
    case ALARMA_CAIDO:
      setColor(true, false, false); // rojo
      tono = buzz_intermitente(700, 1000, 300);
      break;
    case ALARMA_VACIO:
      setColor(true, false, true); // magenta 
      tono = buzz_intermitente(300, 100, 1000);
      break;
  }

  if (tono != tonoActual) {
    if (tono == 0) noTone(PIN_BUZZER);
    else tone(PIN_BUZZER, tono);
    tonoActual = tono;
  }
}


