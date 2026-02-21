#include <stdint.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "eeprom-this.h"
#include "count.h"

#define BATTERY_CAPACITY_AH 280

static uint32_t _capacityMilliAmpSeconds   = 0; //280Ah is 280 * 1000 * 3600 == 3C14 DC00. Could hold up to 1193Ah
static uint32_t _milliAmpSeconds           = 0; //
static uint8_t  _lastSavedSoc0to255        = 0; //Used to save count across resets. Updated in CountMain
static  int16_t _currentOffsetMa            = 0;

void CountInit()
{
    _lastSavedSoc0to255      = EepromReadU8(EEPROM_COUNT_SOC_U8);
    _capacityMilliAmpSeconds = BATTERY_CAPACITY_AH * 3600000;
    _milliAmpSeconds         = _capacityMilliAmpSeconds / 256 * _lastSavedSoc0to255;
    _currentOffsetMa         = EepromReadS16(EEPROM_CURRENT_OFFSET_MA_S16);
}

int16_t CountGetCurrentOffsetMa()           { return _currentOffsetMa;}
void    CountSetCurrentOffsetMa( int16_t v) {        _currentOffsetMa = v; EepromSaveS16(EEPROM_CURRENT_OFFSET_MA_S16, _currentOffsetMa); } 

uint32_t CountGetAmpSeconds()           { return _milliAmpSeconds     / 1000; }
void     CountSetAmpSeconds(uint32_t v) {        _milliAmpSeconds = v * 1000; }
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
uint16_t CountGetAmpHours()             { return (uint16_t)(_milliAmpSeconds     / 3600000ULL); }
void     CountSetAmpHours(uint16_t v)   {                   _milliAmpSeconds = v * 3600000ULL ; }
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

uint8_t  CountGetSoc0to255()          { return (uint8_t)((_milliAmpSeconds + _capacityMilliAmpSeconds / 512) / (_capacityMilliAmpSeconds / 256)); }
void     CountSetSoc0to255(uint8_t v) {                   _milliAmpSeconds = v                               * (_capacityMilliAmpSeconds / 256) ; }

uint32_t CountGetSoCmAh()             { return _milliAmpSeconds     / 3600; }
void     CountSetSoCmAh(uint32_t v)   {        _milliAmpSeconds = v * 3600; }


void CountMain()
{
    //Add current offset
    static uint32_t _msTimerAging = 0;
    if (MsTimerRepetitive(&_msTimerAging, 1000))
    {
        if (_currentOffsetMa > 0) CountAddMilliAmpSeconds((uint32_t)( _currentOffsetMa));
        if (_currentOffsetMa < 0) CountSubMilliAmpSeconds((uint32_t)(-_currentOffsetMa));
    }

    //Calculate Soc (0-255) and save if necessary
    uint8_t thisSoc0to255 = CountGetSoc0to255();
    if (thisSoc0to255 != _lastSavedSoc0to255)
    {
        _lastSavedSoc0to255 = thisSoc0to255;
        EepromSaveU8(EEPROM_COUNT_SOC_U8, _lastSavedSoc0to255); //This limits the number of writes to eeprom while still being good enough
    }
}