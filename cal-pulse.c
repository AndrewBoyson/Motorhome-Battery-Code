#include <stdint.h>
#include <limits.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "eeprom-this.h"
#include "count.h"

static uint16_t _ongoingPositivePulseCount = 0;
static uint16_t _ongoingNegativePulseCount = 0;
static uint16_t _lastEepromPos   = 0;
static uint16_t _lastEepromNeg   = 0;

void     CalPulseIncPosPulses() {        _ongoingPositivePulseCount++;   }
void     CalPulseResPosPulses() {        _ongoingPositivePulseCount = 0; }
void     CalPulseIncNegPulses() {        _ongoingNegativePulseCount++;   }
void     CalPulseResNegPulses() {        _ongoingNegativePulseCount = 0; }


static uint16_t _endOfCyclePositivePulseCount        = 0;
static uint16_t _endOfCycleNegativePulseCount        = 0;
static  int32_t _endOfCycleDifferenceMilliAmpSeconds = 0;

uint16_t CalPulseGetPosPulses    () { return _endOfCyclePositivePulseCount;        }
uint16_t CalPulseGetNegPulses    () { return _endOfCycleNegativePulseCount;        }
 int32_t CalPulseGetDifferenceMas() { return _endOfCycleDifferenceMilliAmpSeconds; }


static int16_t _pulseAdjustMilliAmpSeconds = 0;

int16_t CalPulseGetAdjustMas(         ) { return _pulseAdjustMilliAmpSeconds; }
void    CalPulseSetAdjustMas(int16_t v) {        _pulseAdjustMilliAmpSeconds = v; EepromSaveS16(EEPROM_CAL_PULSE_ADJUST_MAS_S16, v); }

void CalPulseHandleEndOfCycle(uint32_t calculatedMilliAmpSeconds) //Called by cal-charge.c once at the end of a cycle
{
    //Save counts
    _endOfCyclePositivePulseCount = _ongoingPositivePulseCount;
    _endOfCycleNegativePulseCount = _ongoingNegativePulseCount;
    EepromSaveU16(EEPROM_CAL_PULSE_EOC_POS_COUNT_U16, _endOfCyclePositivePulseCount);
    EepromSaveU16(EEPROM_CAL_PULSE_EOC_NEG_COUNT_U16, _endOfCycleNegativePulseCount);
    
    //Save difference
    uint32_t    countedMilliAmpSeconds = CountGetMilliAmpSeconds();
    _endOfCycleDifferenceMilliAmpSeconds = (int32_t)(calculatedMilliAmpSeconds - countedMilliAmpSeconds);
    EepromSaveS16(EEPROM_CAL_PULSE_EOC_DIFFERENCE_MAS_S16, (int16_t)(_endOfCycleDifferenceMilliAmpSeconds >> 16));

    //Calculate new pulse adjust
    /*int32_t pulseCount = _ongoingPositivePulseCount + _ongoingNegativePulseCount;
    if (pulseCount) //Don't divide by zero - undefined behaviour!
    {
        int32_t newPulseAdjustMilliAmpSeconds = _endOfCycleDifferenceMilliAmpSeconds / pulseCount;
        if (newPulseAdjustMilliAmpSeconds > INT_MAX) newPulseAdjustMilliAmpSeconds = INT_MAX;
        if (newPulseAdjustMilliAmpSeconds < INT_MIN) newPulseAdjustMilliAmpSeconds = INT_MIN;
        CalPulseSetAdjustMas((int16_t)newPulseAdjustMilliAmpSeconds);
    }*/
}

void CalPulseInit()
{
    _endOfCyclePositivePulseCount         =          EepromReadU16(EEPROM_CAL_PULSE_EOC_POS_COUNT_U16);
    _endOfCycleNegativePulseCount         =          EepromReadU16(EEPROM_CAL_PULSE_EOC_NEG_COUNT_U16);
    _endOfCycleDifferenceMilliAmpSeconds  = (int32_t)EepromReadS16(EEPROM_CAL_PULSE_EOC_DIFFERENCE_MAS_S16) << 16;
    
    _pulseAdjustMilliAmpSeconds           =          EepromReadS16(EEPROM_CAL_PULSE_ADJUST_MAS_S16);
    
    _ongoingPositivePulseCount            =          EepromReadU16(EEPROM_CAL_PULSE_ONGOING_POS_COUNT_U16);
    _ongoingNegativePulseCount            =          EepromReadU16(EEPROM_CAL_PULSE_ONGOING_NEG_COUNT_U16);
    _lastEepromPos = _ongoingPositivePulseCount;
    _lastEepromNeg = _ongoingNegativePulseCount;
}

void CalPulseMain()
{
    //Save counts in case of reset but with at least a 5 minute gap to give a ten year eeprom life
    static uint32_t _msTimerSavePos = 0;
    static uint32_t _msTimerSaveNeg = 0;
    
    if (_ongoingPositivePulseCount != _lastEepromPos && MsTimerRepetitive(&_msTimerSavePos, 5UL * 60 * 1000))
    {
        EepromSaveU16(EEPROM_CAL_PULSE_ONGOING_POS_COUNT_U16, _ongoingPositivePulseCount);
        _lastEepromPos = _ongoingPositivePulseCount;
    }
    if (_ongoingNegativePulseCount != _lastEepromNeg && MsTimerRepetitive(&_msTimerSaveNeg, 5UL * 60 * 1000))
    {
        EepromSaveU16(EEPROM_CAL_PULSE_ONGOING_NEG_COUNT_U16, _ongoingNegativePulseCount);
        _lastEepromNeg = _ongoingNegativePulseCount;
    }
}