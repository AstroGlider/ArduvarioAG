/*
 * InterpolationLS.h 
 * Interpolation Least Squares (en inglés)
 * Interpolación de mínimos cuadrados (en castellano)
 * 
 *  m = Slope (Pendiente de la recta)
 *  b = y intercept (interseción de la recta en eje y)
 *  
 *  ejemplos de uso de los metodos:
 *  
 *    m = interpolation.getM ();
 *    b = interpolation.getB ();
 *    y = interpolation.getY (x);
 *    
 */

#ifndef INTERPOLATION_LS_H        // Guardian de Inclusión múltiple
#define INTERPOLATION_LS_H

#include "Arduino.h"

class InterpolationLS {           // Definición de la clase

  private:
    float *xVector;               // @ del vector de las x
    float *yVector;               // @ del vector de las y
    int samples;                  // longitud de los vectores (la real)
    int offset;                   // desplazamiento del indice de los vectores. Sirve para
                                  // ajustar la ventana de muestras. A mayor nº, más precision y más lento.
                                  // A menor nº de muestras más rápido, menor precisión
                                  // (0 --> maximo nº de muestras) (n-1 --> menor nº de muestras posible)
    
    int n;                        // longuitud de los vectores tras aplicar offset
    float x;                      // valor de x
    float y;                      // valor de y
    float m;                      // pendiente de la recta
    float b;                      // interseccion de la recta con el eje y
 
    float sumX;                   // guarda la suma de todas las x
    float sumY;                   // guarda la suma de todas las x
    float sumXY;                  // guarda la suma del producto xy
    float sumX2;                  // guarda la suma del cuadrado de x
    
  public:
    InterpolationLS (float *xVector, float *yVector, int samples);
    InterpolationLS (float *xVector, float *yVector, int samples, int offset);
    ~InterpolationLS();
    
    float InterpolationLS::getM ();  // pendiente de la recta
    float InterpolationLS::getB ();  // interseción de la recta con eje Y
    //float getY (float x, float m, float b);   // resuelve la función y=mx+b  
    float getY (float x);            // resuelve la función y=mx+b para un valor dado de x
    void setOffset (int j);          // SETea el valor de offset
};

#endif // INTERPOLATION_LS_H
