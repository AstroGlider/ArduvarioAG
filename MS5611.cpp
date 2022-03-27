/*
 * MS5611.cpp
 * Libreria que lee sensor barométrico MS5611 y devuelve la presión, temperatura
 * y altura.
 * 
 *  sea_level_pressure = ((float)measuredPressure / powf((1 -(altitude/44330.0)), 5.2559));
 *  sea_level_pressure = (float)measuredPressure / pow(1.0 - (altitude/44330.0), 5.2559)
 *  altitude = (float)44330 * (1 - pow(((float) measuredPressure/seaLevelPressure), 0.190295));
 *  altitude = ((pow((sea_press / press), 1/5.2559) - 1.0) * (temp + 273.15)) / 0.0065;
 */

#include "Arduino.h"
#include "MS5611.h"      // carga fichero de cabecera de esta libreria.

// -------------CONSTRUCTORES ---------------------------

MS5611::MS5611(byte _address)                     // Constructor1, solo se indica la @ del sensor
{
  address = _address;                              // @ del sensor
  //measuredPressure = 0;                             // Presión leida em milibares con decimales (ejemplo 1013.25 mb)
  baro_temp_float = 0;                            // Temperatura leída en grados centígrados (ejemplo 25.58 ºC) 
  seaLevelPressure = 101325;                        // Presión a nivel del mar en Pascales
      
}

MS5611::~MS5611()   // Destructor 
{
  
}

// --------------- MÉTODOS -------------------------

void MS5611::begin ()                          // Inicializa el sensor MS5611 para conseguir lecturas lo más precisas posibles
{                                                 // lo cual hace que sea un poco más lento, sobre 12ms cada lectura.
  Wire.beginTransmission(address);
  Wire.write(0x1E);                               // envíamos comando 0x1E - Reset --> Solicitamos resetear el sensor
  Wire.endTransmission(false);                    // no liberamos el bus I2C
  delay(100);                                     // hacemos una pausa de 100ms

  // extract the raw values from PROM
  for (byte i = 1; i <=6; i++)                    //leemos los 6 registros de calibración (en fábrica) de la PROM. No se empieza desde el 0, se empieza desde el 1 hasta el 6 (C1 a C6)
  {
    Wire.beginTransmission(address);
    Wire.write(0xA0+(i*2));                       // los registros no se encuentran consecutivos, el LSB de PROM siempre es 0, por tanto las direcciones donde empieza cada registro es par: 0xA2,0xA4,0xA8,0xAA,0xAC,0XAE 
    Wire.endTransmission(false);                  // no se libera el bus Ic2      
    Wire.requestFrom(address,2,true);             // se piden 2 bytes y luego liberamos el bus ic2
    if (Wire.available()!=2)                      // si no devuelve los 2 bytes...
    {
      Serial.print(F("Error: calibration data not available"));  //utilizando F("") dentro del Serial.print nos ahorramos memoria RAM para más variables.
    }
    calibrationDataMS5611[i] = Wire.read()<<8|Wire.read();       // pasamos los dos Bytes a una variable de 16bits
    /*
    Serial.print(F("calibration data #"));
    Serial.print(i);
    Serial.print(F(" = "));
    Serial.println( calibrationDataMS5611[i] );   
    */
  } 
}

// --------------- GETTERS ------------------------

