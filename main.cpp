#include <Wire.h>

// pines
const int PIN_ROJO = 8;
const int PIN_VERDE = 9;
const int PIN_BOTON = 2;
const int PIN_BUZZER = 4;
const int MPU_addr = 0x68; 

const unsigned long T_CAE = 5000;       // ms inclinado antes de considerar que cayó
const unsigned long T_RECARGA = 10000;  // ms de gracia para recargar la bolsa

const float UMBRAL_INCLINACION = 30.0;  // grados de inclinación para considerarlo inclinado

enum Estado { IDLE, PRE_ALARMA, ALARMA_CAIDO, ALARMA_VACIO };
Estado estado = IDLE;
const char* NOMBRES[] = {"IDLE", "PRE_ALARMA", "ALARMA_CAIDO", "ALARMA_VACIO"};

unsigned long tEstado = 0;        

bool inclinado = false;           
bool vacia = false;               

bool inclinadoIgnorado = false;   
bool vacioReconocido = false;    
unsigned long tReconocido = 0;    

bool botonLectura = HIGH;
bool botonEstable = HIGH;
unsigned long tCambioBoton = 0;

int tonoActual = 0;               

bool botonApretado() {
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

void leerSensorInclinacion() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true);

  if (Wire.available() >= 6) {
    int16_t ax = (Wire.read() << 8 | Wire.read());
    int16_t ay = (Wire.read() << 8 | Wire.read());
    int16_t az = (Wire.read() << 8 | Wire.read());

    float gX = (float)ax / 16384.0;
    float gY = (float)ay / 16384.0;
    float gZ = (float)az / 16384.0;

    float anguloX = atan2(gX, sqrt(gY * gY + gZ * gZ)) * 180.0 / PI;
    float anguloY = atan2(gY, sqrt(gX * gX + gZ * gZ)) * 180.0 / PI;

    if (abs(anguloX) > UMBRAL_INCLINACION || abs(anguloY) > UMBRAL_INCLINACION) {
      inclinado = true;
    } else {
      inclinado = false;
    }
  }
}

void leerSerialBolsa() {
  while (Serial.available() > 0) {
    char t = Serial.read();
    if (t == 'v') { vacia = true;      Serial.println("sensor bolsa: VACIA"); }
    if (t == 'l') { vacia = false;     Serial.println("sensor bolsa: LLENA"); }
  }
}

int buzz_intermitente(int tono1, int tono2, int intervalo) {
  static unsigned long tBuzzer = 0;
  static bool beepOn = false;
  if (millis() - tBuzzer >= intervalo) { 
    tBuzzer = millis();
    beepOn = !beepOn;
  }
  return beepOn ? tono1 : tono2; 
}

void actualizarSalidas() {
  bool rojo = false;                            
  int tono = 0;                                

  switch (estado) {
    case IDLE:
      rojo = (vacia && vacioReconocido);        
      break;
    case PRE_ALARMA:
      rojo = true; 
      tono = buzz_intermitente(500, 0, 400); 
      break;
    case ALARMA_CAIDO:
      rojo = true;
      tono = buzz_intermitente(700, 1000, 300);
      break;
    case ALARMA_VACIO:
      rojo = true;
      tono = buzz_intermitente(300, 100, 1000); 
      break;
  }

  digitalWrite(PIN_ROJO, rojo ? HIGH : LOW);
  digitalWrite(PIN_VERDE, rojo ? LOW : HIGH);

  if (tono != tonoActual) {                     
    if (tono == 0) noTone(PIN_BUZZER);
    else tone(PIN_BUZZER, tono);
    tonoActual = tono;
  }
}

void cambiarEstado(Estado nuevo) {
  Serial.print(NOMBRES[estado]);
  Serial.print(" -> ");
  Serial.println(NOMBRES[nuevo]);
  estado = nuevo;
  tEstado = millis();                           // reinicia el cronómetro del estado
}

void setup() {
  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  
  Wire.begin();
  Serial.begin(115200); // Una sola velocidad limpia y rápida
  
  // Despertar el MPU-6050
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  Serial.println("Sistema iniciado. v=bolsa vacia, l=bolsa llena. Incline el MPU-6050.");
}

void loop() {
  // Leemos el sensor físico de inclinación y el comando serial de la bolsa
  leerSensorInclinacion();
  leerSerialBolsa();
  
  bool boton = botonApretado();

  // Si ya se enderezó, dejamos de ignorar la inclinación
  if (!inclinado) inclinadoIgnorado = false;
  bool hayInclinacion = inclinado && !inclinadoIgnorado;

  switch (estado) {
    case IDLE:
      if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA);              // la caída tiene prioridad
      } else if (vacia) {
        if (!vacioReconocido) {
          cambiarEstado(ALARMA_VACIO);          // primera detección
        } else if (millis() - tReconocido >= T_RECARGA) {
          vacioReconocido = false;              // se acabó el tiempo de gracia
          cambiarEstado(ALARMA_VACIO);          // y sigue vacía: suena de nuevo
        }
      } else {
        vacioReconocido = false;                // se recargó: todo normal
      }
      break;

    case PRE_ALARMA:
      if (boton) {
        inclinadoIgnorado = true;               // cancelado a mano
        cambiarEstado(IDLE);
      } else if (!inclinado) {
        cambiarEstado(IDLE);                    // acomodado
      } else if (millis() - tEstado >= T_CAE) {
        cambiarEstado(ALARMA_CAIDO);            // cae
      }
      break;

    case ALARMA_CAIDO:
      if (boton) {
        inclinadoIgnorado = true;               // no re-alarmar hasta que se endereze
        cambiarEstado(IDLE);
      }
      break;

    case ALARMA_VACIO:
      if (boton) {
        vacioReconocido = true;                 // anota que ya se enteró
        tReconocido = millis();                 // y cuándo
        cambiarEstado(IDLE);
      } else if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA);
      }
      break;
  }

  actualizarSalidas();                          // LED y buzzer según el estado
  delay(50); // Pequeña pausa para estabilizar lecturas del loop
}