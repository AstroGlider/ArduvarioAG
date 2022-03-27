/*
 * VarioBeep.h
 * Libreria que reproduce el sonido de un variometro en función 
 * de la tasa de ascenso/descenso en m/s.
 */


#ifndef VARIO_BEEP_H   // Guardian de inclusión múltiple
#define VARIO_BEEP_H             

#include "Arduino.h"



class VarioBeep {       // ---- Definición de la clase ----

  private:              // ---- Atributos ----
    byte buzzPin;       // Pin del zumbador.
    float vario;        // variación de la altura en m/s.
    float offset;       // conversión de la variable vario a un valor de frecuencia que se le suma a baseFreq para calcular la frecuencia final a sonar (freq).   
    int baseFreq;       // Frecuencia Base - es la freq incialrl inicial el sonido correspondiente al umbral de ascenso.
    int freq;           // Frecuencia - (baseFreq + offset)  en función de la tasa de ascenso/descenso + la frecuencia base
    boolean dynFreq;    // Frecuencia Dinamica - varia el tono mientras suena si la tasa de subida cambia.
    boolean cadence;    // Cadencia - existen dos cadencias configuradas, una es más curva que la otra. 
    float climbTh ;     // Umbral de Ascenso - tasa de ascenso mínima a la que empieza a sonar.
    float sinkTh;       // Umbral inicio de tono de descenso - emite tono continuo a baja frecuencia 
    float sinkAlarm;    // Velocidad de descenso peligrosa. Alarma
    boolean quiteMode;  // Modo Silencioso - indica si se quiere que suene el zumbador o no.
    
    unsigned long beepStart;      // Indica cuando empieza a sonar un beep.
    unsigned long silenceStart;   // Indica cuando empieza un silencio.
    unsigned long beepPeriod;     // tiempo que ha de permanecer sonando cada beep
    unsigned long silencePeriod;  // tiempo que ha de permanecer en silencio entre beep y beep.
    unsigned long tiempo;         // timeStamp, se le pasará millis().

    unsigned long lastBeepPeriod;     // el tiempo que ha de permanecer sonando (sin frecuencia dinamica)
    unsigned long lastSilencePeriod;  // tiempo que ha de permanecer en silencio entre Beep y Beep (sin frecuencia dinamica)
     
    boolean muted;                // indica si está en periodo de silencio.
    boolean beepEnd;              // indica si ha finalizado un beep.
    boolean silenceEnd;           // indica si ha finalizado un silencio.
    

  public:                                   // ---- Métodos ----
    VarioBeep (byte buzzPin);               // Constructor1, solo se indica pin del zumbador.
    //VarioBeep (byte buzzpin,bool DynFreq);  // Constructor2, se le pasa tambien DynFreq 
    
    ~VarioBeep();                           // Destructor
      
    void play (float vario);                // Hace sonar el zumbador (o no).
    void setClimbTh (float climbTh);        // "SETea" el umbral en (m/s)en el que empieza a sonar.
    void setCadence (bool cadence);         // "SETea" con que cadencia sonar.
    void setDynFreq (bool dynFreq);         // "SETea" si ha de sonar con frecuencia dinamica o no.
    void setBaseFreq (int baseFreq);        // "SETea" la frecuencia base.
    void setQuiteMode (bool quiteMode);     // "SETea" si queremos que suene o no.
    void setSinkTh (float sinkTh);          // "SETea" Umbral inicio de tono de descenso - emite tono continuo a baja frecuencia
    void SetSinkAlarm (float sinkAlarm);    // "SETea" alarma de velocidad de descenso peligrosa. 
    //------getters
    bool getCadence ();
    bool getDynFreq ();
    bool getQuiteMode();
    int getBaseFreq();
    float getClimbTh ();
    float getSinkTh ();
    float getSinkAlarm();
    
    //-------
    float mapFloat(float, float, float, float, float);  // función map para floats.
   
};     // Fin de la definición de la clase VarioBeep

#endif // VARIO_BEEP_H

