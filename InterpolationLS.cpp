/*
 * InterpolationLS.cpp 
 * Interpolation Least Squares (en inglés)
 * Interpolación de mínimos cuadrados (en castellano)
 * 
 * m = Slope (Pendiente de la recta)
 * b = y intercept (interseción de la recta en eje y)
 *  
 *  ejemplos de uso de los metodos:
 *    m = interpolation.getM ();
 *    b = interpolation.getB ();
 *    y = interpolation.getY (x);
 *  
 *  
 */

#include "Arduino.h"
#include "InterpolationLS.h"

// -------- Constructores ---------
InterpolationLS::InterpolationLS(float *_xVector, float *_yVector, int _samples)  // Aquí le tenemos que pasar los vectores y el numero de elementos de los vectores.
{
  xVector = _xVector; // @ vector x
  yVector = _yVector; // @ vector y
  samples = _samples; // logitud de los vectores
  offset = 0;         // sin desplazamiento del indice de los vectores, (maxima precisión)

  n = samples;// número de muestras del vector
  m = 0;      // pendiente de la recta
  b = 0;      // interseccion de la recta con el eje y
  x = 0;      // valor de x
  y = 0;      // valor de y
  sumX = 0;
  sumY = 0;
  sumXY = 0;
  sumX2 = 0;   
}

InterpolationLS::InterpolationLS(float *_xVector, float *_yVector, int _samples, int _offset)  // Aquí le tenemos que pasar los vectores y el numero de elementos de los vectores.
{
  xVector = _xVector; // @ vector x
  yVector = _yVector; // @ vector y
  samples = _samples; // logitud de los vectores
  offset = _offset;   // desplazamiento del indice de los vectores.

  n = samples-offset ;// número de muestras del vector
  m = 0;              // pendiente de la recta
  b = 0;              // interseccion de la recta con el eje y
  x = 0;              // valor de x
  y = 0;              // valor de y
  sumX = 0;
  sumY = 0;
  sumXY = 0;
  sumX2 = 0;   
}

// ----- Destructor -------
InterpolationLS::~InterpolationLS()
{
  
}

// ----- Métodos -----
float InterpolationLS::getM ()  // calcula pendiente de la recta (m/s)
{
  sumX = 0;
  sumY = 0;
  sumXY = 0;
  sumX2 = 0;
  n = samples-offset;  // ajusta el nº de muestras

  
 
  for(int i=offset; i<n; i++) // sustituir 0 por offset (si no se quiere offset).
  {
    
    // debido a que se producia desbordamientos, el código original no es valido para el proyecto
    /*   
    sumX += (xVector[i]);
    sumY += yVector[i];
    sumXY += (xVector[i])*(yVector[i]);
    sumX2 += (xVector[i])*(xVector[i]);
    */
    // Se ha tenido que modificar el código para calcular la pendiente de la recta, debido 
    // a que las x (millis) tiene numeros muy largos, al sumar sus potencias o al sumar los 
    // productos de xy (millis*altitudes) no cabian dentro de los 4 bytes que tiene un float.
    // Ello provocaba que se produjeran ovf (dando resultado erroneos en el calculo de la pendiente)
    // Esto obliga a trabajar con los tiempos parciales en vez de con los totales. De tal manera
    // que vamos restando el valor de la primera posición del vector a cada uno de los valores 
    // del resto de posiciones. Es decir se ha de restar xVector[offset] a cada xVector[i].
    sumX += (xVector[i]-xVector[offset]);
    sumY += yVector[i];
    sumXY += (xVector[i]-xVector[offset])*yVector[i];
    sumX2 += (xVector[i]-xVector[offset])*(xVector[i]-xVector[offset]);
  }

  m = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX); 

  return  m;
}

float InterpolationLS::getB ()  // calcula interseccion de la recta en el eje y
{
  sumX = 0;
  sumY = 0;
  sumXY = 0;
  sumX2 = 0;
  n = samples;

  for(int i=0; i<n; i++)
  {
    sumX += xVector[i];
    sumY += yVector[i];
    sumXY += (xVector[i]*yVector[i]);
    sumX2 += (xVector[i]*xVector[i]);
  }

  b = (sumX2 * sumY - sumX * sumXY) / (n * sumX2 - sumX * sumX);

  return b;
}

float InterpolationLS::getY (float _x)    // clcula el valor de y para valor x determindado.
{
  x = _x;

  return (m*x)+b;
}

void InterpolationLS::setOffset(int _j)  // SETea el valor de offset
{
  offset = _j; 
}

