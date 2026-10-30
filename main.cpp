// pines 
const int PIN_ROJO = 8;
const int PIN_VERDE = 9;
const int PIN_BOTON = 2;
const int PIN_BUZZER = 4;

// estados  
enum Estado { IDLE, PRE_ALARMA, ALARMA_CAIDO, ALARMA_VACIO };
Estado estado = IDLE;   // arranca en IDLE

// nombres para imprimir por Serial 
const char* NOMBRES[] = {"IDLE", "PRE_ALARMA", "ALARMA_CAIDO", "ALARMA_VACIO"};

// cambia de estado, avisa y actualiza las salidas
void cambiarEstado(Estado nuevo) {
  Serial.print(NOMBRES[estado]);
  Serial.print(" -> ");
  Serial.println(NOMBRES[nuevo]);
  estado = nuevo;
  aplicarSalidas();
}


// Devuelve true UNA sola vez por cada pulsación
bool botonApretado() {
  bool lectura = digitalRead(PIN_BOTON);       // lee el pin
  if (lectura != botonLectura) {               // si cambió respecto a la vuelta anterior...
    botonLectura = lectura;                    // ...la guarda
    tCambioBoton = millis();                   // ...y anota cuándo cambió
  }
  // si lleva más de 30 ms sin cambiar y es distinta de la confirmada
  if (millis() - tCambioBoton > 30 && lectura != botonEstable) {
    botonEstable = lectura;                    // la confirma
    if (botonEstable == LOW) return true;      // LOW = recién apretado
  }
  return false;
}

// Pone LED y buzzer según el estado en el que se entra
void aplicarSalidas() {
  noTone(PIN_BUZZER);                          // por defecto, silencio
  switch (estado) {
    case IDLE:
      digitalWrite(PIN_ROJO, LOW);
      digitalWrite(PIN_VERDE, HIGH);           // verde fijo
      break;
    case PRE_ALARMA:
      digitalWrite(PIN_VERDE, LOW);
      digitalWrite(PIN_ROJO, HIGH);            // rojo (el parpadeo lo agregamos después)
      break;
    case ALARMA_CAIDO:
      digitalWrite(PIN_VERDE, LOW);
      digitalWrite(PIN_ROJO, HIGH);            // rojo fijo
      tone(PIN_BUZZER, 1000);                  // sonido continuo agudo
      break;
    case ALARMA_VACIO:
      digitalWrite(PIN_VERDE, LOW);
      digitalWrite(PIN_ROJO, HIGH);
      tone(PIN_BUZZER, 500);                   // tono grave, distinto al de caída
      break;
  }
}

// Cambia de estado, avisa por Serial y actualiza las salidas
void cambiarEstado(Estado nuevo) {
  Serial.print(NOMBRES[estado]);
  Serial.print(" -> ");
  Serial.println(NOMBRES[nuevo]);
  estado = nuevo;
  aplicarSalidas();
}


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}