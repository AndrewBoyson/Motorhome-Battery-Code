#include <stdint.h>

extern int32_t  CalChargeGetDifferenceMas (void);
extern int16_t  CalChargeGetPulseAdjustMas(void); extern void CalChargeSetPulseAdjustMas(int16_t);
extern char     CalChargeGetIsActive      (void);

extern void     CalChargeInit(void);
extern void     CalChargeMain(void);
