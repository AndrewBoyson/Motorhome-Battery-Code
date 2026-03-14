#include <stdint.h>

extern int8_t   CalChargeGetValidInflectionMv(void);
extern int32_t  CalChargeGetDifferenceMas(void);
extern int16_t  CalChargeGetPulseAdjustMas(void);
extern void     CalChargeSetPulseAdjustMas(int16_t);
extern void     CalChargeInit(void);
extern void     CalChargeMain(void);
