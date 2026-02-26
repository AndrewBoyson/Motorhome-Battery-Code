extern char OutputGetState(void);

extern char    OutputGetChargeEnabled   (void); extern void OutputSetChargeEnabled   (char   );
extern char    OutputGetDischargeEnabled(void); extern void OutputSetDischargeEnabled(char   );
extern uint8_t OutputGetTargetSoc       (void); extern void OutputSetTargetSoc       (uint8_t);
extern int16_t OutputGetTargetMv        (void); extern void OutputSetTargetMv        (int16_t);
extern int8_t  OutputGetReboundMv       (void); extern void OutputSetReboundMv       (int8_t );
extern char    OutputGetTargetMode      (void); extern void OutputSetTargetMode      (char   );

extern void OutputInit(void);
extern void OutputMain(void);