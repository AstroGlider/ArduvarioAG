/*
 * VarioBeep.ccp
 * Libreria que reproduce el sonido de un variometro en función 
 * de la tasa de ascenso/descenso en m/s.
 */

#include "Arduino.h"
#include "VarioBeep.h"      // carga fichero de cabecera de esta libreria.

// -------------CONSTRUCTORES ---------------------------

VarioBeep::VarioBeep(byte _buzzPin)  // Constructor1, solo se indica pin del zumbador
{
  pinMode (_buzzPin, OUTPUT);
  buzzPin = _buzzPin;
  baseFreq = 600;             // Frecuencia Base - es la freq al inicial el sonido correspondiente al umbral de ascenso.
  dynFreq = 1;                // Frecuencia Dinamica - varia el tono mientras suena si la tasa de subida cambia.
  cadence = 1;                // Cadencia - existen dos cadencias configuradas, una es más curva que la otra.
  climbTh = 0.25;             // Umbral de Ascenso - tasa de ascenso mínima a la que empieza a sonar.
  sinkTh = -1.0;              // Umbral inicio de tono de descenso - emite tono continuo a baja frecuencia
  sinkAlarm = -4.5;           // Velocidad de descenso peligrosa. Alarma
  beepStart = millis();       // Indica cuando empieza a sonar un beep.
  silenceStart = millis();    // Indica cuando empieza un silencio.
  beepPeriod = 400;           // tiempo que ha de permanecer sonando cada Beep
  silencePeriod = 440;        // tiempo que ha de permanecer en silencio entre Beep y Beep.

  lastBeepPeriod = beepPeriod;        // el tiempo que ha de permanecer sonando (sin frecuencia dinamica)
  lastSilencePeriod = silencePeriod;  // tiempo que ha de permanecer en silencio entre Beep y Beep (sin frecuencia dinamica)
  
  muted = true;               // indica si está en periodo de silencio.
  beepEnd = true;             // indica si ha finalizado un beep.
  silenceEnd = false;         // indica si ha finalizado un silencio.
  quiteMode = false;          // Modo Silencioso - Activa o desactiva el sonido.

}

VarioBeep::~VarioBeep()   // Destructor 
{
  
}

// --------------- MÉTODOS -------------------------

