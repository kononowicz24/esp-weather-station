//The code is licensed under GNU GPLv3.

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define ILOSC_POMIAROW 5
#define RESOLUTION_TEMP   5000
#define RESOLUTION_HUMI   5000
#define RESOLUTION_PRES 300000

#include "passwords.h"

float currTemp=0.0f;
float currHumi=0.0f;
float currPres=0.0f;
float currTemp2=0.0f;

//char currTempS[7];
//char currHumiS[7];

const char colors[][7] = { "black", "blue", "black", "gray",    "red"};
const char units[][7] =  {"&deg;C",  "%RH",     "s",  "hPa", "&deg;C"};

int sec=0, min=0, hr=0;

String startUpTime;

std::list <std::vector <float> > History;

std::vector <std::vector <float> > Statistics; // zewnetrzny to poszczegolne staty, srodkowy poszczegolne mierniki
// max / min / avg / avg(1h) / n

std::vector <float> deltas;
std::vector <std::vector <float> > absolutes;

float cC[ILOSC_POMIAROW];
char cCS[ILOSC_POMIAROW][9];

boolean softap=0;

#endif
