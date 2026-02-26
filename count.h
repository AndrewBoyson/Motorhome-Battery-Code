#include <stdint.h>

extern  int16_t CountGetCurrentOffsetMa(void);
extern  void    CountSetCurrentOffsetMa(int16_t v);

extern uint32_t CountGetMilliAmpSeconds(void);
extern void     CountSetMilliAmpSeconds(uint32_t v);
extern void     CountAddMilliAmpSeconds(uint32_t v);
extern void     CountSubMilliAmpSeconds(uint32_t v);

extern uint32_t CountGetAmpSeconds(void);
extern void     CountSetAmpSeconds(uint32_t v);

extern uint16_t CountGetAmpHours(void);
extern void     CountSetAmpHours(uint16_t v);

extern uint8_t  CountGetSocPercent(void);
extern void     CountSetSocPercent(uint8_t v);
extern void     CountAddSocPercent(uint8_t v);
extern void     CountSubSocPercent(uint8_t v);

extern uint8_t  CountGetSoc0to255(void);
extern void     CountSetSoc0to255(uint8_t v);

extern uint32_t CountGetSoCmAh(void);
extern void     CountSetSoCmAh(uint32_t v);


extern void CountInit(void);
extern void CountMain(void);