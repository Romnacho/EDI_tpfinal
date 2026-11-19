#ifndef PARAMETROS_H        // evita que el archivo se incluya dos veces
#define PARAMETROS_H

#include <Wire.h>
#include <Arduino.h>

// pines 
const int PIN_LED_BOLSA = 7;
const int PIN_RECEPTOR_BOLSA = A0;
const int PIN_ROJO = 8;
const int PIN_VERDE = 9;
const int PIN_AZUL = 10;
const int PIN_BOTON = 2;
const int PIN_BUZZER = 4;
const int MPU = 0x68;

// declaraciones de parametros 
const int UMBRAL_LUZ = 780;                  // si el valor del fotoresistor supera esto, llega poca luz: bolsa llena            
const bool VACIA_CON_MUCHA_LUZ = true;       // true; bolsa vacia, false; bolsa llena
const unsigned long T_MEDICION = 50;         // ms entre lecturas del fotoresistor
const unsigned long T_CAE = 5000;            // ms inclinado antes de considerar que cayo
const unsigned long T_RECARGA = 10000;       // ms para recargar la bolsa antes de alarma
const float UMBRAL_INCLINACION = 30.0;       // grados de inclinacion para considerarlo inclinado
const unsigned long T_GRACIA = 15000;       // ms de silencio antes de reactivar la alarma si sigue inclinado

enum Estado { IDLE, PRE_ALARMA, ALARMA_CAIDO, ALARMA_VACIO };

// declaraciones de variables, despues en "variables.cpp" se les da valor inicial y se explica que significan 
extern Estado estado;
extern const char* NOMBRES[];
extern unsigned long tEstado;
extern bool inclinado;
extern bool vacia;
extern bool inclinadoIgnorado;
extern bool vacioReconocido;
extern unsigned long tReconocido;
extern bool botonLectura;
extern bool botonEstable;
extern unsigned long tCambioBoton;
extern int tonoActual;
extern bool modoSensor;
extern unsigned long tUltimaMedicion;

// declaraciones de funciones 
bool botonApretado();                                   // antirrebote del boton
void leerSensorInclinacion();                           // lee el MPU
void leerSensorBolsa();                                 // lee el receptor de la bolsa
void setColor(bool r, bool g, bool b);                  // color del LED RGB
int buzz_intermitente(int tono1, int tono2, int intervalo);
void actualizarSalidas();                               // LED y buzzer segun el estado
void cambiarEstado(Estado nuevo);                       // cambia de estado de la FSM
void despertarMPU();          

#endif