void VarioBeep::play (float _vario) // Método principal, calcula el tono y periodo en función de la tasa de ascenso/descenso (Rate of Climb / Sink Rate)
{
  tiempo = millis();                // timestamp
  vario = _vario;                   // tasa ascenso/descenso en m/s
  offset = vario * 100;             // adapto los m/s a un valor que pueda luego sumarle a la frecuencia base
  freq = baseFreq + offset;         // Frecuencia del tono a sonar varia en función de los m/s
  
  if (quiteMode == 0)
  {
    if (muted) {beepStart = tiempo;}  // si está en periodo de silencio reseteamos la hora a la que empezó a sonar, para que no se alcance el beepPeriod y empiece a sonar mientras estamos dentro del Periodo de silencio.
  //------------------------------------------------------------------------------------------------  
    //if ((vario > sinkTh + (0.5)  && vario < 0)||(vario < sinkTh + (-2.5) && vario > sinkAlarm))  // Si estamos en zona descenso en la que no deba emitir sonido...
    if ((vario > sinkTh && vario <climbTh)||(vario < sinkTh + (-2.5) && vario > sinkAlarm))  // Si estamos en zona descenso en la que no deba emitir sonido...
    {
      noTone (buzzPin);
    }
    else if (vario <= sinkTh && vario >= sinkTh + (-2.5))   // Si estamos en zona de tono de descenso....
    {
      freq = mapFloat (vario, sinkTh + (-2.5), sinkTh, 100, 550); 
      tone (buzzPin, freq);                                 
    }  
    else if (vario <= sinkAlarm)                            // Si estamos  en zona de descenso peligroso...
    {
      tone (buzzPin, 1100, 1000);         
    }
  //-----------------------------------------------------------------------------------------------------------------------------  
    else                                                    // si estamos en zona de ascenso positivo vario > 0...
    {                                                       // Calculamos los periodos de sonido y de silencio en función de la tasa de ascenso.  
      if (cadence == 1)  
      {
        if (vario>=climbTh && vario<8) //verifica si estamos en tasa de ascenso
        {
          if (vario>.03 && vario<=.15)
          {
             beepPeriod = 475;
          }
          else if (vario>.15 && vario<=.35) 
          {
            beepPeriod = mapFloat (vario, 0, 0.35, 475, 400);          //mapea de 475ms a 400ms
          }
          else if (vario>.35 && vario<=.55)
          {
            beepPeriod = mapFloat (vario, 0.35, 0.55, 400, 337);
          }
          else if (vario>.55 && vario<=0.75)
          {
            beepPeriod = mapFloat (vario, 0.55, 0.75, 337, 287);
          }
          else if (vario>0.75 && vario<=0.85)
          {
             beepPeriod = mapFloat (vario, 0.75, 0.85, 287, 269);
          } 
          else if (vario>0.85 && vario<=.95)
          {
             beepPeriod = mapFloat (vario, 0.85, 0.95, 269, 244);
          }  
          else if (vario>0.95 && vario<=1.05)
          {
             beepPeriod = mapFloat (vario, 0.95, 1.05, 244, 225);
          }
          else if (vario>1.05 && vario<=1.15)
          {
             beepPeriod = mapFloat (vario, 1.05, 1.15, 225, 212);
          }
          else if (vario>1.15 && vario<=1.25)
          {
             beepPeriod = mapFloat (vario, 1.15, 1.25, 212, 194);
          }
          else if (vario>1.25 && vario<=1.45)
          {
             beepPeriod = mapFloat (vario, 1.25, 1.45, 194, 169);
          }
          else if (vario>1.45 && vario<=1.55)
          {
             beepPeriod = mapFloat (vario, 1.45, 1.55, 169, 162);
          }
          else if (vario>1.55 && vario<=1.65)
          {
             beepPeriod = mapFloat (vario, 1.55, 1.65, 162, 150);
          }
          else if (vario>1.65 && vario<=1.75)
          {
             beepPeriod = mapFloat (vario, 1.65, 1.75, 150, 144);
          }
          else if (vario>1.75 && vario<=1.85)
          {
             beepPeriod = mapFloat (vario, 1.75, 1.85, 144, 131);
          }
          else if (vario>1.85 && vario<=2.25)
          {
             beepPeriod = mapFloat (vario, 1.85, 2.25, 131, 106);
          }
          else if (vario>2.25 && vario<=2.35)
          {
             beepPeriod = 106;
          }
          else if (vario>2.35 && vario<=2.45)
          {
             beepPeriod = mapFloat (vario, 2.35, 2.45, 106, 100);
          }
          else if (vario>2.45 && vario<=2.55)
          {
             beepPeriod = 100;
          }
          else if (vario>2.55 && vario<=2.65)
          {
             beepPeriod = mapFloat (vario, 2.55, 2.65, 100, 94);
          }
          else if (vario>2.65 && vario<=2.75)
          {
             beepPeriod = 94;
          }
          else if (vario>2.75 && vario<=2.85)
          {
             beepPeriod = mapFloat (vario, 2.75, 2.85, 94, 88);
          }
          else if (vario>2.85 && vario<=3.05)
          {
             beepPeriod = 88;
          }
          else if (vario>3.05 && vario<=3.15)
          {
             beepPeriod = mapFloat (vario, 3.05, 3.15, 88, 81);
          }
          else if (vario>3.15 && vario<=3.45)
          {
             beepPeriod = 81;
          }
          else if (vario>3.45 && vario<=3.55)
          {
             beepPeriod = mapFloat (vario, 3.45, 3.55, 81, 75);
          }
          else if (vario>3.55)
          {
             beepPeriod = 75;
          } 
        }
      }
      else if (cadence == 0)
      {
          if (vario>=.03 && vario<8)           //verifica si estamos en tasa de ascenso
          { 
           
             if (vario>0.0 && vario<=0.15){
                beepPeriod = 400;
             }
             else if (vario>.15 && vario<=.30) {
               beepPeriod = mapFloat (vario, 0, 0.30, 400, 350);          //mapea de 400ms a 350ms
             }
             else if (vario>.30 && vario<=.70){
               beepPeriod = mapFloat (vario, 0.30, 0.70, 350, 294);
             }
             else if (vario>.70 && vario<=0.85){
               beepPeriod = mapFloat (vario, 0.70, 0.85, 294, 281);
             }
             else if (vario>0.85 && vario<=0.95){
                beepPeriod = mapFloat (vario, 0.85, 0.95, 281, 269);
             } 
             else if (vario>0.95 && vario<=1.05){
                beepPeriod = mapFloat (vario, 0.95, 1.05, 269, 263);
             }  
             else if (vario>1.05 && vario<=1.25){
                beepPeriod = mapFloat (vario, 1.05, 1.25, 263, 237);
             }
             else if (vario>1.25 && vario<=1.35){
                beepPeriod = mapFloat (vario, 1.25, 1.35, 237, 231);
             }
             else if (vario>1.35 && vario<=1.45){
                beepPeriod = mapFloat (vario, 1.35, 1.45, 231, 219);
             }
             else if (vario>1.45 && vario<=1.75){
                beepPeriod = mapFloat (vario, 1.45, 1.75, 219, 200);
             }
             else if (vario>1.75 && vario<=1.85){
                beepPeriod = mapFloat (vario, 1.75, 1.85, 200, 187);
             }
             else if (vario>1.85 && vario<=2.45){
                beepPeriod = mapFloat (vario, 1.85, 2.45, 187, 150);
             }
             else if (vario>2.45 && vario<=2.55){
                beepPeriod = 150;
             }
             else if (vario>2.55 && vario<=2.85){
                beepPeriod = mapFloat (vario, 2.55, 2.85, 150, 131);
             }
             else if (vario>2.85 && vario<=2.95){
                beepPeriod = 131;
             }
             else if (vario>2.95 && vario<=3.15){
                beepPeriod = mapFloat (vario, 2.95, 3.15, 131, 119);
             }
             else if (vario>3.15 && vario<=3.25){
                beepPeriod = 119;
             }
             else if (vario>3.25 && vario<=3.35){
                beepPeriod = mapFloat (vario, 3.25, 3.35, 119, 113);
             }
             else if (vario>3.35 && vario<=3.45){
                beepPeriod = 113;
             }
             else if (vario>3.45 && vario<=3.55){
                beepPeriod = mapFloat (vario, 3.45, 3.55, 113, 106);
             }
             else if (vario>3.55 && vario<=3.75){
                beepPeriod = 106;
             }
             else if (vario>3.75 && vario<=3.85){
                beepPeriod = mapFloat (vario, 3.75, 3.85, 106, 100);
             }
             else if (vario>3.85 && vario<=3.95){
                beepPeriod = 100;
             }
             else if (vario>3.95 && vario<=4.05){
                beepPeriod = mapFloat (vario, 3.95, 4.05, 100, 94);
             }
             else if (vario>4.05 && vario<=4.35){
                beepPeriod = 94;
             }
             else if (vario>4.35 && vario<=4.45){
                beepPeriod = mapFloat (vario, 4.35, 4.45, 94, 87);
             }
             else if (vario>4.45 && vario<=4.75){
                beepPeriod = 87;
             }
             else if (vario>4.75 && vario<=4.85){
                beepPeriod = mapFloat (vario, 4.75, 4.85, 87, 81);
             }
             else if (vario>4.85 && vario<=5.45){
                beepPeriod = 81;
             }
             else if (vario>5.45 && vario<=4.55){
                beepPeriod = mapFloat (vario, 5.45, 4.55, 81, 75);
             }
             else if (vario>4.55){
                beepPeriod = 75;
             }
          }
      }
      silencePeriod=beepPeriod*1.1;
    
      if ((dynFreq == 0)&&(beepEnd == false)){beepPeriod = lastBeepPeriod;}
      if ((dynFreq == 0)&&(silenceEnd == false)){silencePeriod = lastSilencePeriod;}
       
      //Serial.println ("---------");
      //Serial.println (vario);
      //Serial.println (beepPeriod);
      //Serial.println (silencePeriod);
       
      // compruebo si toca sonar o silicenciar----
      if (tiempo - beepStart >= beepPeriod)
      {
        beepEnd = true;
        silenceEnd = false;
      }
      else if (tiempo - silenceStart >= silencePeriod)
      {
        silenceEnd = true;
        beepEnd = false;
      }
       
       // Ahora Suena con frecuencia dinámica--------  
       if (((muted && silenceEnd) || (!muted && !beepEnd))&& (vario >= climbTh) && (dynFreq == 1))
       {
         tone (buzzPin, freq, beepPeriod);
         if (muted)
         {
           beepStart = millis ();
           muted = false;
         }
       }
       // Ahora Suena con frecuencia estatica--------
       else if ((muted && silenceEnd) && (vario >= climbTh) && (dynFreq == 0))
       {
         tone (buzzPin, freq, beepPeriod);
         beepStart = millis ();
         muted = false;
         lastBeepPeriod = beepPeriod;     
       }
       // Ahora no suena -----
       else if (!muted && beepEnd)
       {
         noTone (buzzPin);
         silenceStart = millis ();
         muted = true;
         lastSilencePeriod=silencePeriod;
       }
    }
  }    //end if quiteMode==0
  else // si quiteMode==1
  {
    noTone (buzzPin);
  }
}

