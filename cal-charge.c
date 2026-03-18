#include <stdint.h>
#include <limits.h>

#include "../eeprom.h"

#include "count.h"
#include "voltage.h"
#include "rest.h"
#include "eeprom-this.h"

#define INFLEXION_CELL_MV 3299
#define INFLEXION_AS 57UL*36*280

#define INFLEXION_CELL_MV_MAX 15

static int32_t  _differenceMilliAmpSeconds = 0;
static int16_t _pulseAdjustMilliAmpSeconds = 0;

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
int8_t CalChargeGetValidInflectionMv() { return INFLEXION_CELL_MV_MAX; }
static char inflexionAsFromMv(int16_t batteryMv, uint32_t* pAs) //returns 0 if ok or -1 if mv is out of range.
{
    int16_t mv = batteryMv / 4 - INFLEXION_CELL_MV;
    char isNegative = mv < 0;
    int16_t absMv = isNegative ? -mv : mv;
    if (absMv > INFLEXION_CELL_MV_MAX) return 1; //Return invalid
    uint16_t absAs = inflexionAmpSeconds[absMv];
    *pAs = isNegative ? INFLEXION_AS - absAs : INFLEXION_AS + absAs;
    return 0;
}

int32_t  CalChargeGetDifferenceMas(         ) {return  _differenceMilliAmpSeconds; }
int16_t CalChargeGetPulseAdjustMas(         ) {return _pulseAdjustMilliAmpSeconds; }
void    CalChargeSetPulseAdjustMas(int16_t v) {       _pulseAdjustMilliAmpSeconds = v; EepromSaveS16(EEPROM_CAL_PULSE_ADJUST_MAS_S16, v); }

void CalChargeInit()
{
     _differenceMilliAmpSeconds = (int32_t)EepromReadS16(EEPROM_CAL_DIFFERENCE_MAS_S16) << 16;
    _pulseAdjustMilliAmpSeconds =          EepromReadS16(EEPROM_CAL_PULSE_ADJUST_MAS_S16);
}
void CalChargeMain()
{
    static char _oneShot = 0;
    
    char stable = RestGetVoltageIsStable();
    
    if (!stable   ) { _oneShot = 0; return; }
    
    int16_t batteryMv = VoltageGetAsMv();
    if (!batteryMv) { _oneShot = 0; return; }
    
    uint32_t calculatedAs = 0;
    char outOfRange = inflexionAsFromMv(VoltageGetAsMv(), &calculatedAs);
    if (outOfRange)  { _oneShot = 0; return; }
    
    uint32_t calculatedMilliAmpSeconds = calculatedAs * 1000;
    uint32_t    countedMilliAmpSeconds = CountGetMilliAmpSeconds();
    
    if (!_oneShot)
    {
        //Save difference
        _differenceMilliAmpSeconds = (int32_t)(calculatedMilliAmpSeconds - countedMilliAmpSeconds);
        EepromSaveS16(EEPROM_CAL_DIFFERENCE_MAS_S16, (int16_t)(_differenceMilliAmpSeconds >> 16));
        
        //Calculate new pulse adjust
        int32_t pulseCount = CountGetPosPulses() + CountGetNegPulses();
        if (pulseCount) //Don't divide by zero - undefined behaviour!
        {
            int32_t newPulseAdjustMilliAmpSeconds = _differenceMilliAmpSeconds / pulseCount;
            if (newPulseAdjustMilliAmpSeconds > INT_MAX) newPulseAdjustMilliAmpSeconds = INT_MAX;
            if (newPulseAdjustMilliAmpSeconds < INT_MIN) newPulseAdjustMilliAmpSeconds = INT_MIN;
            CalChargeSetPulseAdjustMas((int16_t)newPulseAdjustMilliAmpSeconds);
        }
        
        //Only do this once per cycle
        _oneShot = 1;
    }
    
    CountSetMilliAmpSeconds(calculatedMilliAmpSeconds);
    CountResPosPulses();
    CountResNegPulses();
}
