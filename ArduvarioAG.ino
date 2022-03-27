 /* 
  *  ArduVarioAG V1.3
  *  
  *  realizado por mi mismo :) AstroGlider en GitHub
  *  
  *  Hardaware utilizado: sensor MS5611, un zumbador, arduino mini PRO y modulo Bluetooth
  *  
  *  Envía tramas tipo NMEA que reconoce la aplicación XCTRACK
  *  
  *  Se ha basado en los codigos siguientes:
  *  
// http://taturno.com/code/VariometroV2.pde
// http://taturno.com/2011/10/30/variometro-la-rivincita/
// https://www.instructables.com/id/GoFly-paraglidinghanglidinggliding-altimeter-v/
  *  
  *  primera versión del Vario totalmente funcional
  *  utilizando mis 3 librerías "InterpolationLS"  "VarioBeep" y "MS5611"
  * 
  * setCadence -- Funciona
  * setDynFreq -- Funciona
  * setQuiteMode -- Funciona
  * setBaseFreq -- Funciona
  * setClimbTh -- Funciona
  * setSinkTh -- Funciona
  * SetSinkAlarm -- Funciona
  * 
  * getM -- Funciona (Se ha modificado la libreria InterpolationLS para evitar desbordamientos
  * en el calculo de la tasa de ascenso)
  * 
  * getPressure -- Funciona
  * getAltitude -- Funciona
  * 
  * 
  * ---- Aspecto a mejorar ----------------------------------------
  * Dos vectores float de 64 elementos (para tiempos y alturas) son 512 bytes de memoria dinamica. 
  * Se podría utilizar un vector de usigned int para guardar los tiempos parciales en vez de los
  * tiempos totales. Esto nos ocuparia 256 + 128 = 384 bytes frente a los 512 actuales.
  * 
  * Podríamos hacer lo mismo para el vector de alturas, pasando de 512 bytes a 256 bytes aprox.
 
-------------------------------------------------------------------
  $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1
   0 loger_stored (Y/N)
   1 IAS (kph) ----> Condor uses TAS!
   2 baroaltitude (m)
   3-8 vario (m/s) (last 6 measurements in last second)
   9 heading of plane
  10 windcourse (deg)
  11 windspeed (kph)

 $LXWP0,N,,598.8,0.01,,,,,,,,*50
 $LXWP0,N,,598.9,0.02,,,,,,,,*52
 
 The LXWP0 output mode for use with a range of apps: 
        "$LXWP0,loger_stored (Y/N), IAS (kph), baroaltitude (m), vario (m/s),,,,,,heading of plane,windcourse (deg),windspeed (kph)*CS"
        The BlueFlyVario only has a partial implementation of this sentence. 
        It only outputs the baroaltitude and vario (all other fields are blank). 
        Note that baroaltidude is determined from filtered pressure using the outputQNH setting.
-----------------------------------------------------------------
LK8000 EXTERNAL INSTRUMENT SERIES 1 - NMEA SENTENCE: LK8EX1
VERSION A, 110217

LK8EX1,pressure,altitude,vario,temperature,battery,*checksum

Field 0, raw pressure in hPascal:
 hPA*100 (example for 1013.25 becomes  101325) 
  no padding (987.25 becomes 98725, NOT 098725)
  If no pressure available, send 999999 (6 times 9)
  If pressure is available, field 1 altitude will be ignored

Field 1, altitude in meters, relative to QNH 1013.25
  If raw pressure is available, this value will be IGNORED (you can set it to 99999
  but not really needed)!
  (if you want to use this value, set raw pressure to 999999)
  This value is relative to sea level (QNE). We are assuming that
  currently at 0m altitude pressure is standard 1013.25.
  If you cannot send raw altitude, then send what you have but then
  you must NOT adjust it from Basic Setting in LK.
  Altitude can be negative
  If altitude not available, and Pressure not available, set Altitude
  to 99999  (5 times 9)
  LK will say "Baro altitude available" if one of fields 0 and 1 is available.

Field 2, vario in cm/s
  If vario not available, send 9999  (4 times 9)
  Value can also be negative

Field 3, temperature in C , can be also negative
  If not available, send 99

Field 4, battery voltage or charge percentage
  Cannot be negative
  If not available, send 999 (3 times 9)
  Voltage is sent as float value like: 0.1 1.4 2.3  11.2 
  To send percentage, add 1000. Example 0% = 1000
  14% = 1014 .  Do not send float values for percentages.
Percentage should be 0 to 100, with no decimals, added by 1000!

$LK8EX1,101133,99999,22,36,1000,*2F
  * -------------------------------------------
  * 
  * De momento funciona todo.... :) 
  * 
  * version 1.1 - Se optimiza función para generar y enviar tramas LK8EX1 y LXWP0 
    version 1.2 - Se modifica la función que calcula la altura sin tener que pasarle el QNH como parametro. lo pillará de la libreria FUNCIONA!!
    Version 1.3 - Se modifica la libreria VarioBeep, para poner getters de distintas variables, hasta ahora solo había setters

*/

