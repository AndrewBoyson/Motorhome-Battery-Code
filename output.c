
#include <xc.h>
#include <stdint.h>

#include "../mstimer.h"
#include "../eeprom.h"

#include "output.h"
#include "count.h"
#include "temperature.h"
#include "eeprom-this.h"
#include "voltage.h"
#include "manage.h"

#define CHARGE     LATBbits.LB5
#define SUPPLY_OFF LATCbits.LC7

#define MIN_CHARGE_TEMPERATURE_8_BIT ( 5 << 8)
#define MAX_CHARGE_MV    (3500 * 4) //100%
#define MIN_DISCHARGE_MV (2500 * 4) //0%

#define STATE_NEUTRAL   0
#define STATE_CHARGE    1
#define STATE_DISCHARGE 2

static char    _state            = STATE_NEUTRAL;
static char    _allowed          = 0;
static char    _chargeEnabled    = 0;
static char    _dischargeEnabled = 0;
static char    _targetMode       = 0;
static uint8_t _targetSoc        = 0;
static int16_t _targetMv         = 0;
static int8_t  _reboundMv        = 0;

char OutputGetState()
{
    switch (_state)
    {
        case STATE_NEUTRAL:   return 'N';
        case STATE_CHARGE:    return _allowed ? 'C' : 'c';
        case STATE_DISCHARGE: return _allowed ? 'D' : 'd';
        default:              return '?';
    }
}
static void setState(char v) {_state = v; EepromSaveChar(EEPROM_OUTPUT_STATE_CHAR, _state); }

static void saveEnables()
{
    uint8_t byte = 0;
    if (_chargeEnabled   ) byte |= 2;
    if (_dischargeEnabled) byte |= 1;
    EepromSaveU8(EEPROM_OUTPUT_ENABLES_U8, byte);
}

char    OutputGetChargeEnabled   () { return _chargeEnabled;    } void OutputSetChargeEnabled   (char    v) { _chargeEnabled    = v; saveEnables(); }
char    OutputGetDischargeEnabled() { return _dischargeEnabled; } void OutputSetDischargeEnabled(char    v) { _dischargeEnabled = v; saveEnables(); }
char    OutputGetTargetMode      () { return _targetMode;       } void OutputSetTargetMode      (char    v) { _targetMode       = v; EepromSaveChar(EEPROM_OUTPUT_TARGET_MODE_CHAR, _targetMode); }
uint8_t OutputGetTargetSoc       () { return _targetSoc;        } void OutputSetTargetSoc       (uint8_t v) { _targetSoc        = v; EepromSaveU8  (EEPROM_OUTPUT_TARGET_SOC_U8   , _targetSoc ); } 
int16_t OutputGetTargetMv        () { return _targetMv;         } void OutputSetTargetMv        (int16_t v) { _targetMv         = v; EepromSaveS16 (EEPROM_OUTPUT_TARGET_MV_S16   , _targetMv  ); } 
int8_t  OutputGetReboundMv       () { return _reboundMv;        } void OutputSetReboundMv       (int8_t  v) { _reboundMv        = v; EepromSaveS8  (EEPROM_OUTPUT_REBOUND_MV_S8   , _reboundMv ); } 

void OutputInit()
{
    TRISB5 = 0;	//RB5 (pin 26) output  CHARGE
    TRISC7 = 0;	//RC7 (pin 18) output  SUPPLY_OFF
    
    CHARGE    = 0;
    SUPPLY_OFF = 0;
    
    _state       = EepromReadChar(EEPROM_OUTPUT_STATE_CHAR);
    if (_state != STATE_NEUTRAL && _state != STATE_CHARGE && _state != STATE_DISCHARGE) _state = STATE_NEUTRAL;
    uint8_t byte = EepromReadU8  (EEPROM_OUTPUT_ENABLES_U8);
    _chargeEnabled    = byte & 2;
    _dischargeEnabled = byte & 1;
    _targetMode = EepromReadChar(EEPROM_OUTPUT_TARGET_MODE_CHAR);
    _targetSoc  = EepromReadU8  (EEPROM_OUTPUT_TARGET_SOC_U8);
    _targetMv   = EepromReadS16 (EEPROM_OUTPUT_TARGET_MV_S16);
    _reboundMv  = EepromReadS8  (EEPROM_OUTPUT_REBOUND_MV_S8);
}

