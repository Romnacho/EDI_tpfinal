#include <Wire.h>

// pines
const int PIN_EMISOR_IR = 7;    
const int PIN_RECEPTOR_IR = A0; 
const int PIN_ROJO = 8;
const int PIN_VERDE = 9;
const int PIN_AZUL = 10;
const int PIN_BOTON = 2;
const int PIN_BUZZER = 4;
const int MPU = 0x68; 

const int UMBRAL_ALTO = 150;                  // si el cambio supera esto, se detecta luz (bolsa vacia)
const int UMBRAL_BAJO = 100;                  // si el cambio baja de esto, no hay luz (hay liquido)
const unsigned long T_FILTRO_BOLSA = 1500;    // ms que debe sostenerse un cambio antes de aceptarlo
const unsigned long T_MEDICION = 100;         // cada cuantos ms se mide el sensor IR
const bool DEBUG_SENSOR = true;               // true = imprime las lecturas por Serial
const unsigned long T_CAE = 5000;             // ms inclinado antes de considerar que cayo
const unsigned long T_RECARGA = 10000;        // ms para recargar la bolsa antes de alarma
const float UMBRAL_INCLINACION = 30.0;        // grados de inclinación para considerarlo inclinado

enum Estado { IDLE, PRE_ALARMA, ALARMA_CAIDO, ALARMA_VACIO };
Estado estado = IDLE;
const char* NOMBRES[] = {"IDLE", "PRE_ALARMA", "ALARMA_CAIDO", "ALARMA_VACIO"};

unsigned long tEstado = 0;       // indica tiempo en el estado actual    
bool inclinado = false;          // indica si el sistema esta inclinado o no 
bool vacia = false;              // indica si la bolsa esta vacia o no 
bool inclinadoIgnorado = false;  // indica si se apreto el boton durante una inclinacion  
bool vacioReconocido = false;    // indica si se apreto el boton durante una bolsa vacia
unsigned long tReconocido = 0;   // indica tiempo en el q se reconcocio la bolsa vacia
bool botonLectura = HIGH;        // indica ultima lectura del pin del boton
bool botonEstable = HIGH;        // indica estado del boton ya confirmado desp de 30 ms sin cambiar
unsigned long tCambioBoton = 0;  // indica el tiempo del ultimo cambio de la lectura del boton
int tonoActual = 0;              // indica el tono actual del buzzer
bool modoSensor = true;          // true = 'vacia' la decide el sensor IR; false = modo manual por serial
bool bolsaCambiando = false;     // true mientras una lectura distinta espera a pasar el filtro de tiempo
unsigned long tCandidata = 0;    // tiempo en que empezo a sostenerse la lectura distinta

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
  digitalWrite(PIN_R, r ? HIGH : LOW);   
  digitalWrite(PIN_G, g ? HIGH : LOW);   
  digitalWrite(PIN_B, b ? HIGH : LOW);   
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
  static unsigned long tUltimaMedicion = 0;   // cuando fue la ultima medicion
  static unsigned long tUltimoDebug = 0;      // cuando fue el ultimo print de depuracion

  if (!modoSensor) return;                                  // en modo manual no se usa el sensor
  if (millis() - tUltimaMedicion < T_MEDICION) return;      // todavia no toca medir
  tUltimaMedicion = millis();

  digitalWrite(PIN_EMISOR, HIGH);                // prendo el emisor
  delay(5);                                      // espero a que la lectura se estabilice
  int conLuz = analogRead(PIN_RECEPTOR);         // lectura con emisor prendido
  digitalWrite(PIN_EMISOR, LOW);                 // apago el emisor
  delay(5);
  int sinLuz = analogRead(PIN_RECEPTOR);         // lectura con emisor apagado (luz ambiente)

  int cambio = abs(conLuz - sinLuz);             // cuanto cambia la lectura por culpa del emisor

  // histeresis: dos umbrales distintos para no parpadear en el limite
  bool candidata = vacia;                            // por defecto, el estado no cambia
  if (cambio > UMBRAL_ALTO) candidata = true;        // pasa mucha luz -> bolsa vacia
  else if (cambio < UMBRAL_BAJO) candidata = false;  // casi no pasa luz -> hay liquido

  // filtro de tiempo: el cambio debe sostenerse T_FILTRO_BOLSA ms
  if (candidata == vacia) {
    bolsaCambiando = false;                      // no hay cambio pendiente
  } else if (!bolsaCambiando) {
    bolsaCambiando = true;                       // empieza a sostenerse un cambio
    tCandidata = millis();
  } else if (millis() - tCandidata >= T_FILTRO_BOLSA) {
    vacia = candidata;                           // se sostuvo lo suficiente: lo aceptamos
    bolsaCambiando = false;
    Serial.println(vacia ? "sensor bolsa: VACIA" : "sensor bolsa: LLENA");
  }

  // depuracion: muestra los valores cada 500 ms para ajustar los umbrales
  if (DEBUG_SENSOR && millis() - tUltimoDebug >= 500) {
    tUltimoDebug = millis();
    Serial.print("IR conLuz=");  Serial.print(conLuz);
    Serial.print(" sinLuz=");    Serial.print(sinLuz);
    Serial.print(" cambio=");    Serial.println(cambio);
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

void actualizarSalidas() {
  int tono = 0; // por default en silencio

  switch (estado) {
    case IDLE:
      if (vacia && vacioReconocido) setColor(true, false, true); // magenta 
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

  // esta parte del buzzer queda igual que antes
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
  tEstado = millis();                           
}

void setup() {
  pinMode(PIN_ROJO, OUTPUT);   
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_AZUL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_EMISOR_IR, OUTPUT);
  pinMode(PIN_EMISOR, OUTPUT);       
  digitalWrite(PIN_EMISOR, LOW);     
  
  Wire.begin();
  Serial.begin(115200); 
  
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  Serial.println("Sistema iniciado");
}

void loop() {
  leerSensorInclinacion();
  leerSensorBolsa(); 
  leerSerialBolsa();
  
  bool boton = botonApretado();

  if (!inclinado) inclinadoIgnorado = false;
  bool hayInclinacion = inclinado && !inclinadoIgnorado;

  switch (estado) {
    case IDLE:
      if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA);              
      } else if (vacia) {
        if (!vacioReconocido) {
          cambiarEstado(ALARMA_VACIO);          
        } else if (millis() - tReconocido >= T_RECARGA) {
          vacioReconocido = false;             
          cambiarEstado(ALARMA_VACIO);         
        }
      } else {
        vacioReconocido = false;              
      }
      break;

    case PRE_ALARMA:
      if (boton) {
        inclinadoIgnorado = true;              
        cambiarEstado(IDLE);
      } else if (!inclinado) {
        cambiarEstado(IDLE);                   
      } else if (millis() - tEstado >= T_CAE) {
        cambiarEstado(ALARMA_CAIDO);           
      }
      break;

    case ALARMA_CAIDO:
      if (boton) {
        inclinadoIgnorado = true;               
        cambiarEstado(IDLE);
      }
      break;

    case ALARMA_VACIO:
      if (boton) {
        vacioReconocido = true;                 
        tReconocido = millis();                 
        cambiarEstado(IDLE);
      } else if (hayInclinacion) {
        cambiarEstado(PRE_ALARMA);
      }
      break;
  }

  actualizarSalidas();                           
}