#include <stdint.h>

extern int16_t  CurveGetInflexionCentreMv     (void); extern void CurveSetInflexionCentreMv     (int16_t);
extern uint8_t  CurveGetInflexionCentrePercent(void); extern void CurveSetInflexionCentrePercent(uint8_t);
extern uint32_t CurveGetInflexionCentreAs     (void);
extern int8_t   CurveGetInflexionWidthMv      (void);
extern char     CurveGetInflexionAsFromMv     (int16_t mv, uint32_t* pAs); //returns 0 if ok or -1 if mv is out of range.

extern void CurveInit(void);