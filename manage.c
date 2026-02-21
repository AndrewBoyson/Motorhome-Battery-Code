#include <stdint.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "eeprom-this.h"
#include "output.h"
#include "count.h"
#include "pulse.h"
#include "voltage.h"

#define INFLEXION_CELL_MV 3299
#define INFLEXION_AS 57L*36*280
#define INFLEXION_AS_PER_MV 1008
#define INFLEXION_MV_WIDTH 10

static uint32_t _msTimerRest = 0;
uint32_t ManageGetMsAtRest() { return MsTimerCount - _msTimerRest; }

static uint16_t _voltageSettleTimeMins = 0;
static uint32_t _voltageSettleTimeMs   = 0;

uint16_t ManageGetVoltageSettleTimeMins() { return _voltageSettleTimeMins; }
void     ManageSetVoltageSettleTimeMins(uint16_t v) { _voltageSettleTimeMins = v; _voltageSettleTimeMs = v * 60 * 1000; EepromSaveU16(EEPROM_MANAGE_VOLTAGE_SETTLE_TIME_MINS_U16, v ); }

void ManageInit()
{
    _voltageSettleTimeMins = EepromReadU16(EEPROM_MANAGE_VOLTAGE_SETTLE_TIME_MINS_U16) ;
    _voltageSettleTimeMs   = _voltageSettleTimeMins * 60 * 1000 ;
}

static void zeroCurrent(char run)
{
    static uint32_t _msTimerRepeat = 0;
    if (!run) _msTimerRepeat = MsTimerCount;
    if (!MsTimerRepetitive(&_msTimerRepeat, 1UL*60*60*1000)) return;
    
    int32_t currentMa = PulseGetCurrentMa();       //Say  12mA
    int16_t  offsetMa = CountGetCurrentOffsetMa(); //Say -11mA
    if (currentMa + offsetMa > 0) CountSetCurrentOffsetMa(offsetMa - 1); //Set new current offset to be -12mA
    if (currentMa + offsetMa < 0) CountSetCurrentOffsetMa(offsetMa + 1);
}
static void calibrateChargeFromVoltage(char run)
{
    static uint32_t _msTimerRepeat = 0;
    if (!run) _msTimerRepeat = MsTimerCount;
    if (!MsTimerRepetitive(&_msTimerRepeat, 1UL*10*60*1000)) return;
    
    int32_t mvAwayFromInflexion = VoltageGetAsMv() / 4 - INFLEXION_CELL_MV;
    if (mvAwayFromInflexion >  INFLEXION_MV_WIDTH) return; // No valid rule for this state but ultimately need to add fully charged slope
    if (mvAwayFromInflexion < -INFLEXION_MV_WIDTH) return; // No valid rule for this state but ultimately need to add nearly discharged slope
    int32_t asAwayFromInflexion = mvAwayFromInflexion * INFLEXION_AS_PER_MV;
    CountSetAmpSeconds((uint32_t)(INFLEXION_AS + asAwayFromInflexion));
}

void ManageMain()
{   
    char isAtRest = OutputGetState() == 'N' && PulseGetCurrentMa() > -100;
    
    if (!isAtRest) _msTimerRest = MsTimerCount;

    zeroCurrent                (MsTimerRelative(_msTimerRest,       1UL*60*60*1000));
    calibrateChargeFromVoltage (MsTimerRelative(_msTimerRest, _voltageSettleTimeMs));
}