// ---- LIBRERIAS ----

#include "InterpolationLS.h"
#include "VarioBeep.h"
#include "MS5611.h"
//#include <SoftwareSerial.h>

// ---- CONSTANTES GLOBALES ----

#define MS5611_ADDRESS   0x77   // @ del MS5611 (0b1110111, 119)
const int LONG_V = 64;          // numero de elementos para cada vector de tiempos y alturas.
#define BUZZPIN 5               // Pin donde está conectado el buzzer. ANTES 8
#define LK8EX1  0               // un 0 indica que se ha seleccionado cabecera de la sentencia LK8EX1
#define LXWP0   1               // un 1 indica que se ha seleccionado cabecera de la sentencia LXWP0

//#define BT_RX   10
//#define BT_TX   11
//#define LED  13


// ----VARIABLES GLOBALES ----

float timestampsV[LONG_V] = {};   // contiene los puntos del eje x (Tiempo en millisegundos)
float altitudesV[LONG_V] = {};    // contiene los puntos del eje y (altura en metros)

float vario = 0;              // guarda el valor de ascenso/descenso en m/s

float altitude = 0;           // se incializa la altura a 0 m para las pruebas.
float timestamp = 0;          // guarda el momento en el que se hace la medición
//long qnh = 101325;            // presión atmosferica corregida con la altura de la estación respecto al nivel del mar
unsigned long pressure = 0;   // 101133Pa, presión leida por el sensor.
float temp = 0;               // 25.7ºC, temperatura leída por el sensor.
byte battery = 0;             // % de bateria 


// ---- INSTANCIAS ----

InterpolationLS Interpolation(timestampsV, altitudesV, LONG_V);
VarioBeep beep (BUZZPIN);
MS5611 ms5611 (MS5611_ADDRESS);
//SoftwareSerial bt1(BT_RX,BT_TX); 