void OutputMain()
{
    int16_t actualBatMv = VoltageGetAsMv();
    if (!actualBatMv) //Wait until the battery voltage is valid
    {
        CHARGE     = 0;
        SUPPLY_OFF = 0;
        return;
    }
    
    if (_targetMode == OUTPUT_TARGET_MODE_SOC)
    {
        uint32_t            socAmpSeconds = CountGetAmpSeconds();
        uint32_t         targetAmpSeconds = (uint32_t)OutputGetTargetSoc() * 280 * 36;          //50.000
        uint32_t    chargeStartAmpSeconds = targetAmpSeconds - (uint32_t)4999 * 28 * 36 / 1000; //49.501 = 50 - 0.499% 
        uint32_t dischargeStartAmpSeconds = targetAmpSeconds + (uint32_t)4999 * 28 * 36 / 1000; //50.499 = 50 + 0.499%

        switch (_state)
        {
            case STATE_NEUTRAL:
                if (socAmpSeconds >= dischargeStartAmpSeconds) setState(STATE_DISCHARGE); //Drifts up to 50.499% but in practice only here if the target is changed
                if (socAmpSeconds <=    chargeStartAmpSeconds) setState(STATE_CHARGE   ); //Drifts down to 49.501%
                break;
            case STATE_CHARGE:
                if (socAmpSeconds >=         targetAmpSeconds) setState(STATE_NEUTRAL  ); //Charges to 50.000%
                break;
            case STATE_DISCHARGE:
                if (socAmpSeconds <=         targetAmpSeconds) setState(STATE_NEUTRAL  ); //Discharges to 50.000%
                break;
        }
    }
    else if (_targetMode == OUTPUT_TARGET_MODE_VOLTAGE)
    {
        int16_t            targetBatMv = _targetMv * 4;                    //3300
        int16_t validVoltageSlopeBatMv = ManageGetValidInflectionMv() * 4; //This is the width over which the capacity can be calibrated in manage.c == 15 * 4 mV
        int16_t           reboundbatMv = _reboundMv * 4;                   //This must be smaller than VALID_VOLTAGE_SLOPE_MV or the state won't stay in neutral

        switch (_state)
        {
            case STATE_NEUTRAL:
                if (actualBatMv >= targetBatMv + validVoltageSlopeBatMv) setState(STATE_DISCHARGE); //Drifts up to 3315 but in practice only here if the target is changed
                if (actualBatMv <= targetBatMv - validVoltageSlopeBatMv) setState(STATE_CHARGE   ); //Drifts down to 3285
                break;
            case STATE_CHARGE:
                if (actualBatMv >= targetBatMv + reboundbatMv          ) setState(STATE_NEUTRAL  );   //Charges to 3310
                break;
            case STATE_DISCHARGE:
                if (actualBatMv <= targetBatMv - reboundbatMv          ) setState(STATE_NEUTRAL  );   //Discharges to 3290
                break;
        }
        
    }
    else
    {
        setState(STATE_NEUTRAL);
    }
    
    switch (_state)
    {
        case STATE_NEUTRAL:
            CHARGE     = 0;
            SUPPLY_OFF = 0;
            break;
        case STATE_CHARGE:
            _allowed = (TemperatureGetAs8bfdp() >= MIN_CHARGE_TEMPERATURE_8_BIT) &&
                       (actualBatMv < MAX_CHARGE_MV) &&
                       _chargeEnabled;
            CHARGE     = _allowed;
            SUPPLY_OFF = 0;
            break;
        case STATE_DISCHARGE:
            _allowed = (actualBatMv > MIN_DISCHARGE_MV) &&
                       _dischargeEnabled;
            CHARGE     = 0;
            SUPPLY_OFF = _allowed;
            break;
    }
}