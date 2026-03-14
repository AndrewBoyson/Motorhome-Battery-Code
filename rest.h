#include <stdint.h>

extern void     RestMain(void);
extern void     RestInit(void);
extern uint32_t RestGetMsAtRest(void);
extern uint16_t RestGetVoltageSettleTimeMins(void);
extern void     RestSetVoltageSettleTimeMins(uint16_t);
extern char     RestGetCurrentIsStable(void);
extern char     RestGetVoltageIsStable(void);