// --------------- SETTERS ------------------------

void VarioBeep::setClimbTh (float _climbTh) // "SETea" el umbral en (m/s)en el que empieza a sonar.
{
  climbTh = _climbTh;
}

void VarioBeep::setCadence (bool _cadence) // "SETea" con que cadencia sonar.
{
  cadence = _cadence;
}

void VarioBeep::setDynFreq (bool _dynFreq) // "SETea" si ha de sonar con frecuencia dinamica o no.
{
  dynFreq = _dynFreq;
}

void VarioBeep::setBaseFreq (int _baseFreq) // "SETea" la frecuencia base.
{
  baseFreq = _baseFreq;
}  

void VarioBeep::setQuiteMode (bool _quiteMode) // "SETea" si queremos que suene o no.
{
  quiteMode = _quiteMode;
}

void VarioBeep::setSinkTh (float _sinkTh)    // "SETea" Umbral inicio de tono de descenso - emite tono continuo a baja frecuencia
{
  sinkTh = _sinkTh;
}
    
void VarioBeep::SetSinkAlarm (float _sinkAlarm)    // "SETea" alarma de velocidad de descenso peligrosa. 
{
  sinkAlarm = _sinkAlarm;
}

// -------------- GETTERS -----------------------

bool VarioBeep::getCadence ()
{
  return cadence;
}
bool VarioBeep::getDynFreq ()
{
  return dynFreq;
}
bool VarioBeep::getQuiteMode()
{
  return quiteMode;
}
int VarioBeep::getBaseFreq()
{
  return baseFreq;
}
float VarioBeep::getClimbTh ()
{
  return climbTh;
}
float VarioBeep::getSinkTh ()
{
  return sinkTh;
}
float VarioBeep::getSinkAlarm()
{
  return sinkAlarm;
}


// -------------- FUNCIONES -----------------------

float VarioBeep::mapFloat(float x, float in_min, float in_max, float out_min, float out_max)  // funcion map para floats
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

