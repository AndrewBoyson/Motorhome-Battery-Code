
extern uint16_t CalPulseGetPosPulses(void);
extern void     CalPulseIncPosPulses(void);
extern void     CalPulseResPosPulses(void);
extern uint16_t CalPulseGetNegPulses(void);
extern void     CalPulseIncNegPulses(void);
extern void     CalPulseResNegPulses(void);

extern int32_t  CalPulseGetDifferenceMas (void);
extern int16_t  CalPulseGetAdjustMas(void); extern void CalPulseSetAdjustMas(int16_t);

extern void CalPulseHandleEndOfCycle(uint32_t calculatedMilliAmpSeconds); //Called by cal-charge.c once at the end of a cycle

extern void CalPulseInit(void);
extern void CalPulseMain(void);