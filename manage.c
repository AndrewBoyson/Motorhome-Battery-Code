#include <stdint.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "eeprom-this.h"
#include "output.h"
#include "count.h"
#include "pulse.h"
#include "voltage.h"

#define INFLEXION_CELL_MV 3299
#define INFLEXION_AS 57UL*36*280

#define CAPACITY_AH 280

static const uint16_t inflexionAmpSeconds[] =
{
     0U * CAPACITY_AH * 36 / 10, // 0
     1U * CAPACITY_AH * 36 / 10, // 1
     2U * CAPACITY_AH * 36 / 10, // 2
     3U * CAPACITY_AH * 36 / 10, // 3
     4U * CAPACITY_AH * 36 / 10, // 4
     5U * CAPACITY_AH * 36 / 10, // 5
     6U * CAPACITY_AH * 36 / 10, // 6
     7U * CAPACITY_AH * 36 / 10, // 7
     8U * CAPACITY_AH * 36 / 10, // 8
     9U * CAPACITY_AH * 36 / 10, // 9
    10U * CAPACITY_AH * 36 / 10, //10
    12U * CAPACITY_AH * 36 / 10, //11
    14U * CAPACITY_AH * 36 / 10, //12
    16U * CAPACITY_AH * 36 / 10, //13
    18U * CAPACITY_AH * 36 / 10, //14
    20U * CAPACITY_AH * 36 / 10  //15
};
int8_t ManageGetValidInflectionMv() { return sizeof(inflexionAmpSeconds); }
static char inflexionAsFromMv(int16_t batteryMv, uint32_t* pAs) //returns 0 if ok or -1 if mv is out of range.
{
    int16_t mv = batteryMv / 4 - INFLEXION_CELL_MV;
    char isNegative = mv < 0;
    int16_t absMv = isNegative ? -mv : mv;
    if (absMv > sizeof(inflexionAmpSeconds) - 1) return 1;
    uint16_t absAs = inflexionAmpSeconds[absMv];
    *pAs = isNegative ? INFLEXION_AS - absAs : INFLEXION_AS + absAs;
    return 0;
}

static uint32_t _msTimerRest = 0;
uint32_t ManageGetMsAtRest() { return MsTimerCount - _msTimerRest; }

static uint16_t _voltageSettleTimeMins = 0;
static uint32_t _voltageSettleTimeMs   = 0;

uint16_t ManageGetVoltageSettleTimeMins() { return _voltageSettleTimeMins; }
void     ManageSetVoltageSettleTimeMins(uint16_t v) { _voltageSettleTimeMins = v; _voltageSettleTimeMs = v * 60UL * 1000; EepromSaveU16(EEPROM_MANAGE_VOLTAGE_SETTLE_TIME_MINS_U16, v ); }

void ManageInit()
{
    _voltageSettleTimeMins = EepromReadU16(EEPROM_MANAGE_VOLTAGE_SETTLE_TIME_MINS_U16) ;
    _voltageSettleTimeMs   = _voltageSettleTimeMins * 60UL * 1000 ;
}

#define ZERO_CURRENT_REPEAT_TIME 1UL*60*60*1000
static void zeroCurrent(char run)
{
    static uint32_t _msTimerRepeat = 0;
    if (!run)
    {
        _msTimerRepeat = MsTimerCount - ZERO_CURRENT_REPEAT_TIME; //This allows it to start immediately when next run
        return;
    }
    if (!MsTimerRepetitive(&_msTimerRepeat,   ZERO_CURRENT_REPEAT_TIME)) return;
    
    int32_t currentMa = PulseGetCurrentMa();       //Say  12mA
    int16_t  offsetMa = CountGetCurrentOffsetMa(); //Say -11mA
    if (currentMa + offsetMa > 0) CountSetCurrentOffsetMa(offsetMa - 1); //Set new current offset to be -12mA
    if (currentMa + offsetMa < 0) CountSetCurrentOffsetMa(offsetMa + 1);
}

#define CALIBRATE_REPEAT_TIME 1000
static void calibrateChargeFromVoltage(char run)
{
    static uint32_t _msTimerRepeat = 0;
    if (!run)
    {
        _msTimerRepeat = MsTimerCount - CALIBRATE_REPEAT_TIME; //This allows it to start immediately when next run
        return;
    }
    if (!MsTimerRepetitive(&_msTimerRepeat,   CALIBRATE_REPEAT_TIME)) return;
    
    uint32_t calculatedAs = 0;
    char outOfRange = inflexionAsFromMv(VoltageGetAsMv(), &calculatedAs);
    uint32_t calculatedMilliAmpSeconds = calculatedAs * 1000;
    if (!outOfRange)
    {
        uint32_t countMilliAmpSeconds = CountGetMilliAmpSeconds();
        if (calculatedMilliAmpSeconds > countMilliAmpSeconds)
        {
            uint32_t  differenceMilliAmpSeconds = calculatedMilliAmpSeconds - countMilliAmpSeconds;
            if  (differenceMilliAmpSeconds  > 1000) CountAddMilliAmpSeconds(1000);
            else                                    CountAddMilliAmpSeconds(differenceMilliAmpSeconds);
        }
        else
        {
            uint32_t  differenceMilliAmpSeconds = countMilliAmpSeconds - calculatedMilliAmpSeconds;
            if  (differenceMilliAmpSeconds  > 1000) CountSubMilliAmpSeconds(1000);
            else                                    CountSubMilliAmpSeconds(differenceMilliAmpSeconds);
        }
    }
    //Probably add highAsFromMv and a lowAsFromMv routines some day
}
void ManageMain()
{   
    char isAtRest = OutputGetState() == 'N' && PulseGetCurrentMa() > -100;
    
    if (!isAtRest) _msTimerRest = MsTimerCount;

    zeroCurrent                (MsTimerRelative(_msTimerRest,       1UL*60*60*1000));
    calibrateChargeFromVoltage (MsTimerRelative(_msTimerRest, _voltageSettleTimeMs));
}