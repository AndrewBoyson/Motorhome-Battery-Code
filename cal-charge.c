#include <stdint.h>

#include "../eeprom.h"

#include "count.h"
#include "voltage.h"
#include "rest.h"
#include "eeprom-this.h"
#include "curve.h"
#include "cal-pulse.h"

static char _isActive = 0;
static char _oneShot  = 1; //Init as true so that don't reactivate on reset

char    CalChargeGetIsActive      (         ) { return _isActive; }

void CalChargeInit()
{
}
void CalChargeMain()
{
    _isActive = 0;
    
    int16_t batteryMv = VoltageGetAsMv();
    if (!batteryMv) return;
    
    char stable = RestGetVoltageIsStable();
    if (!stable   ) { _oneShot = 0; return; }
    
    uint32_t calculatedAs = 0;
    char outOfRange = CurveGetInflexionAsFromMv(batteryMv / 4, &calculatedAs);
    if (outOfRange) { _oneShot = 0; return; }
    
    _isActive = 1;
    
    uint32_t calculatedMilliAmpSeconds = calculatedAs * 1000;
    
    if (!_oneShot) CalPulseHandleEndOfCycle(calculatedMilliAmpSeconds); //Only do this once per cycle
    _oneShot = 1;
    
    CountSetMilliAmpSeconds(calculatedMilliAmpSeconds);
    CalPulseResPosPulses();
    CalPulseResNegPulses();
}
