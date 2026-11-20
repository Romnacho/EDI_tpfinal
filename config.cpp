#include "config.h"     

Estado estado = IDLE;

unsigned long tEstado = 0;          // indica tiempo en el estado actual
bool inclinado = false;             // indica si el sistema esta inclinado o no
bool vacia = false;                 // indica si la bolsa esta vacia o no
bool inclinadoIgnorado = false;     // incica si se apreto el boton durante una inclinacion
bool vacioReconocido = false;       // indica si se apreto el boton durante una bolsa vacia
unsigned long tReconocido = 0;      // indica tiempo en que se reconocio la bolsa vacia
bool botonLectura = HIGH;           // indica ultima lectura del pin del boton
bool botonEstable = HIGH;           // indica estado del boton confirmado tras 30 ms sin cambiar
unsigned long tCambioBoton = 0;     // indica tiempo del ultimo cambio de la lectura del boton
int tonoActual = 0;                 // indica tono actual del buzzer
unsigned long tUltimaMedicion = 0;  // cuando fue la ultima lectura