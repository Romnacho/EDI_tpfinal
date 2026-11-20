#ifndef SOFTWARE.H
#define SOFTWARE.H

#include "config.h"
#include "hardware.h"

// declaraciones de funciones 
void mde(bool boton, bool hayInclinacion);              // maquina de estados y salidas
void cambiarEstado(Estado nuevo);                       // cambia de estado de la FSM


#endif 