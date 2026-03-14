#include <stdint.h>

#include "../mstimer.h"
#include "rest.h"
#include "pulse.h"
#include "count.h"

#define ZERO_CURRENT_REPEAT_TIME 1UL*60*60*1000
void CalCurrentInit()
{

}
void CalCurrentMain()
{
    char stable = RestGetCurrentIsStable();
    static uint32_t _msTimerRepeat = 0;
    if (!stable)
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