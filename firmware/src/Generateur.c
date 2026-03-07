// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber


#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "app.h"

// T.P. 2016 100 echantillons
#define MAX_ECH 100
int32_t tableauValDAC[MAX_ECH];


// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    pParam->Frequence = 20;    
    pParam->Amplitude = 10000; 
    pParam->Offset = 0; 
    pParam->Magic = MAGIC; 
    pParam->Forme = SignalTriangle;
}
  

// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    static uint16_t compteurT3 = 0;
    compteurT3 = (uint16_t)(100000 / pParam->Frequence); //Fréquence timer 3 en HZ  
    PLIB_TMR_Period16BitSet(TMR_ID_3, compteurT3);       //Mise à jour de la période du timer 3
}

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
   int16_t nbr_echantillon = 0; // Compteur d'échantillons
   int32_t amplitude; // Amplitude aboslue
   int32_t offset; // Offset
   int32_t valSignal = 0; // Valeur brute du signal
   

    amplitude = (int32_t)(pParam->Amplitude * 3.2767); 
    offset = (pParam->Offset * 3276); //3276 = pas de 1V sur DAC
    
    while(nbr_echantillon < MAX_ECH)
    {    
        switch(pParam->Forme)
        {        
            case SignalCarre: 
                if ((MAX_ECH / 2) > nbr_echantillon) 
                {
                  valSignal = 32767 + offset + amplitude;
                } 
                else 
                {
                  valSignal = 32767 + offset - amplitude;
                }
                break;

           case SignalDentDeScie:
                valSignal = 32767 + offset - amplitude + ((2 * amplitude * nbr_echantillon) / MAX_ECH);
                break;

            case SignalTriangle:
                if ((MAX_ECH / 2) > nbr_echantillon) 
                {
                  //Montee
                  valSignal = 32767 + offset - amplitude + ((4 * amplitude * nbr_echantillon) / MAX_ECH);
                } 
                else 
                {
                  //Descente
                  valSignal = 32767 + offset + amplitude - ((4 * amplitude * (nbr_echantillon - MAX_ECH/2)) / MAX_ECH);
                }
                break;

            case SignalSinus:


                break;
        }  
    
    //Prévention de la saturation
    AntiStaturation(&valSignal);
    //Mise dans le tableau de data
    tableauValDAC[nbr_echantillon] = valSignal;
    nbr_echantillon++;
    }
}


// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)

void  GENSIG_Execute(void)
{
   
   uint16_t EchNb = 0;
   const uint16_t Step = 65535 / MAX_ECH;
   uint16_t cmpt = 0;
   uint16_t valBrute = 0;
   /*
   SPI_WriteToDac(0, Step * EchNb );      // sur canal 0
   EchNb++;
   EchNb = EchNb % MAX_ECH;
   */
   /*
   while(cmpt < MAX_ECH)
   {
       
       if ((MAX_ECH / 2) > cmpt) 
       {
         valBrute = 32767 + (0 * -1 * 3276) + 32767;
         //valBrute = 32767 + 0 - 10000 + ((4 * 10000 * cmpt) / MAX_ECH);  
       } 
       else 
       {
         valBrute = 32767 + (0 * -1 * 3276) - 32767;
         //valBrute = 32767 + 0 + 10000 - ((4 * 10000 * (cmpt - MAX_ECH/2)) / MAX_ECH);  
       }
       
       //valBrute = 32767 + 0 - 10000 + ((2 * 10000 * cmpt) / MAX_ECH);
       
       tableauValDAC[cmpt] = valBrute;
       cmpt++;
   }
   
   SPI_WriteToDac(0, tableauValDAC[EchNb]);
   EchNb++;
   EchNb = EchNb % MAX_ECH;
   */
   while(EchNb < MAX_ECH)
   {
      SPI_WriteToDac(0, tableauValDAC[EchNb]); 
      EchNb++;
   }    
}


// Fonction anti-saturation de valeure
void AntiStaturation(int32_t *val)
{
    //Saturation
    if(*val > 65535)
    {
        *val = 65535;
    }

    if(*val < 0)
    {
        *val = 0;
    }
    
}