void setup() 
{
  Serial.begin (115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //bt1.begin(9600);  
  Wire.begin();
  Wire.setClock(400000);                        // 400kHz I2C clock. Si da problemas comentar esta línea.
  beep.setSinkTh(-0.3);                         // ajusto umbral de descenso 
  beep.setDynFreq(true);                        // activa frecuencia dinamica (o no).
  ms5611.begin ();                              // inicializa el sensor barométrico
  //ms5611.setSeaLevelPressure (101325);             // pasamos presión standard
  ms5611.setSeaLevelPressure (102260);          // pasamos presión local al nivel del mar (QNH)
  
  for (int i=0; i<LONG_V; i++)                  // inicializa los dos vectores con muestras de altura y su timestamp
    {
      altitude =  ms5611.getAltitude (ms5611.getPressure());
      timestamp = millis();       
      updateFIFO (altitude, altitudesV, LONG_V);    
      updateFIFO (timestamp, timestampsV, LONG_V);   
    }   
}

void loop() 
{
     
  pressure = ms5611.getPressure();              // leemos presión atmosférica del sensor
  timestamp = millis();                         // actualiza "la hora" de la lectura.
  altitude =  ms5611.getAltitude (pressure);   // calculamos altura en base a la presión leida y el QNH
    
  updateFIFO (timestamp, timestampsV, LONG_V);  // desplaza todos los elementos del vector una posición a la izquierda y añade un nuevo valor al final
  updateFIFO (altitude, altitudesV, LONG_V);    // desplaza todos los elementos del vector una posición a la izquierda y añade un nuevo valor al final

  vario = Interpolation.getM()*1000 ;           // calcula variación vertical en m/s
  beep.play(vario);                             // suena el zumbador en función del valor de vario

  sendData (100);                                // envía datos por BT cada  x ms según parametro que se pasa
  
}

//-----------------------FUNCIONES -----------------------------------

// ------- añade 1 elemento a un vector tipo FIFO -------------
// ------- desplaza todos los elementos una posición a la -----
// ------- izquierda y añade uno nuevo por la derecha ---------
void updateFIFO (float newValue, float *vector, int sizeOfVector) 
{
  for(int i=0;i<sizeOfVector;i++)
  {
    vector[i] = vector[i+1];    
  };
  vector[(sizeOfVector-1)] = newValue;
}

// ------- añade 1 cadena de char a una array existente--------
// Se pasan todos los parametros por referencia, incluido el indice.
void appendArray (char *destino, char *origen, byte &indiceDest)
{
  for (byte j=0; j<=lengthOfArray(origen); j++)
  {
    destino[indiceDest] = origen[j];
    indiceDest++;
  }  
  destino[indiceDest-1] = ',';  
}

// -------- calcula el tamaño de una cadena dentro de un array de char (busca el caracter de final de cadena '\0' )
byte lengthOfArray (char*vector) 
{
  for (int i=0; i<40; i++)
  {
    if (vector[i] == '\0')
    {
      return i;
    }  
  }
}

// -------- construye una sentencia LXWP0 o LK8EX1 y la envia por el puerto Serial ----- 
// $LXWP0,N,,598.9,0.02,,,,,,,,*52  
// $LK8EX1,101133,99999,22,36,1000,*2F
void printMSG (byte typeMsg)
{  
  char lk8ex1[] = "LK8EX1";     
  char lxwp0[] = "LXWP0,N,";
  char charArray[10]= "";   // es equivalente a char byteArray[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //byteArray[10] = '\0';
  char outArray[40]= "";    // vector donde se irá construyendo la trama  y se enviará por BT
  byte indexOutArray = 0;   // indice donde empieza a escribir en outArray

  if (typeMsg == 0)         // si selecciona LK8EX1...
  {
    appendArray (outArray, lk8ex1, indexOutArray);  
    dtostrf(pressure, 0, 0, charArray); // (variable numerica, numero de enteros,numero de decimales,array donde se guarda en ASCCI)
    appendArray (outArray, charArray, indexOutArray);
    dtostrf(altitude, 0,0, charArray);
    appendArray (outArray, charArray, indexOutArray);
    dtostrf(vario*100, 0,0, charArray);   // le pasamos  cm/s para adaptarla al formato LK8EX1
    appendArray (outArray, charArray, indexOutArray);
    dtostrf(temp, 0,0, charArray);
    appendArray (outArray, charArray, indexOutArray);
    dtostrf(battery+1000, 0,0, charArray);      // le sumamos 1000 al % para  adaptarlo al formato LK8EX1
    appendArray (outArray, charArray, indexOutArray);
    outArray[indexOutArray] = '\0';
  }
  else if (typeMsg == 1)    // si selecciona LXWP0...
  {
    appendArray (outArray, lxwp0, indexOutArray);
    dtostrf (altitude, 0, 1, charArray); // (variable numerica, numero de enteros,numero de decimales,array donde se guarda en ASCCI)
    appendArray (outArray, charArray, indexOutArray);
    dtostrf (vario, 0, 2, charArray);   
    appendArray (outArray, charArray, indexOutArray);
  
    for (int i=0; i<8; i++)           // los siguientes campos no los informamos.
    {
      outArray[indexOutArray] = ',';
      indexOutArray++;
    }
    outArray[indexOutArray-1] = '\0'; // finalizamos la cadena  
  }
  unsigned int checksum_end, ai, bi;                                               // Calculating checksum for data string
  for (checksum_end = 0, ai = 0; ai < lengthOfArray(outArray); ai++)
  {
    bi = (unsigned char)outArray[ai];
    checksum_end ^= bi;
  }

  // la idea es que la siguiente cadena de caracteres se envíe por Bluetooht a un móvil con la aplicación XCTRACK por ejemplo
  Serial.print (F("$"));
  Serial.print (outArray);
  Serial.print (F("*"));
  Serial.println(checksum_end, HEX);
  
}

// ---- comprueba si es momento de enviar mensaje y realiza media de los valores acumulados antes de enviarlos 
void sendData (unsigned long sendMessagePeriod )
{
  static float storeVario = 0;
  static float storePressure = 0;
  static int counter = 0;

  //static unsigned long sendMessagePeriod = 90;    // intervalo de tiempo entre los mensajes, en ms
  static unsigned long nextMessageTime = 0;        // almacena cuando se enviará el próximo mensaje por BT
  
  storeVario += vario;              // va acumulando  todos los valores de vario que no se han enviado 
  storePressure += pressure;        // va acumulando  todos los valores de pressure que no se han enviado 
  counter++;                        // cuenta el nº de veces que se han acumulado.
  
  if (millis() >= nextMessageTime)    // comprueba si es momento de enviar mensaje o no.
  {
    vario = storeVario/counter;       // calcula la media     
    pressure = storePressure/counter; // calcula la media
    temp = ms5611.getTemp();          // actualiza temperatura
    printMSG (LXWP0);                 // imprime trama tipo LXWP0
    printMSG (LK8EX1);                // imprime trama tipo LK8EX1

    // se imprimen el resto de datos (si se desea)
    Serial.println (F("________________"));
  
    Serial.print (F("millis: "));
    Serial.println (millis());
  
    Serial.print (F("QNH: "));
    long qnh = ms5611.getSeaLevelPressure();
    Serial.print (qnh);
    Serial.println (F(" Pa"));
  
    Serial.print (F("Presion: "));
    Serial.print (pressure);
    Serial.println (F(" Pa"));
  
    Serial.print (F("Altitud: "));
    Serial.print (altitude);
    Serial.println (F(" m"));
    
    Serial.print (F("Vario: "));
    Serial.print (vario,3); 
    Serial.println (F(" m/s"));
  
    Serial.print (F("Temp: "));
    Serial.print (temp);
    Serial.println (F(" º"));
   
    nextMessageTime = millis()+sendMessagePeriod; // actualiza cuando será la próxima vez que se deba enviar.
    storeVario = 0;                   // resetea acumulador
    storePressure =0;                 // resetea acumulador
    counter = 0;                      // resetea contador   
  } 
}
