#include "../mstimer.h"
#include "../msticker.h"
#include "../can.h"
#include "../canids.h"

#include "count.h"
#include "pulse.h"
#include "output.h"
#include "temperature.h"
#include "heater.h"
#include "voltage.h"
#include "rest.h"
#include "cal-charge.h"
#include "cal-current.h"
#include "curve.h"

#define BASE_MS 1000

static void receive(uint16_t id, uint8_t length, void* pData)
{
    switch(id)
    {
        case CAN_ID_SERVER  + CAN_ID_TIME:                    MsTickerRegulate              (*(uint32_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_COUNTED_AMP_SECONDS:     CountSetAmpSeconds            (*(uint32_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_MANAGE_PULSE_ADJUST_MAS: CalChargeSetPulseAdjustMas    (*( int16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_OUTPUT_TARGET_SOC:       OutputSetTargetSoc            (*( uint8_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_CHARGE_ENABLED:          OutputSetChargeEnabled        (*(    char*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_DISCHARGE_ENABLED:       OutputSetDischargeEnabled     (*(    char*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_HEATER_TARGET:           HeaterSetTargetTenths         (*( int16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_CURRENT_OFFSET_MA:       CountSetCurrentOffsetMa       (*( int16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_HEATER_PROPORTIONAL:     HeaterSetKp8bfdp              (*(uint16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_HEATER_INTEGRAL:         HeaterSetKi8bfdp              (*(uint16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_OUTPUT_TARGET_MODE:      OutputSetTargetMode           (*(    char*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_CURVE_INFLEXION_MV:      CurveSetInflexionCentreMv     (*( int16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_CURVE_INFLEXION_PERCENT: CurveSetInflexionCentrePercent(*( uint8_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_CURRENT_SETTLE_MINS:     RestSetCurrentSettleTimeMins  (*(uint16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_VOLTAGE_SETTLE_MINS:     RestSetVoltageSettleTimeMins  (*(uint16_t*)pData); break;
        case CAN_ID_BATTERY + CAN_ID_VOLTAGE_REBOUND_MV:      OutputSetReboundMv            (*(  int8_t*)pData); break;
    }
}

void CanThisInit(void)
{
    CanReceive = &receive;
}
void CanThisMain(void)
{
    { uint32_t value = CountGetAmpSeconds            (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_COUNTED_AMP_SECONDS    , sizeof(value), &value); }
    {  int32_t value = CalChargeGetDifferenceMas     (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_MANAGE_DIFFERENCE_MAS  , sizeof(value), &value); }
    {  int16_t value = CalChargeGetPulseAdjustMas    (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_MANAGE_PULSE_ADJUST_MAS, sizeof(value), &value); }
    { uint16_t value = CountGetPosPulses             (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_COUNT_POS_PULSES       , sizeof(value), &value); }
    { uint16_t value = CountGetNegPulses             (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_COUNT_NEG_PULSES       , sizeof(value), &value); }
    {  int32_t value = PulseGetCurrentMa             (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_MA                     , sizeof(value), &value); }
    {     char value = CalChargeGetIsActive          (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CAL_CHARGE_IS_ACTIVE   , sizeof(value), &value); }
    {     char value = CalCurrentGetIsActive         (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CAL_CURRENT_IS_ACTIVE  , sizeof(value), &value); }
    {     char value = RestGetIsAtRest               (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_IS_AT_REST             , sizeof(value), &value); }
    
    {  uint8_t value = OutputGetTargetSoc            (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_OUTPUT_TARGET_SOC      , sizeof(value), &value); }
    {     char value = OutputGetState                (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_OUTPUT_STATE           , sizeof(value), &value); }
    {     char value = OutputGetChargeEnabled        (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CHARGE_ENABLED         , sizeof(value), &value); }
    {     char value = OutputGetDischargeEnabled     (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_DISCHARGE_ENABLED      , sizeof(value), &value); }
    
    {  int16_t value = TemperatureGetAs8bfdp         (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_TEMPERATURE_8BFDP      , sizeof(value), &value); }
    {  int16_t value = HeaterGetTargetTenths         (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_HEATER_TARGET          , sizeof(value), &value); }
    {  uint8_t value = HeaterGetOutputFixed          (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_HEATER_OUTPUT          , sizeof(value), &value); }
    { uint16_t value = HeaterGetKp8bfdp              (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_HEATER_PROPORTIONAL    , sizeof(value), &value); }
    { uint16_t value = HeaterGetKi8bfdp              (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_HEATER_INTEGRAL        , sizeof(value), &value); }
    
    {  int16_t value = VoltageGetAsMv                (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_VOLTAGE                , sizeof(value), &value); }
    {  int16_t value = CountGetCurrentOffsetMa       (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CURRENT_OFFSET_MA      , sizeof(value), &value); }
    
    {     char value = OutputGetTargetMode           (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_OUTPUT_TARGET_MODE     , sizeof(value), &value); }
    {  int16_t value = CurveGetInflexionCentreMv     (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CURVE_INFLEXION_MV     , sizeof(value), &value); }
    {  uint8_t value = CurveGetInflexionCentrePercent(); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CURVE_INFLEXION_PERCENT, sizeof(value), &value); }
    { uint32_t value = RestGetMsAtRest               (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_MS_AT_REST             , sizeof(value), &value); }
    { uint16_t value = RestGetCurrentSettleTimeMins  (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_CURRENT_SETTLE_MINS    , sizeof(value), &value); }
    { uint16_t value = RestGetVoltageSettleTimeMins  (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_VOLTAGE_SETTLE_MINS    , sizeof(value), &value); }
    {   int8_t value = OutputGetReboundMv            (); static struct CanTransmitState state; CanTransmitOnChange(&state, CAN_ID_BATTERY, CAN_ID_VOLTAGE_REBOUND_MV     , sizeof(value), &value); }
    
}
