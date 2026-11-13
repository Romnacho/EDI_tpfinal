void setup() {

  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_AZUL, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);

  pinMode(PIN_EMISOR_IR, OUTPUT);
  digitalWrite(PIN_EMISOR_IR, LOW);

  Wire.begin();
  Serial.begin(115200);

  // Inicializar emisor y receptor infrarrojo
  IrSender.begin(PIN_EMISOR_IR, DISABLE_LED_FEEDBACK);
  IrReceiver.begin(PIN_RECEPTOR_IR, DISABLE_LED_FEEDBACK);

  // Inicializar MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  
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