#include <stdint.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "eeprom-this.h"
#include "count.h"

static uint32_t _capacityMilliAmpSeconds   = 0; //280Ah is 280 * 1000 * 3600 == 3C14 DC00. Could hold up to 1193Ah
static uint32_t _milliAmpSeconds           = 0; //
static  int16_t _currentOffsetMa           = 0;

static uint16_t _positivePulses   = 0;
static uint16_t _negativePulses   = 0;

static uint16_t _lastSavedSoc = 0;
static uint16_t _lastSavedPos = 0;
static uint16_t _lastSavedNeg = 0;

void CountInit()
{
    _capacityMilliAmpSeconds = BATTERY_CAPACITY_AH * 3600000;
    _currentOffsetMa         = EepromReadS16(EEPROM_CURRENT_OFFSET_MA_S16);
    
    _lastSavedSoc = EepromReadU16(EEPROM_COUNT_SOC_MAS_U16);
    _lastSavedPos = EepromReadU16(EEPROM_COUNT_POS_PULSES_U16);
    _lastSavedNeg = EepromReadU16(EEPROM_COUNT_NEG_PULSES_U16);
    _milliAmpSeconds         = (uint32_t)_lastSavedSoc << 16;
    _positivePulses          = _lastSavedPos;
    _negativePulses          = _lastSavedNeg;
    
}

int16_t CountGetCurrentOffsetMa()           { return _currentOffsetMa;}
void    CountSetCurrentOffsetMa( int16_t v) {        _currentOffsetMa = v; EepromSaveS16(EEPROM_CURRENT_OFFSET_MA_S16, _currentOffsetMa); } 

uint32_t CountGetAmpSeconds()           { return _milliAmpSeconds     / 1000; }
void     CountSetAmpSeconds(uint32_t v) {        _milliAmpSeconds = v * 1000; }

uint32_t CountGetMilliAmpSeconds()           { return _milliAmpSeconds; }
void     CountSetMilliAmpSeconds(uint32_t v) {        _milliAmpSeconds = v; }
void     CountAddMilliAmpSeconds(uint32_t v)
{
    if (_milliAmpSeconds < (_capacityMilliAmpSeconds - v)) _milliAmpSeconds += v;
    else                                                   _milliAmpSeconds  = _capacityMilliAmpSeconds;
}
void     CountSubMilliAmpSeconds(uint32_t v)
{
    if (_milliAmpSeconds > v) _milliAmpSeconds -= v;
    else                      _milliAmpSeconds  = 0;
}
uint16_t CountGetAmpHours()             { return (uint16_t)(_milliAmpSeconds     / 3600000); }
void     CountSetAmpHours(uint16_t v)   {                   _milliAmpSeconds = v * 3600000 ; }
/*
  0% = -0.5 to   0.4999%
  1% =  0.5 to   1.4999%
 99% = 98.5 to  99.4999%
100% = 99.5 to 100.4999%

so add 0.5 and take whole part
*/
uint8_t  CountGetSocPercent()           { return (uint8_t)((_milliAmpSeconds + _capacityMilliAmpSeconds / 200) / (_capacityMilliAmpSeconds / 100)); }
void     CountSetSocPercent(uint8_t v)  {                   _milliAmpSeconds = v                               * (_capacityMilliAmpSeconds / 100) ; }
void     CountAddSocPercent(uint8_t v)
{
    uint32_t toAdd = (uint32_t)v * _capacityMilliAmpSeconds /  100;
    CountAddMilliAmpSeconds(toAdd);
}
void     CountSubSocPercent(uint8_t v)
{
    uint32_t toAdd = (uint32_t)v * _capacityMilliAmpSeconds /  100;
    CountSubMilliAmpSeconds(toAdd);
}

uint32_t CountGetSoCmAh()             { return _milliAmpSeconds     / 3600; }
void     CountSetSoCmAh(uint32_t v)   {        _milliAmpSeconds = v * 3600; }

uint16_t CountGetPosPulses() { return _positivePulses;     }
void     CountIncPosPulses() {        _positivePulses++;   }
void     CountResPosPulses() {        _positivePulses = 0; }
uint16_t CountGetNegPulses() { return _negativePulses;     }
void     CountIncNegPulses() {        _negativePulses++;   }
void     CountResNegPulses() {        _negativePulses = 0; }

void CountMain()
{
    //Add current offset
    static uint32_t _msTimerAging = 0;
    if (MsTimerRepetitive(&_msTimerAging, 1000))
    {
        if (_currentOffsetMa > 0) CountAddMilliAmpSeconds((uint32_t)( _currentOffsetMa));
        if (_currentOffsetMa < 0) CountSubMilliAmpSeconds((uint32_t)(-_currentOffsetMa));
    }

    //Save counts in case of reset but with at least a 5 minute gap to give a ten year eeprom life
    static uint32_t _msTimerSaveSoc = 0;
    static uint32_t _msTimerSavePos = 0;
    static uint32_t _msTimerSaveNeg = 0;
    uint16_t thisSoc = (uint16_t)(        _milliAmpSeconds >> 16);
    if (thisSoc != _lastSavedSoc && MsTimerRepetitive(&_msTimerSaveSoc, 5UL * 60 * 1000))
    {
        EepromSaveU16(EEPROM_COUNT_SOC_MAS_U16, thisSoc);
        _lastSavedSoc = thisSoc;
    }
    if (_positivePulses != _lastSavedPos && MsTimerRepetitive(&_msTimerSavePos, 5UL * 60 * 1000))
    {
        EepromSaveU16(EEPROM_COUNT_POS_PULSES_U16, _positivePulses);
        _lastSavedPos = _positivePulses;
    }
    if (_negativePulses != _lastSavedNeg && MsTimerRepetitive(&_msTimerSaveNeg, 5UL * 60 * 1000))
    {
        EepromSaveU16(EEPROM_COUNT_NEG_PULSES_U16, _negativePulses);
        _lastSavedNeg = _negativePulses;
    }
}