long MS5611::getPressure()      // Calcula la presión atmosferica leída en MS5611
{                               // devuelve - presión atmosférica en pascales (ejemplo - 102650)
  long result = 0;      // variable temporal para conseguir leer las presión y temperatura en bruto (raw) del ADC 
  long D1 = 0;          // Valor digital de la presión (raw)
  long D2 = 0;          // Valor digital de la temperatura (raw)
  long dT = 0;          // diferencia entre temperatura actual y de referencia
  long P = 0;           // presión compensada con la temperatura (10…1200mbar con 0.01mbar de resolucion)
  float TEMP = 0;       // Temperatura actual (-40…85°C con 0.01°C de resolucion)
  int64_t OFF = 0;      // offset a la temperatura actual
  int64_t SENS = 0;     // Sensibilidad a la temperatura actual
     
                                                  // leemos sensor de presión (raw)
  Wire.beginTransmission(address);
  Wire.write(0x48);                               // envíamos comando 0x48 - Convert D1 (OSR=4096) --> Solicitamos conversión datos del sensor de presión con máxima resolución
  Wire.endTransmission(false);                    // no liberamos el bus I2C
  //cronomarcador = millis();                     // anotamos timestamp de la lectura
  //delayMicroseconds(9040);                      // hacemos una pausa de 9.04ms para que de tiempo de realizar la conversión (con OSR=4096 se necesitan 9.04ms)
  pausa (9040);                                   // uso esta función porque delay() hace que tone() no funcione bien al 100%
  
  Wire.beginTransmission(address);
  Wire.write(0x00);                               // Enviamos comando 0x00 - ADC Read --> Solicitamos lectura del ADC donde se guarda la conversión en un registro de 24bits
  Wire.endTransmission(false);                    // No liberamos el bus I2C
  Wire.requestFrom(address,3,true);               // A partir del 0x00, se piden 3 bytes porque el ADC es de 24bits y luego liberamos el bus ic2
  if (Wire.available()!=3)                        // si no devuelve los 3 bytes...
  {
    Serial.print(F("Error: calibration data not available"));
  }
  for (byte i = 0; i <=2; i++)                    //leemos los 3 bytes del ADC
  {
    result = (result<<8) | Wire.read();           // vamos desplazando los bytes hacia la izquierda a medida que añadimos un nuevo byte
  } 
  D1 = result;                                    // guardamos el valor digital de la presión (raw)
  
  result = 0;                                     // reiniciamos variable
                                                  // leemos sensor de temperatura (raw)
  Wire.beginTransmission(address);
  Wire.write(0x50);                               // envíamos comando 0x50 - Convert D2 (OSR=256) --> Solicitamos conversión datos del sensor de temperatura con mínima precisión
  Wire.endTransmission(false);                    // no liberamos el bus I2C

  //delayMicroseconds(600);                         // hacemos una pausa de 0.6ms para que de tiempo de realizar la conversión (con OSR=256 se necesitan 0.6ms)
  pausa (600);
  
  Wire.beginTransmission(address);
  Wire.write(0x00);                               // Enviamos comando 0x00 - ADC Read --> Solicitamos lectura del ADC donde se guarda la conversión en un registro de 24bits
  Wire.endTransmission(false);                    // No liberamos el bus I2C
  Wire.requestFrom(address,3,true);        // A partir del 0x00, se piden 3 bytes porque el ADC es de 24bits y luego liberamos el bus ic2
  if (Wire.available()!=3)                        // si no devuelve los 3 bytes...
  {
    Serial.print(F("Error: calibration data not available"));
  }
  for (byte i = 0; i <=2; i++)                    //leemos los 3 bytes del ADC
  {
    result = (result<<8) | Wire.read();           // vamos desplazando los bytes hacia la izquierda a medida que añadimos un nuevo byte
  }
  D2 = result;                                    // guardamos el valor digital de la temperatura (raw)
 
  // hacemos los calculos necesarios para convertir desde valores raw a normalizados en mbares y grados centigrados según datasheet del fabricante del MS5611
  dT = D2 - ((long)calibrationDataMS5611[5] << 8);
  TEMP = (2000 + (((int64_t)dT * (int64_t)calibrationDataMS5611[6]) >> 23)) / (float)100;
  OFF = ((unsigned long)calibrationDataMS5611[2] << 16) + (((int64_t)calibrationDataMS5611[4] * dT) >> 7);
  SENS = ((unsigned long)calibrationDataMS5611[1] << 15) + (((int64_t)calibrationDataMS5611[3] * dT) >> 8);
  P = (((D1 * SENS) >> 21) - OFF) >> 15;
  
  baro_temp_float = TEMP;               // copiamos resultado  
  //pressure_float = float (P)/100;     // copiamos resultado 
  //Serial.println(TEMP);               // imprimimos temperatura
  //Serial.println(P);                  // imprimimos presión
  return P;
} 

/* -----------------------getAltitude (param1, param2)-------------------------- 
 * Calcula la altura respecto al nivel del mar.
 * @ param1 - QFE, presión medida por MS5611 en pascales (ejemplo - 102650)
 * @ param2 - QNH, presión a nivel del mar en pascales (ejemplo - 101325).
 * @ return - altura en metros en metros (ejemplo - 143.25)
 */
float MS5611::getAltitude (long _measuredPressure)  // devuelve la altura en metros
//float MS5611::getAltitude (long _measuredPressure, long _seaLevelPressure)  // devuelve la altura en metros
{
  measuredPressure = _measuredPressure;
  //seaLevelPressure = _seaLevelPressure;
  
  return (float)44330 * (1 - pow(((float) measuredPressure/seaLevelPressure), 0.190295));
}

float MS5611::getTemp ()
{
  if (baro_temp_float != 0)     // si la variable no tiene el valor con el que se inicializó
  {
    return baro_temp_float;     // devuelve su valor
  }
  else                          // sino 
  {
    getPressure();              // lee el sensor
    return baro_temp_float;     // y devuelve la temperatura
  }
}

long MS5611::getSeaLevelPressure()
{
  return seaLevelPressure;
}

// -------------- SETTERS -----------------------

void MS5611::setSeaLevelPressure (long _seaLevelPressure)
{
  seaLevelPressure = _seaLevelPressure;
}

// -------------- FUNCIONES -----------------------

void MS5611::pausa(unsigned long microsegundos)
{
   volatile unsigned long objetivo = micros()+ (microsegundos); 
   do
   {
     //no hace nada 
   } while (micros()<objetivo);
   return;
}

