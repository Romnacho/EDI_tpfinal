#ifndef HARDWARE.H
#define HARDWARE.H

#include "config.h"

// declaraciones de funciones 
bool botonApretado();                                   // antirrebote del boton
void leerSensorInclinacion();                           // lee el MPU
void leerSensorBolsa();                                 // lee el receptor de la bolsa
void setColor(bool r, bool g, bool b);                  // color del LED RGB
int buzz_intermitente(int tono1, int tono2, int intervalo);

#endif 