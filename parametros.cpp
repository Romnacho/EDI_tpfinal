#include <Wire.h>
#include <IRremote.hpp>

// pines
const int PIN_EMISOR_IR = 7;
const int PIN_RECEPTOR_IR = 3; 
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
unsigned long tUltimaRecepcionIR = 0;
unsigned long tUltimaEmisionIR = 0;
unsigned long T_TIMEOUT_IR = 400;
bool senalIR = false;


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