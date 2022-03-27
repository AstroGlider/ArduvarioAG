/*
 * MS5611.h
 * Libreria que lee sensor barométrico MS5611 y devuelve la presión, temperatura
 * y altura.
 * 
 */

#ifndef MS5611_H                                      // Guardian de inclusión múltiple
#define MS5611_H       

#include "Arduino.h"
#include "Wire.h"

class MS5611 {                                        // ---- Definición de la clase ----

  private:                                            // ---- Atributos ----
    byte address;                                     // @ del sensor MS5611
    unsigned int calibrationDataMS5611[7];            // vector donde se guardan los datos de calibración de fábrica del MS5611
    float baro_temp_float;                            // Temperatura leída en grados centígrados (ejemplo 25.58 ºC) 
    long seaLevelPressure;                            // Presión a nivel del mar en pascales (ejemplo - 101325)
    long measuredPressure;                            // presión atmosférica en pascales (ejemplo - 102650)


  public:                                             // ---- Métodos ----
    MS5611 (byte address);                            // Constructor1, solo se indica la @ del sensor
    
    ~MS5611();                                        // Destructor
      
    void begin ();                                    // Inicializa el sensor
    long getPressure ();                              // Calcula la presión atmosferica leída en MS5611 en Pascales
    float getAltitude (long measuredPressure);  // devuelve la altura en metros
    //float getAltitude (long measuredPressure, long seaLevelPressure);  // devuelve la altura en metros
    float getTemp ();
    long getSeaLevelPressure();                       // devuelve la presión atmosférica a nivel del mar (QNH)
    void setSeaLevelPressure (long seaLevelPressure); // "SETea" la presión atmosférica a nivel del mar (QNH)
    

    void pausa (unsigned long microsegundos);  // función map para floats.
    
};     // Fin de la definición de la clase MS5611

#endif // MS5611_H
