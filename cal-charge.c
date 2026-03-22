#include <stdint.h>
#include <limits.h>

#include "../eeprom.h"

#include "count.h"
#include "voltage.h"
#include "rest.h"
#include "eeprom-this.h"
#include "curve.h"

static int32_t _differenceMilliAmpSeconds = 0;
static int16_t _pulseAdjustMilliAmpSeconds = 0;
static char    _isActive = 0;

int32_t CalChargeGetDifferenceMas (         ) { return _differenceMilliAmpSeconds; }
int16_t CalChargeGetPulseAdjustMas(         ) { return _pulseAdjustMilliAmpSeconds; }
void    CalChargeSetPulseAdjustMas(int16_t v) {        _pulseAdjustMilliAmpSeconds = v; EepromSaveS16(EEPROM_CAL_PULSE_ADJUST_MAS_S16, v); }
char    CalChargeGetIsActive      (         ) { return _isActive; }

void CalChargeInit()
{
     _differenceMilliAmpSeconds = (int32_t)EepromReadS16(EEPROM_CAL_DIFFERENCE_MAS_S16) << 16;
    _pulseAdjustMilliAmpSeconds =          EepromReadS16(EEPROM_CAL_PULSE_ADJUST_MAS_S16);
}
void CalChargeMain()
{
    static char _oneShot = 1; //Init as true so that don't reactivate on reset
    _isActive = 0;
    
    char stable = RestGetVoltageIsStable();
    
    if (!stable   ) { _oneShot = 0; return; }
    
    int16_t batteryMv = VoltageGetAsMv();
    if (!batteryMv) { _oneShot = 0; return; }
    
    uint32_t calculatedAs = 0;
    char outOfRange = CurveGetInflexionAsFromMv(batteryMv / 4, &calculatedAs);
    if (outOfRange) { _oneShot = 0; return; }
    
    _isActive = 1;
    
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
