#ifndef PARAMETROS_H        // evita que el archivo se incluya dos veces
#define PARAMETROS_H

#include <Wire.h>
#include <Arduino.h>

// pines 
#define PIN_LED_BOLSA 7
#define PIN_RECEPTOR_BOLSA A0
#define PIN_ROJO 8
#define PIN_VERDE 9
#define PIN_AZUL 10
#define PIN_BOTON 2
#define PIN_BUZZER 4
#define MPU 0x68

// parametros 
#define UMBRAL_LUZ 780                  // si el valor del fotoresistor supera esto, llega poca luz: bolsa llena            
#define T_MEDICION 50                   // ms entre lecturas del fotoresistor
#define T_CAE 5000                      // ms inclinado antes de considerar que cayo
#define T_RECARGA 10000                 // ms para recargar la bolsa antes de alarma
#define UMBRAL_INCLINACION 30           // grados de inclinacion para considerarlo inclinado
#define T_GRACIA 15000                  // ms de silencio antes de reactivar la alarma si sigue inclinado

enum Estado { IDLE, PRE_ALARMA, ALARMA_CAIDO, ALARMA_VACIO };

// declaraciones de variables, despues en "variables.cpp" se les da valor inicial y se explica que significan 
extern Estado estado;
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
extern unsigned long tUltimaMedicion;

// declaraciones de funciones 
bool botonApretado();                                   // antirrebote del boton
void leerSensorInclinacion();                           // lee el MPU
void leerSensorBolsa();                                 // lee el receptor de la bolsa
void setColor(bool r, bool g, bool b);                  // color del LED RGB
int buzz_intermitente(int tono1, int tono2, int intervalo);
void mde(bool boton, bool hayInclinacion);              // maquina de estados + salidas
void cambiarEstado(Estado nuevo);                       // cambia de estado de la FSM

#endif