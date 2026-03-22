#include <stdint.h>

#include "../eeprom.h"

#include "count.h"
#include "eeprom-this.h"

#define INFLEXION_CELL_MV_MAX 15

static int16_t  _inflexionCentreMv      = 0;
static uint8_t  _inflexionCentrePercent = 0;
static uint32_t _inflexionCentreAs      = 0;

static const uint16_t inflexionAmpSeconds[] =
{
     0U * BATTERY_CAPACITY_AH * 36 / 10, // 0
     1U * BATTERY_CAPACITY_AH * 36 / 10, // 1
     2U * BATTERY_CAPACITY_AH * 36 / 10, // 2
     3U * BATTERY_CAPACITY_AH * 36 / 10, // 3
     4U * BATTERY_CAPACITY_AH * 36 / 10, // 4
     5U * BATTERY_CAPACITY_AH * 36 / 10, // 5
     6U * BATTERY_CAPACITY_AH * 36 / 10, // 6
     7U * BATTERY_CAPACITY_AH * 36 / 10, // 7
     8U * BATTERY_CAPACITY_AH * 36 / 10, // 8
     9U * BATTERY_CAPACITY_AH * 36 / 10, // 9
    10U * BATTERY_CAPACITY_AH * 36 / 10, //10
    12U * BATTERY_CAPACITY_AH * 36 / 10, //11
    14U * BATTERY_CAPACITY_AH * 36 / 10, //12
    16U * BATTERY_CAPACITY_AH * 36 / 10, //13
    18U * BATTERY_CAPACITY_AH * 36 / 10, //14
    20U * BATTERY_CAPACITY_AH * 36 / 10  //15
};
int16_t  CurveGetInflexionCentreMv     () { return _inflexionCentreMv;      } void CurveSetInflexionCentreMv     (int16_t v) { _inflexionCentreMv      = v;                                        EepromSaveS16(EEPROM_CURVE_INFLEXION_MV_S16     , v  ); } 
uint8_t  CurveGetInflexionCentrePercent() { return _inflexionCentrePercent; } void CurveSetInflexionCentrePercent(uint8_t v) { _inflexionCentrePercent = v;  _inflexionCentreAs = v * 36UL * 280 ; EepromSaveU8 (EEPROM_CURVE_INFLEXION_PERCENT_U8 , v  ); } 
uint32_t CurveGetInflexionCentreAs     () { return _inflexionCentreAs;      }
int8_t   CurveGetInflexionWidthMv      () { return INFLEXION_CELL_MV_MAX;   }

char     CurveGetInflexionAsFromMv(int16_t mv, uint32_t* pAs) //returns 0 if ok or -1 if mv is out of range.
{
    int16_t mvIndex = mv - _inflexionCentreMv;
    char isNegative = mvIndex < 0;
    int16_t absMv = isNegative ? -mvIndex : mvIndex;
    if (absMv > INFLEXION_CELL_MV_MAX) return 1; //Return invalid
    uint16_t absAs = inflexionAmpSeconds[absMv];
    *pAs = isNegative ? _inflexionCentreAs - absAs : _inflexionCentreAs + absAs;
    return 0;
}

void CurveInit()
{
    _inflexionCentreMv      = EepromReadS16(EEPROM_CURVE_INFLEXION_MV_S16    );
    _inflexionCentrePercent = EepromReadU8 (EEPROM_CURVE_INFLEXION_PERCENT_U8);
    _inflexionCentreAs = _inflexionCentrePercent * 36UL * 280;
}
