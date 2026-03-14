#include <stdint.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "eeprom-this.h"
#include "output.h"
#include "count.h"
#include "pulse.h"
#include "voltage.h"

#define MAX_REST_TIMER_MS 10UL * 24 * 3600 * 1000

static uint32_t _msTimerRest = 0;
uint32_t RestGetMsAtRest() { return MsTimerCount - _msTimerRest; }

static uint16_t _voltageSettleTimeMins = 0;

uint16_t RestGetVoltageSettleTimeMins()    { return _voltageSettleTimeMins; }
void     RestSetVoltageSettleTimeMins(uint16_t v) { _voltageSettleTimeMins = v; EepromSaveU16(EEPROM_MANAGE_VOLTAGE_SETTLE_TIME_MINS_U16, v ); }

static char _currentIsStable = 0;
static char _voltageIsStable = 0;
char RestGetCurrentIsStable() { return _currentIsStable; }
char RestGetVoltageIsStable() { return _voltageIsStable; }

void RestInit()
{
    uint16_t restTimeMinutes = EepromReadU16(EEPROM_MANAGE_REST_TIMER_MINUTES_U16);
    uint32_t restTimeMs = (uint32_t)restTimeMinutes << 16;
    if (restTimeMs > MAX_REST_TIMER_MS) restTimeMs = 0; //Eeprom value is likely not initialised
    _msTimerRest = MsTimerCount - restTimeMs;
            
    _voltageSettleTimeMins = EepromReadU16(EEPROM_MANAGE_VOLTAGE_SETTLE_TIME_MINS_U16) ;
}

void RestMain()
{   
    char isAtRest = OutputGetState() == 'N' && PulseGetCurrentMa() > -100;
    if (isAtRest)
    {
        if (MsTimerCount > _msTimerRest + MAX_REST_TIMER_MS) _msTimerRest = MsTimerCount - MAX_REST_TIMER_MS;   //Limit the rest timer to 10 days
        uint16_t restTimeMinutes = (uint16_t)((MsTimerCount - _msTimerRest) >> 16);                             //Approximate minutes using ms * 65536
        if ((restTimeMinutes & 0xF) == 0) EepromSaveU16(EEPROM_MANAGE_REST_TIMER_MINUTES_U16, restTimeMinutes); //Save time about every 16 minutes
        _currentIsStable = restTimeMinutes >= 60;
        _voltageIsStable = restTimeMinutes >= _voltageSettleTimeMins;
    }
    else
    {
        _msTimerRest = MsTimerCount;                                                                           //Set rest time to zero
        EepromSaveU16(EEPROM_MANAGE_REST_TIMER_MINUTES_U16, 0);                                                //Save time - eeprom save checks the current value (no wear) and only actually saves if different
        _currentIsStable = 0;
        _voltageIsStable = 0;                                                                                                
    }
    
    
}