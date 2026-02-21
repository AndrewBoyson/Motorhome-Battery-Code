#include <stdint.h>

extern void     ManageMain(void);
extern void     ManageInit(void);
extern uint32_t ManageGetMsAtRest(void);
extern uint16_t ManageGetVoltageSettleTimeMins(void);
extern void     ManageSetVoltageSettleTimeMins(uint16_t);
